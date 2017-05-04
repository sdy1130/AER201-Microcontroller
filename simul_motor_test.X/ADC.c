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
#include "macros.h"
#include "ADC.h"
#include "interrupt.h"
#include "variables.h"

signed float abs(signed float num){
    if(num > 0){
        return num;
    }
    return -num;
}

void read(char channel2){
    // Select A2D channel to read
    ADCON0 = ((channel2 << 2));
    ADON = 1;
    ADCON0bits.GO = 1;
   while(ADCON0bits.GO_NOT_DONE){__delay_ms(2);}
}

float readADC(char channel){ //convert analog to digital
    
    signed float a = 0;

    for(unsigned char i = 0; i < 100; i++){
        
        read(channel);
        a = a + ADRES;

        __delay_ms(1);
        
    }
    
    a = a/100;
    signed float volt = a*4.957152/1023*2.5/2.96;
    //volt = volt - 2.5;
    initLCD();
    printf("%f" ,volt - 2.5);
    __delay_1s();
    //if(0.9 < volt && volt < 4.1){
    
    signed float volt2 = abs(volt - 2.5);
    
        
        //if(channel == 2){
            //volt = (1/1.5)*volt;
        //}
        
    return volt2;
        
    //}
    
    //return 0;

}