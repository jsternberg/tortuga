/*header file for time.c 
 *Scott Watson April 20, 2008
 *include this file whenever using time.c functions in another file
 */
void init_Timer2(void);
void init_Timer3(void);
void delay(int microseconds);

void disableTimer1(void);
void disableTimer2(void);
void disableTimer3(void);
 
void enableTimer1(void);
void enableTimer2(void);
void enableTimer3(void);

