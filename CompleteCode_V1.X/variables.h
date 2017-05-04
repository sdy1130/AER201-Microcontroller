#ifndef VARIABLES_H
#define	VARIABLES_H

volatile int up_period1; //high and low duration
volatile int down_period1;
volatile int up_period3;
volatile int down_period3;
volatile unsigned char current_state1;
volatile unsigned char current_state3;
volatile unsigned char stage1;//1 top, 2 centre, 3 bot, 4 gate open/close
volatile unsigned char stage3;
volatile int count1; //pulse counter
volatile int count3;
volatile unsigned char gate_status1; //1 if open, 0 if not
volatile unsigned char gate_status3;
volatile unsigned char init1; //initialization flag
volatile unsigned char init3;
volatile float voltage_1; //voltage input from pins
volatile float voltage_2;
volatile float voltage_3;
volatile float voltage_4;
volatile float voltage_6;
volatile float voltage_AA; //measured battery voltage
volatile float voltage_9V;
volatile float voltage_C;
volatile unsigned char AA_or_9V; //1 if AA, 2 if 9V
volatile unsigned char volt_check1;
volatile unsigned char volt_check3;
volatile int count_AA; //number of batteries
volatile int count_9V;
volatile int count_C;
volatile int count_drained;
volatile char process1; //1 if process is on, 0 if not
volatile char process3;
//1 if pin was set high when other timer's interrupt occurred
volatile unsigned char isr1; 
volatile unsigned char isr3;

#endif	/* VARIABLES_H */