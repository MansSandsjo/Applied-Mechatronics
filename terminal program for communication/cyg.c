/*Example serial port use in PC*/

/*Bridge the RXD and TXD pins in the serial port connector (or use your boards to perform a loop-back test!)*/
/*Compile the code, together with serialport.c ->  gcc -o myfile.exe serialexample.c serialport.c */
/*Execute your file!   ./myfile.exe */

#include "serialport.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	/*Declaration of variables*/
	int sp,sl;
	unsigned char outP[8];
	char cin[9];
	char cout[9];
	
	/*Initialise serial port */
	sp = serial_init("/dev/ttyS0",0);
	if(sp == 0)
	{
		printf("Error! Serial port could not be opened.\n");
	}
	else
	{
		printf("Serial port open with identifier %d \n",sp);
	}
	
	/*Initialise both strings - they are initially different! */
	/*The max size of the strings is 6 characters (by declaration)*/
	/*Remember to leave space for the termination character!*/
	while(1){
	scanf(" %5s", outP);
	
	strncpy(cout,outP,9); 
	strncpy(cin,"0",9);
	
	/*Verify that the strings were successfully initialised... */	
	printf("Input: %s", outP);
	
	/*Send the string out */
	write(sp,cout,9);
	/*NOTE: The string itself is smaller than 9 bits... 
	but we are still sending 9 bits, 3 of them useless. 
	When communicating with the AVR we must be more careful! 
	We may want to use fixed string length all the time,
	or have a for loop and send one character at a time*/
	
	
	/*Wait a little bit... (or until a signal comes!)*/
	sleep(1);
	
	/*Read the incoming string */
	read(sp,cin,9);
	
	/*Check that the input string is equal to the output one! */
	printf(" Output: %s", cin);
	}
	
	/*Close the serial port */	
	serial_cleanup(sp);
	return 1;
}