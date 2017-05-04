/*
 * File:   main.c
 * Author: sdy1130
 *
 * Created on February 25, 2017, 11:54 AM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "macros.h"
#include "lcd.h"
#include "ADC.h"

volatile unsigned char current_state0 = 0; //low initial state
volatile unsigned char current_state1 = 0; //low initial state
volatile unsigned char current_state3 = 0; //low initial state
volatile unsigned char stage0 = 1; //1 top, 2 centre, 3 bot
volatile unsigned char stage1 = 1; //1 top, 2 centre, 3 bot
volatile unsigned char stage3 = 1; //1 top, 2 centre, 3 bot
volatile unsigned char count0 = 0;
volatile unsigned char count1 = 0;
volatile unsigned char count3 = 0;
volatile unsigned char gate_status0 = 0; //0 closed, 1 open
volatile unsigned char gate_status1 = 0; //0 closed, 1 open
volatile unsigned char gate_status3 = 0; //0 closed, 1 open
volatile unsigned char gate_flag0 = 0; //1 if gate opened and closed
volatile unsigned char gate_flag1 = 0; //1 if gate opened and closed
volatile unsigned char gate_flag3 = 0; //1 if gate opened and closed
int right_up = 3250;
int centre_up = 1750;   
int left_up = 3750;
int right_down = 19625;
int centre_down = 18250;   
int left_down = 16750;
float nominal_AA_C = 1.5;
float nominal_9V = 9;
volatile float voltage_AA = 0;
volatile float voltage_9V = 0;
volatile float voltage_C = 0;
volatile int volt_check0 = 0; //1 if voltage is checked
volatile int volt_check1 = 0;
volatile int volt_check3 = 0;
volatile int count_AA = 0; //number of batteries
volatile int count_9V = 0;
volatile int count_C = 0;
volatile int count_drained = 0;

void set_external_interrupt0(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/2); // with oscillation at 8MHz, prescaler 8
    
    TMR0H = set_time >> 8;
    TMR0L = set_time & 0b11111111;
	TMR0ON = 1; // turn timer0 on
	return;
}

void set_external_interrupt1(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/2); // with oscillation at 8MHz, prescaler 8
    
    TMR1H = set_time >> 8;
    TMR1L = set_time & 0b11111111;
	TMR1ON = 1; // turn timer0 on
	return;
}

void set_external_interrupt3(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/2); // with oscillation at 8MHz, prescaler 8
    
    TMR3H = set_time >> 8;
    TMR3L = set_time & 0b11111111;
	TMR3ON = 1; // turn timer0 on
	return;
}

void open_close_gate_0(){
    
    volatile int up_period;
    volatile int down_period;
    
    if(gate_status0 == 0){ 
        if (count0 < 19){
            if(current_state0 == 1){     

                current_state0 = 0;
                down_period = centre_down;
                LATCbits.LATC3 = 0;
                set_external_interrupt0(down_period);

            }
            else if (current_state0 == 0){

                current_state0 = 1;
                up_period = centre_up;
                LATCbits.LATC3 = 1;
                set_external_interrupt0(up_period);

            }
            count0++;
            return;
        }
        else{
            gate_status0 = 1;
            count0 = 0;
            LATCbits.LATC3 = 0;
            set_external_interrupt0(centre_up);
            return;
        }
    }
    if(gate_status0 == 1){
        if (count0 < 19){
            if(current_state0 == 1){     

                current_state0 = 0;
                down_period = right_down;
                LATCbits.LATC3 = 0;
                set_external_interrupt0(down_period);

            }
            else if (current_state0 == 0){

                current_state0 = 1;
                up_period = right_up;
                LATCbits.LATC3 = 1;
                set_external_interrupt0(up_period);

            }
            count0++;
            return;
        }
        else{
            gate_status0 = 0;
            count0 = 0;
            LATCbits.LATC3 = 0;
            gate_flag0 = 1;
            set_external_interrupt0(right_up);
            return;
        }
    }
}

void open_close_gate_1(){
    
    volatile int up_period;
    volatile int down_period;
    
    if(gate_status1 == 0){ 
        if (count1 < 19){
            if(current_state1 == 1){     

                current_state1 = 0;
                down_period = centre_down;
                LATCbits.LATC4 = 0;
                set_external_interrupt1(down_period);

            }
            else if (current_state1 == 0){

                current_state1 = 1;
                up_period = centre_up;
                LATCbits.LATC4 = 1;
                set_external_interrupt1(up_period);

            }
            count1++;
            return;
        }
        else{
            gate_status1 = 1;
            count1 = 0;
            LATCbits.LATC4 = 0;
            set_external_interrupt1(centre_up);
            return;
        }
    }
    if(gate_status1 == 1){
        if (count1 < 19){
            if(current_state1 == 1){     

                current_state1 = 0;
                down_period = right_down;
                LATCbits.LATC4 = 0;
                set_external_interrupt1(down_period);

            }
            else if (current_state1 == 0){

                current_state1 = 1;
                up_period = right_up;
                LATCbits.LATC4 = 1;
                set_external_interrupt1(up_period);

            }
            count1++;
            return;
        }
        else{
            gate_status1 = 0;
            count1 = 0;
            LATCbits.LATC4 = 0;
            gate_flag1 = 1;
            set_external_interrupt1(right_up);
            return;
        }
    }
}

void open_close_gate_3(){
    
    volatile int up_period;
    volatile int down_period;
    
    if(gate_status3 == 0){ 
        if (count3 < 19){
            if(current_state3 == 1){     

                current_state3 = 0;
                down_period = centre_down;
                LATCbits.LATC5 = 0;
                set_external_interrupt3(down_period);

            }
            else if (current_state3 == 0){

                current_state3 = 1;
                up_period = centre_up;
                LATCbits.LATC5 = 1;
                set_external_interrupt3(up_period);

            }
            count3++;
            return;
        }
        else{
            gate_status3 = 1;
            count3 = 0;
            LATCbits.LATC5 = 0;
            set_external_interrupt3(centre_up);
            return;
        }
    }
    if(gate_status3 == 1){
        if (count3 < 19){
            if(current_state3 == 1){     

                current_state3 = 0;
                down_period = right_down;
                LATCbits.LATC5 = 0;
                set_external_interrupt3(down_period);

            }
            else if (current_state3 == 0){

                current_state3 = 1;
                up_period = right_up;
                LATCbits.LATC5 = 1;
                set_external_interrupt3(up_period);

            }
            count3++;
            return;
        }
        else{
            gate_status3 = 0;
            count3 = 0;
            LATCbits.LATC5 = 0;
            gate_flag3 = 1;
            set_external_interrupt3(right_up);
            return;
        }
    }
}

void main(void) {
    
    OSCCON = 0xF0; //8Mhz
    
    // <editor-fold defaultstate="collapsed" desc=" STARTUP SEQUENCE ">
    
    TRISA = 0b11100111; // Set Port RA4, RA3 as output
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
    ADCON1 = 0x0B;  //Set PORTB to be digital instead of analog default  
    
    nRBPU = 0;
    
    // code to set up timer1
    T0CON = 0b00010000;
    T1CON = 0b10010000; // 16-bit operation 
    T3CON = 0b10010100;
    // bit 6 is unimplemented
    // prescale - 1:8

    TMR0ON = 0; // disable timer1 for now
    TMR0IE = 1; // enable interrupts
    TMR0IF = 0; //clear interrupt flag

    TMR1ON = 0; // disable timer1 for now
    TMR1IE = 1; // enable interrupts
    TMR1IF = 0; //clear interrupt flag
    
    TMR3ON = 0; // disable timer1 for now
    TMR3IE = 1; // enable interrupts
    TMR3IF = 0; //clear interrupt flag
    ei();
    PEIE = 1;
    //</editor-fold>
    
    set_external_interrupt3(1000);
    
    //initLCD();
    
    //printf("3: PC Download");
    
    while(1){
        
    }
    
    /*while(1){
        
       LATAbits.LATA4 = 1;
       __delay_us(1500);
       LATAbits.LATA4 = 0;
       __delay_us(18500);
    }*/
    return;
}

