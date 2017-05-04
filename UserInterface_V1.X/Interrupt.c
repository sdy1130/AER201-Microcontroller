/*
 * File:   Interrupt.c
 * Author: sdy1130
 *
 * Created on February 24, 2017, 6:15 PM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "ADC.h"

void set_external_interrupt1(float time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int settime = 65535-(int)((float)time*2/8); // with oscillation at 8MHz, prescaler 8
    TMR1H = settime >> 8;
    TMR1L = settime & 0b11111111;
	TMR1ON = 1; // turn timer0 on
	return;
}

void servo_ISR(void){
    
	TMR1IF = 0; //clear_interrupt(timer1);
    TMR1IE = 0; // disable timer1 interrupts
    TMR1ON = 0; // turn timer1 off
    
    volatile unsigned char current_state;
    volatile float up_period;
    volatile float down_period;
    
    //static int count = 0;
    
    if(current_state == 1){
        
        current_state = 0;
        set_external_interrupt1(down_period);
        LATA = 0b00000000;
        
    }
    else if (current_state == 0){
        
        current_state = 1;
        set_external_interrupt1(up_period);
        LATA = 0b00000001;
        
    }
    
    //count++;
	return;
}