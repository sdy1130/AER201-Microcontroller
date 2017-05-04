#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "ADC.h"

void set_external_interrupt1(float time) {
	// set interrupt for external timer1 for time in ms to control servo
	unsigned int set_time = 65535-(int)((float)time*2000/8); // with oscillation at 8MHz, prescaler 8
    TMR1H = set_time >> 8;
    TMR1L = set_time & 0b11111111;
	TMR1ON=1; // turn timer0 on
    TMR1IF = 0; // clear interrupt flag
    TMR1IE=1; // enable timer0 interrupts
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

void main(void){
    
    OSCCON = 0xF0; //8Mhz
    
    // <editor-fold defaultstate="collapsed" desc=" STARTUP SEQUENCE ">
    
    TRISA = 0x11111110; // Set Port A as all input
    TRISB = 0xFF; 
    TRISC = 0x00;
    TRISD = 0x00; //All output mode for LCD
    TRISE = 0x00;    

    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0xFF;  //Set PORTB to be digital instead of analog default  
    
    nRBPU = 0;
    
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
    
    volatile unsigned char current_state = 0;
    volatile float up_period = 1.5;
    volatile float down_period = 20 - up_period;

    //</editor-fold>
    
    set_external_interrupt1(down_period);
    
    while(1){
        
    }
    
}