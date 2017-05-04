/*
 * File:   variables.c
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
#include "eeprom.h"

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