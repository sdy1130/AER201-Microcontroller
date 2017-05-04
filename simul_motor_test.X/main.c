/*
 * File:   main.c
 * Author: sdy1130
 *
 * Created on February 26, 2017, 5:58 PM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "macros.h"
#include "interrupt.h"
#include "variables.h"
#include "lcd.h"
#include "ADC.h"

//extern volatile unsigned char current_state0; //low initial state
extern volatile unsigned char current_state1;
extern volatile unsigned char current_state3;
//extern volatile unsigned char stage0; //1 top, 2 centre, 3 bot, 4 gate open/close left, 54 gate open/close right
extern volatile unsigned char stage1;
extern volatile unsigned char stage3;
//extern volatile int count0;
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

void main(void) {
    
    count1 = 0;
    count3 = 0;
    current_state1 = 0;
    current_state3 = 0;
    beam_check1 = 0;
    beam_check3 = 0;
    
    // <editor-fold defaultstate="collapsed" desc=" STARTUP SEQUENCE ">
    OSCCON = 0xF0;
    
    TRISA = 0xFF; // Set Port A as input
    TRISB = 0xFF;
    TRISC = 0x00;
    TRISD = 0x00; //All output mode for LCD
    TRISE = 0b00000011;    

    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0b00001000;  //Set PORTB to be digital instead of analog default
    CVRCON = 0x00; // Disable CCP reference voltage output
    CMCONbits.CIS = 0;
    ADFM = 1;
    
    nRBPU = 0;
    
    // code to set up timer 0, 1, 3
    //T0CON = 0b00100000;
    T1CON = 0b10010000; // 16-bit operation 
    T3CON = 0b10010000;
    // bit 6 is unimplemented
    // prescale - 1:2
    
    /*TMR0ON = 0; // disable timer1 for now
    TMR0IE = 1; // enable interrupts
    TMR0IF = 0; //clear interrupt flag*/

    TMR1ON = 0; // disable timer1 for now
    TMR1IE = 1; // enable interrupts
    TMR1IF = 0; //clear interrupt flag
    
    TMR3ON = 0; // disable timer1 for now
    TMR3IE = 1; // enable interrupts
    TMR3IF = 0; //clear interrupt flag
    
    ei();
    PEIE = 1;

    //</editor-fold>
    
    /*set_external_interrupt1(1000);
    
    __delay_us(7);
    
    set_external_interrupt3(1000);
    
    while(1){ //servo test [interrupt]
        
    }
    
    unsigned char i;
    unsigned char keypress;
    const char keys[] = "123A456B789C*0#D";*/
    
    /*while(1){ //servo test [no interrupt]
        
        initLCD();
        printf("No pulse");
        if (PORTBbits.RB1 == 0){
                
        }
        keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code

        while(PORTBbits.RB1 == 1){
                    // Wait until the key has been released
        }

        if (keys[keypress] == keys[0]) //If 1 is pressed
        {
            initLCD();
            printf("Pulse 2ms");
            for (i=0;i<100;i++)
            {
               RC0 = 1;
               __delay_us(2375);
               RC0 = 0;
               __delay_ms(20);
            }
        }
        
        else if (keys[keypress] == keys[1]) //If 2 is pressed
        {
            initLCD();
            printf("Pulse 1.0ms");
            for (i=0;i<100;i++)
            {
               RC0 = 1;
               __delay_us(1000);
               RC0 = 0;
               __delay_ms(20);
            }
        }
        else if (keys[keypress] == keys[2]) //If 3 is pressed
        {
            initLCD();
            printf("Pulse 1.5ms");
            for (i=0;i<100;i++)
            {
               RC0 = 1;
               __delay_us(1650);
               RC0 = 0;
               __delay_ms(20);
            }
        }
        __delay_ms(1000);
    }*/
    
    /*while(1){ //solenoid & dc motor test
        
        if (PORTBbits.RB1 == 0){
                
        }
        keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code

        while(PORTBbits.RB1 == 1){
                    // Wait until the key has been released
        }
        if (keys[keypress] == keys[0]) //If 1 is pressed
        {
            LATCbits.LATC0 = 1;
            __delay_1s();
            __delay_1s();
            __delay_1s();
            __delay_1s();
            __delay_1s();
            __delay_1s();
            __delay_1s();
            __delay_1s();
            __delay_1s();
            LATCbits.LATC0 = 0;
        }
    }*/
    
    /*while(1){ //voltage test
        float a = readADC(4);
        //float b = readADC(1);
        
        initLCD();
        printf("a: %f", a);
        __delay_1s();
        // initLCD();
        //printf("b: %f", b);
        //__delay_1s();
    }*/
    
    /*while(1){ //break beam sensor
        
        float a = readADC(0);
        
        initLCD();
        printf("%f", a);
        __lcd_newline();
        
        if(a > 4.8){
            
            printf("nothing");
            __delay_1s();
        }
        else{
            
            printf("detected");
            __delay_1s();
        }
    }*/
    
    while(1){
        LATDbits.LATD0 = 1;
        __delay_1s();
        __delay_1s();
        __delay_1s();
        __delay_1s();
        LATDbits.LATD0 = 0;
         __delay_1s();
          __delay_1s();
        
    }
    
    /*while(1){
        LATCbits.LATC0 = 1;
        __delay_ms(10);
             LATCbits.LATC0 = 0;
           __delay_ms(10);
                   LATCbits.LATC0 = 1;
        __delay_ms(10);
             LATCbits.LATC0 = 0;
           __delay_ms(10);
                   LATCbits.LATC0 = 1;
        __delay_ms(10);
             LATCbits.LATC0 = 0;
           __delay_ms(10);
           
           
            __delay_1s();
             __delay_1s();
             
    }*/
    
}

void interrupt IRQ(void) {
    di();
    if (TMR1IF) {
            servo_ISR_1();
            ei();
            return;
        }
    //if (TMR0IE && TMR0IF) {
  //     
    //		servo_ISR_0();
   // 		return;
//		}
    if (TMR3IF) {
            servo_ISR_3();
            ei();
            return;
        }
}