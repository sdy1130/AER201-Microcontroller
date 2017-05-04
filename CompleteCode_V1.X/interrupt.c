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

//OUTPUT
//[RC0] for transistor, in 9V and AA voltage measurement
//[RC1] [RC2] for servo, AA9V and C
//[RD0] for dc motor
//[RD1] [RC5] for solenoid
//[RC6] [RC7] for gates, AA9V and C

//Input
//readADC(0) break beam top 9VAA [RA0]
//readADC(1) voltage 9V input 1 [RA1]
//readADC(2) voltage 9V input 2 [RA2]
//readADC(3) break beam bot 9VAA [RA3]
//readADC(4) break beam C  [RA5]
//readADC(5) voltage C [RE0]
//readADC(6) voltage AA input [RE1]

//2400 top //1600 centre //900 bot turning rightwards (cable at the left) //1425 45 degrees
//servo blue: 3200 close (turning to the right), 2000 open 9VAA
//C: 3200 close 2000 open

extern volatile int up_period1; //high and low duration
extern volatile int down_period1;
extern volatile int up_period3;
extern volatile int down_period3;
extern volatile unsigned char current_state1;
extern volatile unsigned char current_state3;
extern volatile unsigned char stage1;//1 top, 2 centre, 3 bot, 4 gate open/close
extern volatile unsigned char stage3;
extern volatile int count1; //pulse counter
extern volatile int count3;
extern volatile unsigned char gate_status1; //1 if open, 0 if not
extern volatile unsigned char gate_status3;
extern volatile unsigned char init1; //initialization flag
extern volatile unsigned char init3;
extern volatile float voltage_1; //voltage input from pins
extern volatile float voltage_2;
extern volatile float voltage_3;
extern volatile float voltage_4;
extern volatile float voltage_6;
extern volatile float voltage_AA; //measured battery voltage
extern volatile float voltage_9V;
extern volatile float voltage_C;
extern volatile unsigned char AA_or_9V; //1 if AA, 2 if 9V
extern volatile unsigned char volt_check1;
extern volatile unsigned char volt_check3;
extern volatile int count_AA; //number of batteries
extern volatile int count_9V;
extern volatile int count_C;
extern volatile int count_drained;
extern volatile char process1; //1 if process is on, 0 if not
extern volatile char process3;
//1 if pin was set high when the other timer's interrupt occurred
extern volatile unsigned char isr1; 
extern volatile unsigned char isr3;

void set_external_interrupt1(int time) {
	// set interrupt for external timer1 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/4); // with oscillation at 8MHz, prescaler 4
    
    TMR1H = set_time >> 8;
    TMR1L = set_time & 0b11111111;
    TMR1ON = 1; // turn timer1 on
    
    if(isr3 == 1){ //if the isr flag is set, set 20 ms timer interrupt for next pulse output
        
        set_time = 65535 - (20000*2/4);
        
        TMR3H = set_time >> 8;
        TMR3L = set_time & 0b11111111;
        
    }
    
    if(process3 == 1){
        TMR3ON = 1;
    }
    isr3 = 0;
    
	return;
}

void set_external_interrupt3(int time) {
	// set interrupt for external timer3 for time in us to control servo
	unsigned int set_time = 65535 - (time*2/4); // with oscillation at 8MHz, prescaler 4
    
    TMR3H = set_time >> 8;
    TMR3L = set_time & 0b11111111;
    
    TMR3ON = 1; // turn timer3 on
    if(isr1 == 1){
        
        set_time = 65535 - (20000*2/4);
        
        TMR1H = set_time >> 8;
        TMR1L = set_time & 0b11111111;
        
    }
    
    if(process1 == 1){
        TMR1ON = 1;
    }
    isr1 = 0;
    
    return;
}

