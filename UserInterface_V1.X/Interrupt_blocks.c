/*
 * File:   Interrupt.c
 * Author: sdy1130
 *
 * Created on February 24, 2017, 5:23 PM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "macros.h"

void set_external_interrupt1(int time) {
	// set interrupt for external timer1 for time in ms to control servo
	unsigned int set_time = 65535-(int)((float)time*2000/8); // with oscillation at 8MHz, prescaler 8
    TMR1H = set_time >> 8;
    TMR1L = set_time & 0b11111111;
	TMR1ON=1; // turn timer0 on
    TMR1IF = 0; // clear interrupt flag
    TMR1IE=1; // enable timer0 interrupts
	return;
}

void update_servo_position(float bat_type) {
	// updates up and down period, which is used to drive the servo_IRQ
    extern volatile int servo_up_period;
    extern volatile int servo_down_period;

	servo_up_period = bat_type*SERVO_PERIOD;
	servo_down_period = SERVO_PERIOD - servo_up_period;
	return;
}

void main(void) {
	
	// code to set up timer1
    T1CON = 0b10000000; // 16-bit operation
    // bit 6 is unimplemented
    // prescale - 1:8
    T1CKPS1 = 1;
    T1CKPS0 = 1;

    T1OSCEN = 0; // timer1 oscillator shut off
    T1SYNC = 1; // (bit is ignored when TMR1CS=0) do not synchronize with external clock input
    TMR1CS = 0; // timer mode
    TMR1ON=0; // disable timer1 for now
    TMR1IE = 0; // disable interrupts
    TMR1IF=0; //clear interrupt flag
}

void servo_ISR() {
    extern volatile unsigned char servo_current_state;
    extern volatile int servo_up_period;
    extern volatile int servo_down_period;
	TMR1IF = 0; //clear_interrupt(timer1);
    TMR1IE = 0; // disable timer1 interrupts
    TMR1ON = 0; // turn timer1 off
    
	if (servo_current_state == SERVO_UP) {
		set_external_interrupt1(servo_down_period);
		servo_current_state = SERVO_DOWN;
		SERVO_LAT = 0;
	}
	else if (servo_current_state == SERVO_DOWN) {
		set_external_interrupt1(servo_up_period);
		servo_current_state = SERVO_UP;
		SERVO_LAT = 1;
	}
	else {
		whoops();
	}
	return;
}

void interrupt IRQ(void) {
	if (TMR1IE && TMR1IF) {
			servo_ISR();
			return;
		}
}