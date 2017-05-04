/*
 * File:   main.c
 * Author: sdy1130
 *
 * Created on February 26, 2017, 2:00 PM
 */

#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "ADC.h"
#include "interrupt.h"
#include "variables.h"
#include "eeprom.h"

void set_time(void);

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

const char keys[] = "123A456B789C*0#D";
const char happynewyear[7] = {  0x00, //Seconds 
                            0x33, //Minutes
                            0x3, //24 hour mode
                            0x01, //Monday 
                            0x10, //10th
                            0x04, //April
                            0x17};//2017

void main(void) {
    OSCCON = 0xF0; //8Mhz
    
    
    TRISA = 0xFF; // Set Port A as input
    TRISB = 0xFF;
    TRISC = 0x00; // Set Port C as output
    TRISD = 0x00; //All output mode for LCD
    TRISE = 0b00000011; //RE0 and RE1 as inputs

    LATA = 0x00; //All pins set to 0
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0b00001000;  //Set AN1-AN6 as Analog Inputs
    CVRCON = 0x00; // Disable CCP reference voltage output
    CMCONbits.CIS = 0;
    ADFM = 1;
    
    nRBPU = 0;
    
    T1CON = 0b10100100; // 16-bit operation, internal clock 8 MHz, prescale - 1:4 
    T3CON = 0b10100100;
    TMR1ON = 0; //disable timer1
    TMR1IF = 0; //clear interrupt flag
    TMR3ON = 0; //disable timer3
    TMR3IF = 0; //clear interrupt flag
    TMR1IE = 1; //enable interrupts
    TMR3IE = 1;
    PEIE = 1; //enable peripheral interrupt
    
    unsigned char time[7];
    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
    //set_time();
    
    initLCD();
    
    while(1){
        
        ei();
        isr1 = 0;
        isr3 = 0;
        current_state1 = 0;
        current_state3 = 0;
        //stage0 = 1; //1 top, 2 centre, 3 bot, 4 gate open&close
        stage1 = 4; //for initialization
        stage3 = 4;
        //count0 = 0;
        count1 = 0;
        count3 = 0;
        //gate_status0 = 0; //0 closed, 1 open
        gate_status1 = 1; //for initialization, close gate
        gate_status3 = 1;
        init1 = 0;
        init3 = 0;
        voltage_0 = 0; //voltage measurement
        voltage_1 = 0;
        voltage_2 = 0;
        voltage_3 = 0;
        voltage_AA = 0;
        voltage_9V =0;
        voltage_C = 0;
        AA_or_9V = 0; //1 if AA, 2 if 9V
        //volt_check0 = 0; //1 if voltage is checked
        volt_check1 = 0;
        volt_check3 = 0;
        count_AA = 0; //number of batteries
        count_9V = 0;
        count_C = 0;
        count_drained = 0;
        //disable0 = 0; //1 if timer is disabled
        process1 = 0; //for initialization
        process3 = 0;
        
        unsigned char flag_interrupt = 0; //1 if a key is pressed
        unsigned char done = 0; //1 if sorting is done
        
        unsigned char address_min = 0x00; //eeprom starting addresses, 0x40 apart
        unsigned char address_seconds = 0x28;
        unsigned char address_AA = 0x90;
        unsigned char address_9V = 0xB8;
        unsigned char address_C = 0xE0;
        unsigned char address_drained = 0x130;
        unsigned char address_total = 0x108;

        signed char eeprom_min; //eeprom values
        signed char eeprom_seconds;
        signed char eeprom_AA;
        signed char eeprom_9V;
        signed char eeprom_C;
        signed char eeprom_drained;
        signed char eeprom_total;
        
        unsigned char count_min_initial = 0; //time values
        unsigned char count_seconds_initial = 0;
        unsigned char count_min_final = 0;
        unsigned char count_seconds_final =  0;
        signed char count_min = 0;
        signed char count_seconds = 0;
        
        signed char past_min; //EEPROM display
        signed char past_seconds;
        signed char past_total;
        signed char past_AA;
        signed char past_9V;
        signed char past_C;
        signed char past_drained;

        while(PORTBbits.RB1 == 0){ 
            // RB1 is the interrupt pin, so if there is no key pressed, RB1 will be 0
            // the PIC will wait and do nothing until a key press is signaled
            I2C_Master_Start(); //Start condition
            I2C_Master_Write(0b11010000); //7 bit RTC address + Write
            I2C_Master_Write(0x00); //Set memory pointer to seconds
            I2C_Master_Stop(); //Stop condition

            //Read Current Time
            I2C_Master_Start();
            I2C_Master_Write(0b11010001); //7 bit RTC address + Read
            for(unsigned char j=0;j<0x06;j++){
                time[j] = I2C_Master_Read(1);
            }
            time[6] = I2C_Master_Read(0);       //Final Read without ack
            I2C_Master_Stop();
            
            __lcd_home();
            printf("%02x/%02x/%02x ", time[6],time[5],time[4]);    //Print date in YY/MM/DD
            printf("1:Start");
            __lcd_newline();
            printf("%02x:%02x:%02x ", time[2],time[1],time[0]);    //HH:MM:SS
            printf("2:Logs");

            flag_interrupt = delay_1s_interrupt(1); //if interrupt (pressing a button) occurs, break the loop
            if(flag_interrupt == 1){
                break;
            }
        }
        unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code
        
        while(PORTBbits.RB1 == 1){
            continue;
            //Wait until the key has been released
        }
        
        if (keys[keypress] == keys[0]){ //if key 1 is pressed

            I2C_Master_Start(); //Start condition
            I2C_Master_Write(0b11010000); //7 bit RTC address + Write
            I2C_Master_Write(0x00); //Set memory pointer to seconds
            I2C_Master_Stop(); //Stop condition
            //Read Current Time
            I2C_Master_Start();
            I2C_Master_Write(0b11010001); //7 bit RTC address + Read
            for(unsigned char k=0;k<0x06;k++){
                time[k] = I2C_Master_Read(1);
            }
            time[6] = I2C_Master_Read(0);       //Final Read without ack
            I2C_Master_Stop();

            count_min_initial = time[1]; //fetch real time minutes
            count_seconds_initial = time[0]; //fetch real time seconds
            
            //convert min and sec measurement value to decimal
            count_min_initial = (count_min_initial & 0x0F) + ((count_min_initial & 0xF0)>>4)*10;
            count_seconds_initial = (count_seconds_initial & 0x0F) + ((count_seconds_initial & 0xF0)>>4)*10;
            
            initLCD();
            printf("Initializing...");

            process1 = 1;
            set_external_interrupt1(500); //move the stuff to top
            process3 = 1;
            set_external_interrupt3(700);

            while(init1 == 0 || init3 == 0){
                continue;
            }

            initLCD();
            printf("Sorting...");
            
            int loop_count; //loop
            
            while(done == 0){//wait until the operation is done          
                //while loop this 
                
                while(process1 == 0 && process3 == 0 && loop_count < 7 && count_min < 3){ 
                    // break loop if there are no batteries or the DC motor has turned 7 times
                    // or the operation time has elapsed more than 3 minutes
                    di(); //disable interrupts
                    
                    LATDbits.LATD0 = 1; //DC on and off
                    __delay_ms(100);
                    LATDbits.LATD0 = 0;
                    
                    __delay_1s(); //wait for the battery to be dropped
                    __delay_1s();
                    __delay_1s();
                    
                    //break beam inputs
                    voltage_3 = readADC(3); //check if there is AA or 9V
                    voltage_4 = readADC(4); //check if there is C

                    if(voltage_3 < 3.5){
                        process1 = 1; //bottom plate activated
                        ei(); //enable interrupts
                        set_external_interrupt1(700); //initialize interrupts

                    }
                    if(voltage_4 < 3.5){                  
                        process3 = 1; //top plate activated
                        ei();
                        set_external_interrupt3(500);
                    }
                    loop_count++; //increment loop count
                }

                I2C_Master_Start(); //Start condition
                I2C_Master_Write(0b11010000); //7 bit RTC address + Write
                I2C_Master_Write(0x00); //Set memory pointer to seconds
                I2C_Master_Stop(); //Stop condition
                //Read Current Time
                I2C_Master_Start();
                I2C_Master_Write(0b11010001); //7 bit RTC address + Read
                for(unsigned char k=0;k<0x06;k++){
                    time[k] = I2C_Master_Read(1);
                }
                time[6] = I2C_Master_Read(0);       //Final Read without ack
                I2C_Master_Stop();
                
                count_min_final = time[1]; //fetch the most recent real time minutes
                count_seconds_final =  time[0]; //fetch the most recent real time seconds
                count_min_final = (count_min_final & 0x0F) + ((count_min_final & 0xF0)>>4)*10;
                count_seconds_final = (count_seconds_final & 0x0F) + ((count_seconds_final & 0xF0)>>4)*10;
                
                count_min = count_min_final - count_min_initial;
                count_seconds = count_seconds_final - count_seconds_initial;
                
                if(count_min < 0){

                    count_min = count_min + 60;

                }
                
                if(count_seconds < 0){

                    count_min = count_min - 1;
                    count_seconds = count_seconds + 60;

                }
                
                if(loop_count == 7 || count_min > 2){
                //if the DC motor turned 7 times or operation time exceed 3 minutes
                    
                    done = 1; //set done flag

                }
                
                loop_count = 0; //reinitialize
            }
            
           //disable timer interrupts
            di();
            TMR1ON = 0;
            TMR3ON = 0;
            TMR1IF = 0;
            TMR3IF = 0;
            //stop all motors
            LATA = 0x00;
            LATB = 0x00;
            LATC = 0x00;
            LATDbits.LATD0 = 0;
            LATDbits.LATD1 = 0;
            LATE = 0x00;
            Nop();  //Apply breakpoint here because of compiler optimizations
            Nop();

            I2C_Master_Start(); //Start condition
            I2C_Master_Write(0b11010000); //7 bit RTC address + Write
            I2C_Master_Write(0x00); //Set memory pointer to seconds
            I2C_Master_Stop(); //Stop condition
            //Read Current Time
            I2C_Master_Start();
            I2C_Master_Write(0b11010001); //7 bit RTC address + Read
            for(unsigned char k=0;k<0x06;k++){
                time[k] = I2C_Master_Read(1);
            }
            time[6] = I2C_Master_Read(0);       //Final Read without ack
            I2C_Master_Stop();

            count_min_final = time[1]; //fetch the real time minutes at the end of operation
            count_seconds_final =  time[0]; //fetch the real time seconds at the end of operation
            count_min_final = (count_min_final & 0x0F) + ((count_min_final & 0xF0)>>4)*10;
            count_seconds_final = (count_seconds_final & 0x0F) + ((count_seconds_final & 0xF0)>>4)*10;

            count_min = count_min_final - count_min_initial; //take difference (end - start)
            count_seconds = count_seconds_final - count_seconds_initial;
            
            if(count_min < 0){ //negative means the hour changed

                count_min = count_min + 60;

            }
            
            if(count_seconds < 0){ //negative means the minute chaned

                count_min = count_min - 1;
                count_seconds = count_seconds + 60;

            }

            //starting addresses; 0x40 apart
            //min  sec  AA   9V   C    total drained
            //0x00 0x28 0x90 0xB8 0xE0 0x108  0x130

            for(unsigned char i=0; i<3; i++){ //shift the data in memory down by 0x10
                eeprom_min = Eeprom_ReadByte(address_min + 10);
                Eeprom_WriteByte(address_min, eeprom_min);
                eeprom_seconds = Eeprom_ReadByte(address_seconds + 10);
                Eeprom_WriteByte(address_seconds, eeprom_seconds);
                eeprom_AA = Eeprom_ReadByte(address_AA + 10);
                Eeprom_WriteByte(address_AA, eeprom_AA);
                eeprom_9V = Eeprom_ReadByte(address_9V + 10);
                Eeprom_WriteByte(address_9V, eeprom_9V);
                eeprom_C = Eeprom_ReadByte(address_C + 10);
                Eeprom_WriteByte(address_C, eeprom_C);
                eeprom_drained = Eeprom_ReadByte(address_drained + 10);
                Eeprom_WriteByte(address_drained, eeprom_drained);
                eeprom_total = Eeprom_ReadByte(address_total + 10);
                Eeprom_WriteByte(address_total, eeprom_total);

                address_min = address_min + 10;
                address_seconds = address_seconds + 10;
                address_AA = address_AA + 10;
                address_9V = address_9V + 10;
                address_C = address_C + 10;
                address_drained = address_drained + 10;
                address_total = address_total + 10;
            }

            Eeprom_WriteByte(address_min, count_min);
            Eeprom_WriteByte(address_seconds, count_seconds);
            Eeprom_WriteByte(address_AA, count_AA);
            Eeprom_WriteByte(address_9V, count_9V);
            Eeprom_WriteByte(address_C, count_C);
            Eeprom_WriteByte(address_drained, count_drained);
            Eeprom_WriteByte(address_total, count_AA + count_9V + count_C + count_drained);

            initLCD(); //print logs
            printf("%02d", count_min);
            printf(" minutes");
            __lcd_newline();
            printf("%02d", count_seconds);
            printf(" seconds");
            __delay_1s();
            __delay_1s();
            initLCD();
            printf("AA: %02d ", count_AA);
            printf("9V: %02d", count_9V);
            __lcd_newline();
            printf(" C: %02d  ", count_C);
            printf("X: %02d", count_drained);
            __delay_1s();
            __delay_1s();
            initLCD();
            printf("total: %02d", (count_AA + count_9V + count_C + count_drained));
            __delay_1s();
            __delay_1s();

        }
        else if (keys[keypress] == keys[1]){ //key "2" pressed, EEPROM Past Sorting Logs
            
            address_min = address_min + 30; //set address to the most recent log
            address_seconds = address_seconds + 30;
            address_total = address_total + 30;
            address_AA = address_AA + 30;
            address_9V = address_9V + 30;
            address_C = address_C + 30;
            address_drained = address_drained + 30;
            
            while(keys[keypress] != keys[13]){ //press "0" to go back
                
                past_min = Eeprom_ReadByte(address_min); //read data
                past_seconds = Eeprom_ReadByte(address_seconds);
                past_total = Eeprom_ReadByte(address_total);
                past_AA = Eeprom_ReadByte(address_AA);
                past_9V = Eeprom_ReadByte(address_9V);
                past_C = Eeprom_ReadByte(address_C);
                past_drained = Eeprom_ReadByte(address_drained);

                initLCD(); //display log
                printf("%02d", past_min);
                printf(" min  ");
                printf("%02d", past_seconds);
                printf(" sec");
                __lcd_newline();
                printf("total: %02d", past_total);

                while(PORTBbits.RB1 == 0){ //wait for key press
                    continue;
                }        
                keypress = (PORTB & 0xF0)>>4;// check which key
                while(PORTBbits.RB1 == 1){ //wait until key is released
                    continue;
                }
                
                if(keys[keypress] == keys[3]){ //press "A" to see details of the log
                    
                    initLCD();
                    printf("AA: %02d ", past_AA);
                    printf("9V: %02d", past_9V);
                    __lcd_newline();
                    printf(" C: %02d  ", past_C);
                    printf("X: %02d", past_drained);
                    
                    while(PORTBbits.RB1 == 0){ //press any key to go back
                        continue;
                    }        

                    while(PORTBbits.RB1 == 1){ //wait until key is released
                        continue;
                    }
                }
                
                if(keys[keypress] == keys[12]){ //press "#" to view the later
                    if(address_min < 0x00+30){
                        address_min = address_min + 10;
                        address_seconds = address_seconds + 10;
                        address_AA = address_AA + 10;
                        address_9V = address_9V + 10;
                        address_C = address_C + 10;
                        address_drained = address_drained + 10;
                        address_total = address_total + 10;                    
                    }
                }
                
                if(keys[keypress] == keys[14]){ //press "*" to view the earlier
                    if(address_min > 0x00){
                        address_min = address_min - 10;
                        address_seconds = address_seconds - 10;
                        address_AA = address_AA - 10;
                        address_9V = address_9V - 10;
                        address_C = address_C - 10;
                        address_drained = address_drained - 10;
                        address_total = address_total - 10;
                    }
                }
            }
        }
    }
    return;
}

void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<7; i++){
        I2C_Master_Write(happynewyear[i]);
    }    
    I2C_Master_Stop(); //Stop condition
}

void interrupt IRQ(void) {
    di();
    TMR3ON = 0; //turn timers off during the ISR
    __delay_us(1);
    TMR1ON = 0;
    
    //check which timer caused the interrupt
    if (TMR1IF) {
            servo_ISR_1(); //bottom disk subroutine
            ei();
            return;
        }
    if (TMR3IF) {
            servo_ISR_3(); //top disk subroutine
            ei();
            return; //return to main code
        }

}