void servo_ISR_0(){ //AA
    
    TMR0IF = 0; //clear_interrupt(timer0);
    TMR0ON = 0;
    
    volatile int up_period;
    volatile int down_period;
    
    if(stage0 == 1){
        if(PORTAbits.RA3 == 1){
            TMR0IE = 0;
            return;
        }      
        if (count0 < 19){
            if(current_state0 == 1){     

                current_state0 = 0;
                down_period = centre_down;
                LATCbits.LATC0 = 0;
                set_external_interrupt0(down_period);

            }
            else if (current_state0 == 0){

                current_state0 = 1;
                up_period = centre_up;
                LATCbits.LATC0 = 1;
                set_external_interrupt0(up_period);

            }
            count0++;
            return;
        }
        count0 = 0;
        LATCbits.LATC0 = 0;
        stage0 = 2;
        set_external_interrupt0(up_period);
        return;
    }
    if(stage0 == 2){
        if(volt_check0 == 0){
            voltage_AA = readADC(0);
            volt_check0 = 1;
        }
        
        if(voltage_AA < (0.85*nominal_AA_C)){
            if(gate_flag0 == 0){
                open_close_gate_0();
                return;
            }
            if (count0 < 19){
                if(current_state0 == 1){     

                    current_state0 = 0;
                    down_period = right_down;
                    LATCbits.LATC0 = 0;
                    set_external_interrupt0(down_period);

                }
                else if (current_state0 == 0){

                    current_state0 = 1;
                    up_period = right_up;
                    LATCbits.LATC0 = 1;
                    set_external_interrupt0(up_period);

                }
                count0++;
                return;
            }
            count0 = 0;
            gate_flag0 = 0;
            volt_check0 = 0;
            count_drained++;
            LATCbits.LATC0 = 0;
            stage0 = 1;
            set_external_interrupt0(right_up);
            return;
        }
        if (count0 < 19){
            if(current_state0 == 1){     

                current_state0 = 0;
                down_period = left_down;
                LATCbits.LATC0 = 0;
                set_external_interrupt0(down_period);

            }
            else if (current_state0 == 0){

                current_state0 = 1;
                up_period = left_up;
                LATCbits.LATC0 = 1;
                set_external_interrupt0(up_period);

            }
            count0++;
            return;
        }
        count0 = 0;
        count_AA++;
        volt_check0 = 0;
        LATCbits.LATC0 = 0;
        set_external_interrupt0(left_up);
        stage0 = 3;
        return;
    }
    if(stage0 == 3){
        if (count0 < 19){
            if(current_state0 == 1){     

                current_state0 = 0;
                down_period = right_down;
                LATCbits.LATC0 = 0;
                set_external_interrupt0(down_period);

            }
            else if (current_state0 == 0){

                current_state0 = 1;
                up_period = right_up;
                LATCbits.LATC0 = 1;
                set_external_interrupt0(up_period);

            }
            count0++;
            return;
        }
        count0 = 0;
        LATCbits.LATC0 = 0;
        stage0 = 1;
        set_external_interrupt0(right_up);
        return;
    }
	return;
}



