/*
 * File:   main.c
 * Author: True Administrator
 *
 * Created on July 18, 2016, 12:11 PM
 */


#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"

const char keys[] = "123A456B789C*0#D";
const char happynewyear[7] = {  0x00, //0 Seconds 
                        0x32, //Minutes
                        0x00, //24 hour mode, set to 00:00
                        0x07, //Friday 
                        0x28, //day
                        0x01, //January
                        0x17};//2017

void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<7; i++){
        I2C_Master_Write(happynewyear[i]);
    }    
    I2C_Master_Stop(); //Stop condition
}

void main(void) {
    OSCCON = 0xF0;  //8Mhz
    
    // RTC uses RC3 and RC4
    // LCD uses RD2 - RD7
    // KEYPAD uses RB4 - RB7 and RB1
    
    TRISA = 0x00; // Set Port A as all output
    TRISB = 0b11110010; // Set Keypad Pins as input, rest are output
    TRISC = 0x00011000; // Set I2C pins as input, rest are output
    TRISD = 0x00;   //All output mode
    TRISE = 0x00;   //RE0 and RE1 output

    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0xFF;  //Set PORTB to be digital instead of analog default  
    
    // nRBPU = 0;
    initLCD(); // Initialize LCD
    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
    di(); // Disable all interrupts
    unsigned char time[7];
    
    //set_time();
    
    // Set the output ports to have alternating outputs
    LATA = 0b01010101;
    LATB = 0b01010101;
    LATC = 0b01010101;
    LATDbits.LATD0 = 1;
    LATDbits.LATD1 = 0;
    LATEbits.LATE0 = 1;
    LATEbits.LATE1 = 0;

    
    int i = 0; // Loop counter i
    
    while(1){
        //Reset RTC memory pointer 
        I2C_Master_Start(); //Start condition
        I2C_Master_Write(0b11010000); //7 bit RTC address + Write
        I2C_Master_Write(0x00); //Set memory pointer to seconds
        I2C_Master_Stop(); //Stop condition

        //Read Current Time
        I2C_Master_Start();
        I2C_Master_Write(0b11010001); //7 bit RTC address + Read
        for(unsigned char i=0;i<0x06;i++){
            time[i] = I2C_Master_Read(1);
        }
        time[6] = I2C_Master_Read(0);       //Final Read without ack
        I2C_Master_Stop();
        __lcd_home();
        printf("%02x/%02x/%02x", time[6],time[5],time[4]);    //Print date in YY/MM/DD
        __lcd_newline(); //new line
        printf("%02x:%02x:%02x", time[2],time[1],time[0]);    //HH:MM:SS
        __lcd_home();
        
        for (i = 0; i < 200; i++){
        if (PORTBbits.RB1 != 0){
        unsigned char keypress = (PORTB & 0xF0)>>4;
        while(PORTBbits.RB1 == 1){
        }
        Nop();  //Apply breakpoint here because of compiler optimizations
        Nop();
        unsigned char LCDtemp = keys[keypress];
        putch(LCDtemp);   
        }
        delay_10ms(1);
      }
        
        // Flip State of all unused pins
        LATA = LATA ^ 0xFF;
        LATB = LATB ^ 0xFF;
        LATC = LATC ^ 0xFF;
        LATDbits.LATD0 = !LATDbits.LATD0;
        LATDbits.LATD1 = !LATDbits.LATD1;
        LATEbits.LATE0 = !LATEbits.LATE0;
        LATEbits.LATE1 = !LATEbits.LATE1;
        
        
    }
    
    // This statement is here incase the main loop breaks, the return
    //statement will initiate a soft reset
    return; 
}