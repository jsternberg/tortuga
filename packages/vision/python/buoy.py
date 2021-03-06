# Copyright (C) 2010 Maryland Robotics Club
# Copyright (C) 2010 Jonathan Sternberg <jsternbe@umd.edu>
# All rights reserved.
#
# Author: Jonathan Sternberg <jsternbe@umd.edu>
# File:  packages/vision/python/buoy.py
#
# Requirements:
#   - Must have opencv python bindings
#      - Package list:
#         - python-opencv
#         - python-yaml
#   - Must have a ~/stock_images/buoy folder
#   - Within this folder, it must have the folders
#      - ppm
#      - output
#      - canny
#      - processed
#      - grayscale
#      - hue
#   - Inside the ppm folder must be the images to be tested
#     in ppm format
#   - Must be in a Unix type system (for commands module)

# Use true division everywhere
from __future__ import division

import commands
import math
import os
import sys
import yaml

from opencv.cv import *
from opencv.highgui import *

class Blob(object):
    def __init__(self, x, y, r, percentage):
        self._x, self._y, self._r, self._percentage = \
            x, y, r, percentage

    def __cmp__(self, other):
        if self._percentage < other._percentage:
            return -1
        elif self._percentage > other._percentage:
            return 1
        else:
            return self._r - other._r

    def __str__(self):
        return '%d %d %d %.2f' % (self._x, self._y, self._r, self._percentage)

def histogram_circle_row(img, center, radius, row, debug_img = None):
    x0, y0 = center
    hist = {}
    hist[0] = [0]*256
    hist[1] = [0]*256
    hist[2] = [0]*256
    
    # Proof that this works (with explanation) in arclength.py
    theta = math.asin( row / radius )
    width = cvRound(radius * math.cos(theta))
    
    for x in xrange(-width, width):
        try:
            colors = img[y0 + row, x0 + x]
            # Black out the pixel we read if debug is on
            increment = math.sqrt(row**2 + x**2) / radius
            hist[0][int(colors[0])] += 1 - increment
            hist[1][int(colors[1])] += 1 - increment
            hist[2][int(colors[2])] += 1 - increment
            if debug_img is not None:
                hue = int(colors[0])
                debug_img[y0 + row, x0 + x] = [hue]
        except IndexError:
            pass

    return 2 * width + 1, hist

