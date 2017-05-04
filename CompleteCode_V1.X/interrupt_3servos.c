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
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "ADC.h"
#include "variables.h"
#include "eeprom.h"

int right_up = 600; //servo PWM values
int centre_up = 1500;
int left_up = 2500;
int right_down = 19400;
int centre_down = 18500;   
int left_down = 17500;
float nominal_AA_C = 1.5;
float nominal_9V = 9;
int right_up_2 = 300; //after gate opens and close
int right_down_2 = 19700;
int right_up_gate = 100;
int right_down_gate = 19900;
int centre_down_gate = 18800;
int centre_up_gate = 1200;

float abs(float num){
    if(num > 0){
        return num;
    }
    return -num;
}

void set_external_interrupt0(int time) {
	// set interrupt for external timer0 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/2); // with oscillation at 8MHz, prescaler 2
    
    TMR0H = set_time >> 8;
    TMR0L = set_time & 0b11111111;
	TMR0ON = 1; // turn timer0 on
    if(disable1 == 0){
        TMR1ON = 1;
    }
    if(disable3 == 0){
        TMR3ON = 1;
    }
	return;
}

void set_external_interrupt1(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/2); // with oscillation at 8MHz, prescaler 2
    
    TMR1H = set_time >> 8;
    TMR1L = set_time & 0b11111111;
	TMR1ON = 1; // turn timer1 on
    if(disable0 == 0){
        TMR0ON = 1;
    }
    if(disable3 == 0){
        TMR3ON = 1;
    }
	return;
}

void set_external_interrupt3(int time) {
	// set interrupt for external timer3 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/2); // with oscillation at 8MHz, prescaler 2
    
    TMR3H = set_time >> 8;
    TMR3L = set_time & 0b11111111;
	TMR3ON = 1; // turn timer3 on
    if(disable0 == 0){
        TMR0ON = 1;
    }
    if(disable1 == 0){
        TMR1ON = 1;
    }
	return;
}