void servo_ISR_1(){ //9V
    
    TMR1IF = 0; //clear_interrupt(timer0);
    TMR1ON = 0;
    
    volatile int up_period;
    volatile int down_period;
    
    if(stage1 == 1){
        if(PORTAbits.RA4 == 1){
            TMR1IE = 0;
            return;
        }      
        if (count1 < 19){
            if(current_state1 == 1){     

                current_state1 = 0;
                down_period = centre_down;
                LATCbits.LATC1 = 0;
                set_external_interrupt1(down_period);

            }
            else if (current_state1 == 0){

                current_state1 = 1;
                up_period = centre_up;
                LATCbits.LATC1 = 1;
                set_external_interrupt1(up_period);

            }
            count0++;
            return;
        }
        count1 = 0;
        LATCbits.LATC1 = 0;
        stage1 = 2;
        set_external_interrupt1(up_period);
        return;
    }
    if(stage1 == 2){
        if(volt_check1 == 0){
            voltage_9V = readADC(1);
            volt_check1 = 1;
        }
        
        if(voltage_9V < (0.85*nominal_9V)){
            if(gate_flag1 == 0){
                open_close_gate_1();
                return;
            }
            if (count1 < 19){
                if(current_state1 == 1){     

                    current_state1 = 0;
                    down_period = right_down;
                    LATCbits.LATC1 = 1;
                    set_external_interrupt1(down_period);

                }
                else if (current_state1 == 0){

                    current_state1 = 1;
                    up_period = right_up;
                    LATCbits.LATC1 = 1;
                    set_external_interrupt1(up_period);

                }
                count1++;
                return;
            }
            count1 = 0;
            gate_flag1 = 0;
            volt_check1 = 0;
            count_drained++;
            LATCbits.LATC1 = 0;
            stage1 = 1;
            set_external_interrupt1(right_up);
            return;
        }
        if (count1 < 19){
            if(current_state1 == 1){     

                current_state1 = 0;
                down_period = left_down;
                LATCbits.LATC1 = 0;
                set_external_interrupt1(down_period);

            }
            else if (current_state1 == 0){

                current_state1 = 1;
                up_period = left_up;
                LATCbits.LATC1 = 1;
                set_external_interrupt1(up_period);

            }
            count1++;
            return;
        }
        count1 = 0;
        count_9V++;
        volt_check1 = 0;
        LATCbits.LATC1 = 0;
        set_external_interrupt1(left_up);
        stage1 = 3;
        return;
    }
    if(stage1 == 3){
        if (count1 < 19){
            if(current_state1 == 1){     

                current_state1 = 0;
                down_period = right_down;
                LATCbits.LATC1 = 0;
                set_external_interrupt1(down_period);

            }
            else if (current_state1 == 0){

                current_state1 = 1;
                up_period = right_up;
                LATCbits.LATC1 = 1;
                set_external_interrupt1(up_period);

            }
            count1++;
            return;
        }
        count1 = 0;
        LATCbits.LATC1 = 0;
        stage1 = 1;
        set_external_interrupt1(right_up);
        return;
    }
	return;
}