void servo_ISR_1(){ //controlling the rotating disk of AA and 9V using timer 1
    TMR1IF = 0; //clear_interrupt;
    
    if(LATCbits.LATC2 == 1){ //finish sending the pulse that was being sent
        TMR3ON = 1; //turn the timer back on
        while(TMR3IF == 0){ //wait until the timer overflows
            
        }
        TMR3IF = 0; //clear timer interrupt flag
        TMR3ON = 0; //turn off timer
        count3++; //this counts as one pulse sent
        LATCbits.LATC2 = 0; //now the pin is low
        current_state3 = 0; 
        isr3 = 1; //set the isr flag
    }
    
    if(LATCbits.LATC7 == 1){
        TMR3ON = 1;
        while(TMR3IF == 0){
            
        }
        TMR3IF = 0;
        TMR3ON = 0;
        count3++;
        current_state3 = 0;
        isr3 = 1;
        LATCbits.LATC7 = 0;
    }
    
    if(stage1 == 1){ //TOP receiving the battery

        if(volt_check1 == 0){ //check the bottom break beam
            
            voltage_3 = readADC(3);
            volt_check1 = 1;
            
            if(voltage_3 > 3.5){ //if there is no battery, process is one and return
                process1 = 0; //process is done
                volt_check1 = 0;
                if(process3 == 1){
                   TMR3ON = 1; 
                }
                return;
            }
            set_external_interrupt1(1000);
            return;
        }
        
        if (count1 < 40){ //moving to centre, 50 iterations
            if(current_state1 == 1){ //if high, set to low

                current_state1 = 0;
                down_period1 = 20000; //20 ms
                LATCbits.LATC1 = 0;
                set_external_interrupt1(down_period1);

            }
            else if (current_state1 == 0){ //if low, set to high

                current_state1 = 1;
                up_period1 = 1420; //1.42 ms
                
                if(process3 == 1){ //the other disk is also operating,
                    //therefore need to account for power drop
                    up_period1 = 1450; //1.45 ms
                }
                LATCbits.LATC1 = 1;
                set_external_interrupt1(up_period1);

            }
            count1++; //increment
            return;
        }
        
        count1 = 0; //re-initialize
        LATCbits.LATC1 = 0;
        current_state1 = 0;
        set_external_interrupt1(20000);
        volt_check1 = 0;
        stage1 = 2; //moved to centre
        return;
    }
    
    if(stage1 == 2){ //CENTRE checking the voltage
        
        if(volt_check1 == 0){ //flag to check voltage just once
            
            LATDbits.LATD1 = 1; //solenoid push
            __delay_ms(500);
            
            voltage_0 = readADC(0); //top break beam input
            
            if(voltage_0 > 3.5){ //there is AA
                
                LATCbits.LATC0 = 1; //let the current flow to the switch for AA measurement
                __delay_ms(100);
                voltage_6 = readADC(6); //AA input
                AA_or_9V = 1; //it is AA
                voltage_AA = voltage_6;
                volt_check1 = 1;
                set_external_interrupt1(1000);
                LATCbits.LATC0 = 0;
                LATDbits.LATD1 = 0; //solenoid pull
                return;
                
            }
            
            AA_or_9V = 2; //it is 9V
            voltage_1 = readADC(1); //9V input 1
            voltage_2 = readADC(2); //9V input 2
            
            if(voltage_1 >= voltage_2){
                voltage_9V = voltage_1*(1.33/1.26); //take whatever plate that is measuring the 9V
            }
            else{
                voltage_9V = voltage_2*(1.3/1.2);
            }
            
            volt_check1 = 1;
            set_external_interrupt1(1000);
            LATDbits.LATD1 = 0; //solenoid pull
            return;
            
        }
        
        if(AA_or_9V == 1){ //if AA
            
            if(voltage_AA < 1.23){ //if drained
                
                if (count1 < 40){ //move to bot
                    if(current_state1 == 1){     

                        current_state1 = 0;
                        down_period1 = 20000;
                        LATCbits.LATC1 = 0;
                        set_external_interrupt1(down_period1);

                    }
                    else if (current_state1 == 0){

                        current_state1 = 1;
                        up_period1 = 1800;
                        LATCbits.LATC1 = 1;
                        set_external_interrupt1(up_period1);

                    }
                    count1++;
                    return;
                }      
                
                count1 = 0;
                current_state1 = 0;
                LATCbits.LATC1 = 0;
                set_external_interrupt1(20000);
                
                stage1 = 3; //back to top
                count_drained++; //increment drained count 
                volt_check1 = 0; //re-initialize flags
                return;
                
            }
            if (count1 < 40){ //move to between centre and top
                if(current_state1 == 1){     

                    current_state1 = 0;
                    down_period1 = 20000;
                    LATCbits.LATC1 = 0;
                    set_external_interrupt1(down_period1);

                }
                else if (current_state1 == 0){

                    current_state1 = 1;
                    up_period1 = 1100;
                    if(process3 == 1){
                        up_period1 = 1135;
                    }
                    LATCbits.LATC1 = 1;
                    set_external_interrupt1(up_period1);

                }
                count1++;
                return;
            }

            count1 = 0; //re-initialize
            current_state1 = 0;
            LATCbits.LATC1 = 0;
            volt_check1 = 0;
            count_AA++; //increment AA count
            stage1 = 4; //open and close the gate
            set_external_interrupt1(20000);
            return;
        }
        if(AA_or_9V == 2){ //if 9V
            
            if(voltage_9V > 1.25){ //if not drained
                
                stage1 = 4; //open and close gate, and move back to top
                set_external_interrupt1(1000);
                count_9V++; //increment 9V count
                volt_check1 = 0;
                return;

            }
            if (count1 < 40){ //move to bot if it is drained
                if(current_state1 == 1){     

                    current_state1 = 0;
                    down_period1 = 20000;
                    LATCbits.LATC1 = 0;
                    set_external_interrupt1(down_period1);

                }
                else if (current_state1 == 0){

                    current_state1 = 1;
                    up_period1 = 1800;
                    LATCbits.LATC1 = 1;
                    set_external_interrupt1(up_period1);

                }
                count1++;
                return;
            }
            count1 = 0; //re-initialize
            current_state1 = 0;
            LATCbits.LATC1 = 0;
            set_external_interrupt1(20000);
            
            volt_check1 = 0; //re-initialize flag
            count_drained++; //increment drained count
            
            stage1 = 3; //back to top
            return;
        }
    }
    if(stage1 == 3){ //after voltage measurement

        if (count1 < 60){ //back to TOP
            if(current_state1 == 1){     

                current_state1 = 0;
                down_period1 = 20000;
                LATCbits.LATC1 = 0;
                set_external_interrupt1(down_period1);

            }
            else if (current_state1 == 0){

                current_state1 = 1;
                up_period1 = 910;
                if(process3 == 1){
                    up_period1 = 950;
                }
                LATCbits.LATC1 = 1;
                set_external_interrupt1(up_period1);

            }
            count1++;
            return;
        }
        count1 = 0; //re-initialize
        LATCbits.LATC1 = 0;
        current_state1 = 0;
        stage1 = 1; //Back to TOP
        
        if(init1 == 0){
            init1 = 1;
            process1 = 0;
            if(process3 == 1){
                TMR3ON = 1;
            }
            return;
        }     
        
        set_external_interrupt1(20000);
        return;
    }
    if(stage1 == 4){ //open and close gate to the left

        if(gate_status1 == 0){ //open gate to the left
            if (count1 < 25){
                if(current_state1 == 1){     

                    current_state1 = 0;
                    down_period1 = 20000;
                    LATCbits.LATC6 = 0;
                    set_external_interrupt1(down_period1);

                }
                else if (current_state1 == 0){

                    current_state1 = 1;
                    up_period1 = 650;
                    if(process3 == 1){
                        up_period1 = 680;
                    }
                    LATCbits.LATC6 = 1;
                    set_external_interrupt1(up_period1);

                }
                count1++;
                return;
            }
            gate_status1 = 1; //flag that gate is open
            count1 = 0; //re-initialize
            LATCbits.LATC6 = 0;
            current_state1 = 0;
            set_external_interrupt1(20000);
            return;
        }
        if(gate_status1 == 1){ //close gate
            if (count1 < 25){
                if(current_state1 == 1){     

                    current_state1 = 0;
                    down_period1 = 20000;
                    LATCbits.LATC6 = 0;
                    set_external_interrupt1(down_period1);

                }
                else if (current_state1 == 0){

                    current_state1 = 1;
                    up_period1 = 1575;
                    if(process3 == 1){
                        up_period1 = 1605;
                    }
                    LATCbits.LATC6 = 1;
                    set_external_interrupt1(up_period1);

                }
                count1++;
                return;
            }
            gate_status1 = 0; //re-initialize
            count1 = 0;
            LATCbits.LATC6 = 0;
            current_state1 = 0;
            set_external_interrupt1(20000);
            stage1 = 3; //go back to top
            return;
        }
    }
    return;
}