def analyze(filename, cfg, output_cfg):
    found, result = False, [0, 0, 0, 'unknown']

    # Load the original image
    original = cvLoadImage(filename)

    # Use color segmentation algorithm using cmd line executable
    output = output_cfg.setdefault(os.path.basename(filename), {})
    outputpath = os.path.join(os.path.dirname(filename), '..', 'output',
                          os.path.basename(filename))
    commands.getstatusoutput(cfg['command'] + ' ' + filename + ' ' + outputpath)

    img = cvLoadImage(outputpath)
    debug = cvCreateImage(cvGetSize(img), 8, 3)
    cvCopy(img, debug)
    gray = cvCreateImage(cvGetSize(img), 8, 1)
    cvCvtColor(img, gray, CV_BGR2GRAY)

    # Calculate cvCanny and save the results so we can see it instead
    # of it being hidden in cvHoughCircles
    canny = cvCreateImage(cvGetSize(img), 8, 1)
    cvCanny(gray, canny, cfg.get('param1', 100),
            max(cfg.get('param1', 100) // 2, 1))
    # cannypath = os.path.join(os.path.dirname(filename), '..', 'canny',
    #                          os.path.basename(filename))
    # cvSaveImage(cannypath, canny)

    storage = cvCreateMemStorage(0)
    seq = cvHoughCircles(gray, storage, CV_HOUGH_GRADIENT,
                            cfg['dp'], cfg['min_dist'],
                            cfg.get('param1',100), cfg.get('param2',100),
                            cfg.get('min_radius',0),
                            cfg.get('max_radius',0))

    for p in seq:
        cvCircle(debug, (cvRound(p[0]), cvRound(p[1])),
                 3, cvScalar(0,0,0), -1)
        cvCircle(debug, (cvRound(p[0]), cvRound(p[1])),
                 cvRound(p[2]), cvScalar(0,0,0), 3)

    # Dilate the image and analyze the circles percentage of circleness
    cvDilate(canny, canny, None, cfg['dilate'])
    cannypath = os.path.join(os.path.dirname(filename), '..', 'canny',
                             os.path.basename(filename))
    cvSaveImage(cannypath, canny)

    blobList = []
    real_min_radius = cfg.get('real_min_radius', cfg.get('min_radius', 0))
    for p in seq:
        x, y, r = p[0], p[1], p[2]
        acc, total = 0, 0
        # Check the radius of the circle for points
        for deg in xrange(360):
            theta = math.radians(deg)
            ptx = cvRound(r * math.cos(theta) + x)
            pty = cvRound(r * math.sin(theta) + y)
            try:
                if canny[pty, ptx] > 0:
                    acc += 1
                total += 1
            except IndexError:
                # Ignore index errors, don't consider them part of the circle
                pass
        if total > 0 and r > real_min_radius:
            blobList.append(Blob(x, y, r, acc / total))

    # Sort the list into best matches first
    blobList.sort(reverse = True)

    # Save these results
    for i, k in enumerate(blobList):
        output[i] = str(k)

    # Declare histogram data
    hist, pixel_count = {}, 0
    hist[0] = [0]*256
    hist[1] = [0]*256
    hist[2] = [0]*256

    hue_image = cvCreateImage(cvGetSize(img), 8, 1)
    cvSetZero(hue_image)

    # Choose the best match
    if len(blobList) > 0 and blobList[0]._percentage > cfg['min_percent']:
        found = True
        blob = blobList[0]
        result[0], result[1], result[2] = \
            cvRound(blob._x), cvRound(blob._y), cvRound(blob._r)

        # Try and identify the color
        hsv = cvCreateImage(cvGetSize(img), 8, 3)
        cvCvtColor(img, hsv, CV_BGR2HSV)

        # Run histogram on each row
        x, y, radius = result[0], result[1], result[2]
        for row in xrange(-radius, radius+1):
            pixels, newhist = histogram_circle_row(original, (x, y), radius, row, hue_image)
            hist[0] = map(lambda a: a[0] + a[1], zip(hist[0], newhist[0]))
            hist[1] = map(lambda a: a[0] + a[1], zip(hist[1], newhist[1]))
            hist[2] = map(lambda a: a[0] + a[1], zip(hist[2], newhist[2]))
            pixel_count += pixels

        def largest_point(a, b):
            if a[1] > b[1]:
                return a
            else:
                return b
        # Look at the first channel and find the peak point
        hue = reduce(largest_point, enumerate(hist[0]))[0]
        output['hue'] = hue

        if cfg['red_min'] < hue and cfg['red_max'] > hue:
            result[3] = 'red'
        elif cfg['green_min'] < hue and cfg['green_max'] > hue:
            result[3] = 'green'
        elif cfg['yellow_min'] < hue and cfg['yellow_max'] > hue:
            result[3] = 'yellow'
        else:
            pass # No color found in range, keep as unknown

        # Output the histogram (none if one wasn't generated)
        # Only print if configuration asks for one
        if cfg.get('generate_hist', False):
            for i, k in hist.iteritems():
                histmap = {}
                for i2, k2 in enumerate(k):
                    histmap[i2] = k2
                hist[i] = histmap
            output['hist'] = hist
    output['found'] = found

    huepath = os.path.join(os.path.dirname(filename), '..', 'hue',
                             os.path.basename(filename))
    cvSaveImage(huepath, hue_image)

    # Redraw the chosen circle as the found color (unknown is gray)
    if found:
        color = result[3]
        if 'red' == color:
            scalar = cvScalar(0,0,255)
        elif 'yellow' == color:
            scalar = cvScalar(0,255,255)
        elif 'green' == color:
            scalar = cvScalar(0,255,0)
        else:
            # Assumed to be unknown
            scalar = cvScalar(128,128,128)

        cvCircle(debug, (result[0], result[1]),
                 3, scalar, -1)
        cvCircle(debug, (result[0], result[1]),
                 result[2], scalar, 3)

    processed = os.path.join(os.path.dirname(filename), '..', 'processed',
                             os.path.basename(filename))
    grayscale = os.path.join(os.path.dirname(filename), '..', 'grayscale',
                             os.path.basename(filename))
    cvSaveImage(processed, debug)
    cvSaveImage(grayscale, gray)

    return (found, result)

def main():
    if len(sys.argv) < 2:
        raise Exception('Usage: python process.py CONFIG')

    spos, sneg, accuracy, rad_acc, fpos, fneg, ptotal, ntotal, correct_color = \
        0, 0, 0, 0, 0, 0, 0, 0, 0

    source = os.path.expanduser(os.path.join('~', 'stock_images', 'buoy', 'ppm'))
    config = yaml.load(file(sys.argv[1]))
    output = {}

    if not config.has_key('command'):
        raise Exception('No command given in the configuration file')

    files = [os.path.join(source, x) for x in os.listdir(source)]
    files.sort()
    for i, f in enumerate(files):
        print '\r%d / %d' % (i+1, len(files)),
        sys.stdout.flush()
        #print '\rProgress:', i, '/', len(files),
        found, result = analyze(f, config, output)
        basename = os.path.basename(f)
        if config.has_key(basename) and found:
            # Average the distance
            real = config[basename]
            #ssd = (((result[0]*real['x'])**2)/(result[0]+real['x']) + \
            #            ((result[1]*real['y'])**2)/(result[1]+real['y']) + \
            #            ((result[2]*real['r'])**2)/(result[2]+real['r'])) * .5
            ssd = math.sqrt((result[0]-real['x'])**2 + \
                                (result[1]-real['y'])**2)
            rad_ssd = math.sqrt((result[2]-real['r'])**2)
            accuracy += ssd
            rad_acc += rad_ssd
            # Success
            spos += 1
            ptotal += 1

            # Check if the correct color was returned
            if real['color'] == result[3]:
                correct_color += 1

        elif config.has_key(basename) and not found:
            # Didn't find the point even though we should have, false negative
            fneg += 1
            ptotal += 1
        elif found and not config.has_key(f):
            # Found something we shouldn't have, false positive
            fpos += 1
            ntotal += 1
        else:
            # No buoy, no result, success
            sneg += 1
            ntotal += 1

    print '\n\nSuccess Positive:',spos,'/',ptotal,'- %.2f %%'%(spos/ptotal*100)
    print 'False Negative:',fneg,'/',ptotal,'- %.2f %%' % (fneg/ptotal*100)
    print 'Success Negative:',sneg,'/',ntotal,'- %.2f %%' % (sneg/ntotal*100)
    print 'False Positive:', fpos, '/', ntotal,'- %.2f %%' % (fpos/ntotal*100)
    print 'Correct Color:', correct_color, '/', ptotal,\
        '- %.2f %%'%(correct_color/ptotal*100)
    print 'Accuracy: %e' % (accuracy / spos)
    print 'Radius Accuracy: %e' % (rad_acc / spos)

    with open(config.get('output', 'results.yml'), 'w') as fd:
        print 'Dumping results file. Please wait.'
        yaml.dump(output, fd, indent=4, default_flow_style=False)
        print 'Finished.'

if __name__ == '__main__':
    main()
