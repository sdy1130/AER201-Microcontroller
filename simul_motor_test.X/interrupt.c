/*
 * File:   interrupt.c
 * Author: sdy1130
 *
 * Created on February 26, 2017, 2:05 PM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "macros.h"
#include "variables.h"
#include "lcd.h"

extern volatile unsigned char current_state0; //low initial state
extern volatile unsigned char current_state1;
extern volatile unsigned char current_state3;
//extern volatile unsigned char stage0; //1 top, 2 centre, 3 bot, 4 gate open/close left, 54 gate open/close right
extern volatile unsigned char stage1;
extern volatile unsigned char stage3;
extern volatile int count0;
extern volatile int count1;
extern volatile int count3;
//extern volatile unsigned char gate_status0; //0 closed, 1 open
extern volatile unsigned char gate_status1;
extern volatile unsigned char gate_status3;
//extern volatile unsigned char gate_flag0; //1 if gate opened and closed
extern volatile unsigned char gate_flag1;
extern volatile unsigned char gate_flag3;
extern volatile float voltage_0; //voltage measurement
extern volatile float voltage_1;
extern volatile float voltage_2;
extern volatile float voltage_3;
extern volatile float voltage_AA;
extern volatile float voltage_9V;
extern volatile float voltage_C;
extern volatile unsigned char AA_or_9V; //1 if AA, 2 if 9V
//extern volatile unsigned char disable0; //1 if timer is disabled
extern volatile unsigned char disable1;
extern volatile unsigned char disable3;
//extern volatile unsigned char volt_check0; //1 if voltage is checked
extern volatile unsigned char volt_check1;
extern volatile unsigned char volt_check3;
//extern volatile unsigned char delay0; // 0 delay, 1 skip
//extern volatile unsigned char delay1;
//extern volatile unsigned char delay3;
extern volatile int count_AA; //number of batteries
extern volatile int count_9V;
extern volatile int count_C;
extern volatile int count_drained;
extern volatile char beam_check1;
extern volatile char beam_check3;

/*void set_external_interrupt0(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/4); // with oscillation at 8MHz, prescaler 4
    
    TMR0H = set_time >> 8;
    TMR0L = set_time & 0b11111111;
	TMR0ON = 1; // turn timer0 on
	return;
}*/

//2600 top //1800 centre //1100 bot turning rightwards (cable at the left)

//servo blue: 3200 close (turning to the right), 2000 open 9VAA
//C: 32000 close 2300 open

void set_external_interrupt1(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time1 = (2^16) - time; // with oscillation at 8MHz, prescaler 2
    
    TMR1H = set_time1 >> 8;
    TMR1L = set_time1 & 0b11111111;
	TMR1ON = 1; // turn timer0 on
    
    if(beam_check3 == 0){
        __delay_us(4);
        TMR3ON = 1;
    }
    
	return;
}

void set_external_interrupt3(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time3 = (2^16) - time; // with oscillation at 8MHz, prescaler 2
    
    TMR3H = set_time3 >> 8;
    TMR3L = set_time3 & 0b11111111;
	TMR3ON = 1; // turn timer0 on
    
    if(beam_check1 == 0){
        __delay_us(4);
        TMR1ON = 1;
    }
	return;
}

void servo_ISR_1(){ //C
    
    TMR1IF = 0; //clear_interrupt(timer0);
    TMR3ON = 0;
    __delay_us(4);
    TMR1ON = 0;
    
    int up_period1;
    int down_period1;
   
    if (count1 < 500){
        if(current_state1 == 1){

            current_state1 = 0;
            down_period1 = 20000;
            LATEbits.LATE0 = 0;
            set_external_interrupt1(down_period1);

        }
        else if (current_state1 == 0){

            current_state1 = 1;
            up_period1 = 2600;
            LATEbits.LATE0 = 1;
            set_external_interrupt1(up_period1);

        }
        count1++;
        return;
    }
    count1 = 0;
    LATEbits.LATE0 = 0;
    beam_check1 = 1;
	return;
}

void servo_ISR_3(){ //C
     //clear_interrupt(timer0);
    TMR1ON = 0;
    TMR3IF = 0;
    __delay_us(4);
    TMR3ON = 0;
    
    int up_period;
    int down_period;
   
    if (count3 < 500){
        if(current_state3 == 1){     

            current_state3 = 0;
            down_period = 20000; //centre_down-600;
            LATEbits.LATE1 = 0;
            set_external_interrupt3(down_period);

        }
        else if (current_state3 == 0){

            current_state3 = 1;
            up_period = 1425;
            LATEbits.LATE1 = 1;
            set_external_interrupt3(up_period);

        }
        count3++;
        return;
    }
    count3 = 0;
    LATEbits.LATE1 = 0;
    beam_check3 = 1;
	return;
}