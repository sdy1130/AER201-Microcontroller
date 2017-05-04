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

float readADC(char channel){

    ADCON0 = ((channel <<2));
    ADON = 1;
    ADCON0bits.GO = 1;
    
    while(ADCON0bits.GO_NOT_DONE){__delay_ms(2);}

    int v = ADRES;
    float volt = 0.00244140625*v*(1.62/0.981440);
   
    return volt;
}