void servo_ISR_0(){ //AA
    
    TMR0IF = 0; //clear_interrupt;
    TMR0ON = 0;
    TMR1ON = 0;
    TMR3ON = 0;
    
    volatile int up_period;
    volatile int down_period;
    
    if(stage0 == 1){
        if(PORTAbits.RA3 != 0){
            disable0 = 1;
            if(disable1 == 0){
                TMR1ON = 1;
            }
            if(disable3 == 0){
                TMR3ON = 1;
            }
            return;
        }      
        if (count0 < 100){
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
        current_state0 = 0;
        set_external_interrupt0(centre_down);
        return;
    }
    
    if(stage0 == 2){
        if(volt_check0 == 0){
            voltage_AA = abs(readADC(0));
            volt_check0 = 1;
        }
        
        if(voltage_AA < (0.85*nominal_AA_C)){
            if(gate_flag0 == 0){
                
                if(gate_status0 == 0){ 
                    if (count0 < 100){
                        if(current_state0 == 1){     

                            current_state0 = 0;
                            down_period = centre_down_gate;
                            LATCbits.LATC5 = 0;
                            set_external_interrupt0(down_period);

                        }
                        else if (current_state0 == 0){

                            current_state0 = 1;
                            up_period = centre_up_gate;
                            LATCbits.LATC5 = 1;
                            set_external_interrupt0(up_period);

                        }
                        count0++;
                        return;
                    }
                    gate_status0 = 1;
                    count0 = 0;
                    LATCbits.LATC5 = 0;
                    current_state0 = 0;
                    set_external_interrupt0(centre_down_gate);
                    return;
                }
                if(gate_status0 == 1){
                    if (count0 < 100){
                        if(current_state0 == 1){     

                            current_state0 = 0;
                            down_period = right_down_gate;
                            LATCbits.LATC5 = 0;
                            set_external_interrupt0(down_period);

                        }
                        else if (current_state0 == 0){

                            current_state0 = 1;
                            up_period = right_up_gate;
                            LATCbits.LATC5 = 1;
                            set_external_interrupt0(up_period);

                        }
                        count0++;
                        return;
                    }
                    gate_status0 = 0;
                    count0 = 0;
                    LATCbits.LATC5 = 0;
                    gate_flag0 = 1;
                    current_state0 = 0;
                    set_external_interrupt0(right_down);
                    return;
                }
                
            }
            if (count0 < 100){
                if(current_state0 == 1){     

                    current_state0 = 0;
                    down_period = right_down_2;
                    LATCbits.LATC0 = 0;
                    set_external_interrupt0(down_period);

                }
                else if (current_state0 == 0){

                    current_state0 = 1;
                    up_period = right_up_2;
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
            current_state0 = 0;
            set_external_interrupt0(right_down_2);
            return;
        }
        if (count0 < 100){
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
        current_state0 = 0;
        set_external_interrupt0(left_down);
        stage0 = 3;
        return;
    }
    if(stage0 == 3){
        if (count0 < 100){
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
        current_state0 = 0;
        set_external_interrupt0(right_down);
        return;
    }
	return;
}

void servo_ISR_1(){ //9V
    
    TMR1IF = 0; //clear_interrupt(timer1);
    TMR1ON = 0;
    TMR0ON = 0;
    TMR3ON = 0;
    
    volatile int up_period;
    volatile int down_period;
    
    if(stage1 == 1){
        if(PORTAbits.RA4 != 0){
            disable1 = 1;
            if(disable0 == 0){
                TMR0ON = 1;
            }
            if(disable3 == 0){
                TMR3ON = 1;
            }
            return;
        } 
        if (count1 < 100){
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
            count1++;
            return;
        }
        count1 = 0;
        LATCbits.LATC1 = 0;
        stage1 = 2;
        current_state1 = 0;
        set_external_interrupt1(centre_down);
        return;
    }
    if(stage1 == 2){
        if(volt_check1 == 0){
            voltage_9V = abs(readADC(1));          
            volt_check1 = 1;
        }
        
        if(voltage_9V < (0.85*nominal_9V)){
            if(gate_flag1 == 0){
                if(gate_status1 == 0){ 
                    if (count1 < 100){
                        if(current_state1 == 1){     

                            current_state1 = 0;
                            down_period = centre_down_gate;
                            LATCbits.LATC6 = 0;
                            set_external_interrupt1(down_period);

                        }
                        else if (current_state1 == 0){

                            current_state1 = 1;
                            up_period = centre_up_gate;
                            LATCbits.LATC6 = 1;
                            set_external_interrupt1(up_period);

                        }
                        count1++;
                        return;
                    }
                    else{
                        gate_status1 = 1;
                        count1 = 0;
                        LATCbits.LATC6 = 0;
                        current_state1 = 0;
                        set_external_interrupt1(centre_down_gate);
                        return;
                    }
                }
                if(gate_status1 == 1){
                    if (count1 < 100){
                        if(current_state1 == 1){     

                            current_state1 = 0;
                            down_period = right_down_2;
                            LATCbits.LATC6 = 0;
                            set_external_interrupt1(down_period);

                        }
                        else if (current_state1 == 0){

                            current_state1 = 1;
                            up_period = right_up_2;
                            LATCbits.LATC6 = 1;
                            set_external_interrupt1(up_period);

                        }
                        count1++;
                        return;
                    }
                    else{
                        gate_status1 = 0;
                        count1 = 0;
                        LATCbits.LATC6 = 0;
                        gate_flag1 = 1;
                        current_state1 = 0;
                        set_external_interrupt1(right_down);
                        return;
                    }
                }
            }
            if (count1 < 100){
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
            gate_flag1 = 0;
            volt_check1 = 0;
            count_drained++;
            LATCbits.LATC1 = 0;
            stage1 = 1;
            current_state1 = 0;
            set_external_interrupt1(right_down);
            return;
        }
        if (count1 < 100){
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
        current_state1 = 0;
        set_external_interrupt1(left_down);
        stage1 = 3;
        return;
    }
    if(stage1 == 3){
        if (count1 < 100){
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
        current_state1 = 0;
        set_external_interrupt1(right_down);
        return;
    }
	return;
}

void servo_ISR_3(){ //C
    
    TMR3IF = 0; //clear_interrupt(timer3);
    TMR3ON = 0;
    TMR0ON = 0;
    TMR1ON = 0;
    
    volatile int up_period;
    volatile int down_period;
    
    if(stage3 == 1){
        if(PORTAbits.RA5 != 0){
            disable3 = 1;
            if(disable0 == 0){
                TMR0ON = 1;
            }
            if(disable1 == 0){
                TMR1ON = 1;
            }
            return;
        }   
        if (count3 < 100){
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
        current_state3 = 0;
        set_external_interrupt3(centre_down);
        return;
    }
    if(stage3 == 2){
        if(volt_check3 == 0){
            voltage_C = abs(readADC(2));
            volt_check3 = 1;
        }
        
        if(voltage_C < (0.85*nominal_AA_C)){
            if(gate_flag3 == 0){
                if(gate_status3 == 0){
                    if (count3 < 100){
                        if(current_state3 == 1){   

                            current_state3 = 0;
                            down_period = centre_down_gate;
                            LATCbits.LATC7 = 0;
                            set_external_interrupt3(down_period);

                        }
                        else if (current_state3 == 0){

                            current_state3 = 1;
                            up_period = centre_up_gate;
                            LATCbits.LATC7 = 1;
                            set_external_interrupt3(up_period);

                        }
                        count3++;
                        return;
                    }
                    gate_status3 = 1;
                    count3 = 0;
                    LATCbits.LATC7 = 0;
                    current_state3 = 0;
                    set_external_interrupt3(centre_down_gate);
                    return;
                }
                if(gate_status3 == 1){
                    if (count3 < 100){
                        if(current_state3 == 1){     

                            current_state3 = 0;
                            down_period = right_down_2;
                            LATCbits.LATC7 = 0;
                            set_external_interrupt3(down_period);

                        }
                        else if (current_state3 == 0){

                            current_state3 = 1;
                            up_period = right_up_2;
                            LATCbits.LATC7 = 1;
                            set_external_interrupt3(up_period);

                        }
                        count3++;
                        return;
                    }
                    else{
                        gate_status3 = 0;
                        count3 = 0;
                        LATCbits.LATC7 = 0;
                        gate_flag3 = 1;
                        current_state3 = 0;
                        set_external_interrupt3(right_down_2);
                        return;
                    }
                }
            }
            if (count3 < 100){
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
            gate_flag3 = 0;
            volt_check3 = 0;
            count_drained++;
            LATCbits.LATC2 = 0;
            stage3 = 1;
            current_state3 = 0;
            set_external_interrupt3(right_down);
            return;
        }
        if (count3 < 100){
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
        current_state3 = 0;
        set_external_interrupt3(left_down);
        stage3 = 3;
        return;
    }
    if(stage3 == 3){
        if (count3 < 100){
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
        current_state3 = 0;
        set_external_interrupt3(right_down);
        return;
    }
	return;
}