void servo_ISR_3(){ //C
    
    TMR3IF = 0; //clear_interrupt(timer0);
    TMR3ON = 0;
    
    volatile int up_period;
    volatile int down_period;
    
    if(stage3 == 1){
        if(PORTAbits.RA5 == 1){
            TMR3IE = 0;
            return;
        }      
        if (count3 < 19){
            if(current_state3 == 1){     

                current_state3 = 0;
                down_period = centre_down;
                LATCbits.LATC2 = 0;
                set_external_interrupt3(down_period);

            }
            else if (current_state3 == 0){

                current_state3 = 1;
                up_period = centre_up;
                LATCbits.LATC2 = 1;
                set_external_interrupt3(up_period);

            }
            count3++;
            return;
        }
        count3 = 0;
        LATCbits.LATC2 = 0;
        stage3 = 2;
        set_external_interrupt3(up_period);
        return;
    }
    if(stage3 == 2){
        if(volt_check3 == 0){
            voltage_C = readADC(2);
            volt_check3 = 1;
        }
        
        if(voltage_C < (0.85*nominal_AA_C)){
            if(gate_flag3 == 0){
                open_close_gate_3();
                return;
            }
            if (count3 < 19){
                if(current_state3 == 1){     

                    current_state3 = 0;
                    down_period = right_down;
                    LATCbits.LATC2 = 1;
                    set_external_interrupt3(down_period);

                }
                else if (current_state3 == 0){

                    current_state3 = 1;
                    up_period = right_up;
                    LATCbits.LATC2 = 1;
                    set_external_interrupt3(up_period);

                }
                count3++;
                return;
            }
            count3 = 0;
            gate_flag3 = 0;
            volt_check3 = 0;
            count_drained++;
            LATCbits.LATC2 = 0;
            stage3 = 1;
            set_external_interrupt3(right_up);
            return;
        }
        if (count3 < 19){
            if(current_state3 == 1){     

                current_state3 = 0;
                down_period = left_down;
                LATCbits.LATC2 = 0;
                set_external_interrupt3(down_period);

            }
            else if (current_state3 == 0){

                current_state3 = 1;
                up_period = left_up;
                LATCbits.LATC2 = 1;
                set_external_interrupt3(up_period);

            }
            count3++;
            return;
        }
        count3 = 0;
        count_C++;
        volt_check3 = 0;
        LATCbits.LATC2 = 0;
        set_external_interrupt3(left_up);
        stage3 = 3;
        return;
    }
    if(stage3 == 3){
        if (count3 < 19){
            if(current_state3 == 1){     

                current_state3 = 0;
                down_period = right_down;
                LATCbits.LATC2 = 0;
                set_external_interrupt3(down_period);

            }
            else if (current_state3 == 0){

                current_state3 = 1;
                up_period = right_up;
                LATCbits.LATC2 = 1;
                set_external_interrupt3(up_period);

            }
            count3++;
            return;
        }
        count3 = 0;
        LATCbits.LATC2 = 0;
        stage3 = 1;
        set_external_interrupt3(right_up);
        return;
    }
	return;
}

void interrupt IRQ(void) {
	if (TMR1IE && TMR1IF) {
			servo_ISR_1();
			return;
		}
    if (TMR0IE && TMR0IF) {
			servo_ISR_0();
			return;
		}
    if (TMR3IE && TMR3IF) {
			servo_ISR_3();
			return;
		}
}