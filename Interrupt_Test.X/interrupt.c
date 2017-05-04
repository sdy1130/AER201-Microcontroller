#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "macros.h"

int right_up = 3250;
int centre_up = 1750;   
int left_up = 3750;
int right_down = 19625;
int centre_down = 18250;   
int left_down = 16750;
float nominal_AA_C = 1.5;
float nominal_9V = 9;

void set_external_interrupt1(float time) {
	// set interrupt for external timer1 for time in ms to control servo
	unsigned int set_time = 65535-(int)((float)time*2/8); // with oscillation at 8MHz, prescaler 8
    TMR1H = set_time >> 8;
    TMR1L = set_time & 0b11111111;
	TMR1ON=1; // turn timer0 on
	return;
}

void servo_ISR(){
    
	TMR1IF = 0; //clear_interrupt(timer1);
    TMR1IE = 0; // disable timer1 interrupts
    TMR1ON = 0; // turn timer1 off
    
    extern volatile unsigned char current_state;
    extern volatile float up_period;
    extern volatile float down_period;
    
    //static int count = 0;
    
    if(current_state == 1){
        
        current_state = 0;
        set_external_interrupt1(down_period);
        LATA = 0x00000000;
        
    }
    else if (current_state == 0){
        
        current_state = 1;
        set_external_interrupt1(up_period);
        LATA = 0x00000001;
        
    }
    
    //count++;
	return;
}

void interrupt IRQ(void) {
	if (TMR1IE && TMR1IF) {
			servo_ISR();
			return;
		}
}