void servo_ISR_3(){ //C
        
    TMR3IF = 0;
    
    if(LATCbits.LATC1 == 1){ //make the servo less jiggly
        TMR1ON = 1;
        while(TMR1IF == 0){
            
        }
        TMR1IF = 0;
        TMR1ON = 0;
        count1++;
        current_state1 = 0;
        LATCbits.LATC1 = 0;
        isr1 = 1;
    }
    
    if(LATCbits.LATC6 == 1){
        TMR1ON = 1;
        while(TMR1IF == 0){
            
        }
        TMR1IF = 0;
        TMR1ON = 0;
        count1++;
        current_state1 = 0;
        LATCbits.LATC6 = 0;
        isr1 = 1;
    }

    if(stage3 == 1){ //TOP receiving the battery
        
        if(volt_check3 == 0){
            
            voltage_4 = readADC(4); //read bream beam input
            volt_check3 = 1; //the input is checked
            if(voltage_4 > 3.5){ //if there is no battery, process is done and return
                process3 = 0; //process is done
                volt_check3 = 0; //initialize
                if(process1 == 1){
                   TMR1ON = 1; //turn timer back on
                }
                return;
            }
            set_external_interrupt3(1000);
            return;
            
        }
        
        if (count3 < 40){ //moving to centre, 40 iterations
            if(current_state3 == 1){ //if high, set to low

                current_state3 = 0;
                down_period3 = 20000;
                LATCbits.LATC2 = 0;
                set_external_interrupt3(down_period3);

            }
            else if (current_state3 == 0){ //if low, set to high

                current_state3 = 1;
                up_period3 = 1620; //1.62 ms
                LATCbits.LATC2 = 1;
                set_external_interrupt3(up_period3);

            }
            count3++; //increment
            return;
        }
        count3 = 0; //re-initialize
        LATCbits.LATC2 = 0;
        current_state3 = 0;
        set_external_interrupt3(20000);
        volt_check3 = 0;
        stage3 = 2; //moved to centre
        return;
    }
    
    if(stage3 == 2){ //45 degrees checking the voltage
        
        if(volt_check3 == 0){ //flag to check voltage just once
            
            LATCbits.LATC5 = 1; //solenoid push
            __delay_ms(500);
            voltage_C = readADC(5); //voltage measurements from port RA4
            
            volt_check3 = 1; //set flag
            set_external_interrupt3(1000);
            
            LATCbits.LATC5 = 0; //solenoid pull
            
            return;
        }
       
        if(voltage_C > 1.23){ //if not drained
            stage3 = 4; //open and close gate to the left
            count_C++; //increment C count
            volt_check3 = 0;
            set_external_interrupt3(1000);
            return;
        }
        
        if (count3 < 40){ //move to 180 degrees if drained
            if(current_state3 == 1){

                current_state3 = 0;
                down_period3 = 20000;
                LATCbits.LATC2 = 0;
                set_external_interrupt3(down_period3);

            }
            else if (current_state3 == 0){

                current_state3 = 1;
                up_period3 = 900; //0.9 ms
                LATCbits.LATC2 = 1;
                set_external_interrupt3(up_period3);

            }
            count3++;
            return;
        }            
        count3 = 0; //re-initialize
        current_state3 = 0;
        LATCbits.LATC2 = 0;
        set_external_interrupt3(20000);

        count_drained++; //increment drained count
        volt_check3 = 0; //re-initialize flag
        stage3 = 3; //back to top
        
        return;
    }
    if(stage3 == 3){ //after voltage measurement
        if (count3 < 60){ //back to 0 degrees
            if(current_state3 == 1){

                current_state3 = 0;
                down_period3 = 20000;
                LATCbits.LATC2 = 0;
                set_external_interrupt3(down_period3);

            }
            else if (current_state3 == 0){

                current_state3 = 1;
                up_period3 = 2405; //2.405 ms
                LATCbits.LATC2 = 1;
                set_external_interrupt3(up_period3);

            }
            count3++;
            return;
        }

        count3 = 0; //re-initialize
        LATCbits.LATC2 = 0;
        current_state3 = 0;
        stage3 = 1; //In 0 degrees position
        
        if(init3 == 0){
            init3 = 1; //Set flag indicating that initialization is done
            process3 = 0;
            if(process1 == 1){ //if bottom disk initialization is still in process
                TMR1ON = 1; //turn Timer 1 on 
            }
            return;
        }     
        
        set_external_interrupt3(20000);
        return;
    }
    if(stage3 == 4){ //open and close gate
        if(gate_status3 == 0){ //open gate to the left
            if (count3 < 25){
                if(current_state3 == 1){     

                    current_state3 = 0;
                    down_period3 = 20000;
                    LATCbits.LATC7 = 0;
                    set_external_interrupt3(down_period3);

                }
                else if (current_state3 == 0){

                    current_state3 = 1;
                    up_period3 = 2000;
                    LATCbits.LATC7 = 1;
                    set_external_interrupt3(up_period3);

                }
                count3++;
                return;
            }
            gate_status3 = 1; //flag that gate is open
            count3 = 0; //re-initialize
            LATCbits.LATC7 = 0;
            current_state3 = 0;
            set_external_interrupt3(20000);
            return;
        }
        if(gate_status3 == 1){ //close gate
            if (count3 < 25){
                if(current_state3 == 1){     

                    current_state3 = 0;
                    down_period3 = 20000;
                    LATCbits.LATC7 = 0;
                    set_external_interrupt3(down_period3);

                }
                else if (current_state3 == 0){

                    current_state3 = 1;
                    up_period3 = 3200;
                    LATCbits.LATC7 = 1;
                    set_external_interrupt3(up_period3);

                }
                count3++;
                return;
            }
            gate_status3 = 0; //re-initialize
            count3 = 0;
            LATCbits.LATC7 = 0;
            current_state3 = 0;
            stage3 = 3;
            set_external_interrupt3(20000);
            return;
        }
    }
    return;
}