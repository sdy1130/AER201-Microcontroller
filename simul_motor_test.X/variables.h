#ifndef VARIABLES_H
#define	VARIABLES_H

volatile int up_period_1; //high and low duration
volatile int down_period_1;
volatile int up_period_3;
volatile int down_period_3;
volatile unsigned char current_state0; //low initial state
volatile unsigned char current_state1;
volatile unsigned char current_state3;
//extern volatile unsigned char stage0; //1 top, 2 centre, 3 bot, 4 gate open/close left, 54 gate open/close right
volatile unsigned char stage1;
volatile unsigned char stage3;
volatile int count0;
volatile int count1;
volatile int count3;
//extern volatile unsigned char gate_status0; //0 closed, 1 open
volatile unsigned char gate_status1;
volatile unsigned char gate_status3;
//extern volatile unsigned char gate_flag0; //1 if gate opened and closed
volatile unsigned char gate_flag1;
volatile unsigned char gate_flag3;
volatile float voltage_0; //voltage measurement
volatile float voltage_1;
volatile float voltage_2;
volatile float voltage_3;
volatile float voltage_AA;
volatile float voltage_9V;
volatile float voltage_C;
volatile unsigned char AA_or_9V; //1 if AA, 2 if 9V
//extern volatile unsigned char disable0; //1 if timer is disabled
volatile unsigned char disable1;
volatile unsigned char disable3;
//extern volatile unsigned char volt_check0; //1 if voltage is checked
volatile unsigned char volt_check1;
volatile unsigned char volt_check3;
//extern volatile unsigned char delay0; // 0 delay, 1 skip
//extern volatile unsigned char delay1;
//extern volatile unsigned char delay3;
volatile int count_AA; //number of batteries
volatile int count_9V;
volatile int count_C;
volatile int count_drained;
volatile char beam_check1;
volatile char beam_check3;

#endif	/* VARIABLES_H */