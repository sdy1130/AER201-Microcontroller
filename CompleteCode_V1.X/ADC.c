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
#include "ADC.h"
#include "interrupt.h"
#include "variables.h"
#include "eeprom.h"

signed float abs(signed float num){
    if(num > 0){
        return num;
    }
    return (-1)*num; //make negative number positive
}

void read(char channel2){
    // Select A2D channel to read
    ADCON0 = ((channel2 << 2));
    ADON = 1;
    ADCON0bits.GO = 1;
   while(ADCON0bits.GO_NOT_DONE){__delay_ms(2);}
}

signed float readADC(char channel){ //convert analog to digital
    
    signed float a = 0;

    for(unsigned char i = 0; i < 100; i++){ //take 100 cumulative measurements
        
        read(channel);
        a = a + ADRES;

        __delay_us(1);
    }

    a = a/100; //take average measurement
    
    signed float volt = a*4.957152/1024; //the voltage conversion

    if(channel == 1 || channel == 2 || channel == 5 || channel == 6){ //voltage measuring inputs
        volt = abs(volt - 2.5);
    }
    
    return volt;

}