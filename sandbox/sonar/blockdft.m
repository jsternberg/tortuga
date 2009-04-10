function blockdft(fname)

% Open the data file.
% Data is stored as little endian two's complement
% 16 bit integers.
fid=fopen(fname,'r');

% If we couldn't open the file, complain about it and fail out.
if fid == -1
  disp('Failed to open file');
  return;
end

% Read data into a 4xN array.
dat = fread(fid,[4,Inf],'int16');
N = length(dat);

% Close the data file.
fclose(fid);

% ADC sample rate
sampRate = 500000;

% We are going to loop over the entire dataset and take the
% FFT of sequential blocks.  This is the size of the Fourier window.
blockSize = 256;

% Number of blocks in the dataset
blockCount = floor(N/blockSize);

% Flag indicating whether we have detected a ping
triggered = 0;

% Loop over each blockSize set of samples
for blockNum=1:blockCount
  
  % Index where block starts
  blockStartIndex = (blockNum-1)*blockSize+1;
  
  % Index where block ends
  blockStopIndex = min(N, blockNum*blockSize);
  
  % Copy block into an array for convenience
  block = dat(:,blockStartIndex:blockStopIndex);
  
  % Take the DFT of each channel, and find its magnitude squared.
  dft = abs(fft(block,[],2)).^2;
  
  % Normalize the DFT to the total signal power.
  dft = dft./repmat(sum(dft,2),1,size(dft,2));
  
  % Look at harmonics between 10 and 20, roughly 20 kHz to 40 kHz.
  dft = dft(:,10:20);
  
  % Find the moust powerful harmonic in the range [10,20].
  [dummy,dftMax] = max(dft,[],2);
  
  % Determine if the maximum occurs at the same harmonic on all channels.
  sameMax = all(dftMax == dftMax(1));
  dftMax = dftMax(1);
  
  % If the maximum harmonic contains more than .2 of the power of the
  % signal, and the maximum occurs at the same harmonic on each channel,
  % and a trigger has not already occurred, then set a trigger.
  if max(max(dft))>.2 && sameMax && !triggered
    
    % Trigger set.
    triggered = 1;
    
    % Announce the frequency that we have detected.
    disp(sprintf("Signal at %f %kHz", \
		 sampRate/blockSize*(dftMax+10)/1000));

    % Plot the spectrum of the block.
    figure(1);
    for i=1:size(dat,1)
      plot(dft(i,:));
      hold on;
    end
    hold off;
    
    % Plot the waveform of the bock we are looking at, plus a few blocks
    % before and after for context.
    figure(2);
    contextBefore = 10*blockSize;
    contextAfter = 10*blockSize;
    contextBeforeStart = blockStartIndex-contextBefore;
    contextAfterStop = blockStopIndex+contextAfter+1;
    if contextBeforeStart <= 0
      contextAfterStop += -contextBeforeStart+1;
      contextBeforeStart = 1;
    elseif contextAfterStop > N
      contextBeforeStart -= contextAfterStop-N;
      contextAfterStop = N;
    end
    skip = 32;
    contextBeforeRange=contextBeforeStart:skip:(blockStartIndex-1);
    contextAfterRange=(blockStopIndex+1):skip:contextAfterStop;
    blockRange=blockStartIndex:skip:blockStopIndex;
    for i=1:size(dat,1)
      plot(blockRange,dat(i,blockRange),'b');
      hold on;
      if length(contextBeforeRange)>0
	plot(contextBeforeRange,dat(i,contextBeforeRange),'c');
      end
      if length(contextAfterRange)>0
	plot(contextAfterRange,dat(i,contextAfterRange),'c');
      end
    end
    hold off;
    axis('auto');
    drawnow;
    pause;
  elseif max(max(dft))<.001
    % Else if the trigger is set, but the signal has died down again,
    % then unset the trigger.
    triggered = 0;
  end
end
end
