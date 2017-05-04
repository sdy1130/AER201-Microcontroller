#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "ADC.h"
#include "interrupt.h"

//THINGS TO DO:
//Make a header and c files for other functions outside of main
//Standby mode interface is controlled not by the timer, but by left and right buttons
//Better interface for sorting logs
//AD converter for signals input and output (voltage measuring (ADC clear!), gate opening, disk turning)
//      How to multitask?: http://electronics.stackexchange.com/questions/60675/carrying-out-multiple-synchronous-tasks-with-a-micrcontroller
//PC Download

void set_time(void);

unsigned char key_pressed_check(){ //check if a key is REALLY pressed
    while(PORTBbits.RB1 == 0){
        continue;
    }
    unsigned char check = PORTBbits.RB1;
    delay_10ms(2);
    
    if(check == PORTBbits.RB1){
        return 1;
    }
    return 0;
}

signed char Eeprom_ReadByte(signed char address)
{

    // Set address registers
    EEADRH = (signed char)(address >> 8);
    EEADR = (signed char)address;

    EECON1bits.EEPGD = 0;       // Select EEPROM Data Memory
    EECON1bits.CFGS = 0;        // Access flash/EEPROM NOT config. registers
    EECON1bits.RD = 1;          // Start a read cycle

    // A read should only take one cycle, and then the hardware will clear
    // the RD bit
    while(EECON1bits.RD == 1);

    return EEDATA;              // Return data

}

void Eeprom_WriteByte(signed char address, signed char data)
{    
    // Set address registers
    EEADRH = (signed char)(address >> 8);
    EEADR = (signed char)address;

    EEDATA = data;          // Write data we want to write to SFR
    EECON1bits.EEPGD = 0;   // Select EEPROM data memory
    EECON1bits.CFGS = 0;    // Access flash/EEPROM NOT config. registers
    EECON1bits.WREN = 1;    // Enable writing of EEPROM (this is disabled again after the write completes)

    // The next three lines of code perform the required operations to
    // initiate a EEPROM write
    EECON2 = 0x55;          // Part of required sequence for write to internal EEPROM
    EECON2 = 0xAA;          // Part of required sequence for write to internal EEPROM
    EECON1bits.WR = 1;      // Part of required sequence for write to internal EEPROM

    // Loop until write operation is complete
    while(PIR2bits.EEIF == 0)
    {
        continue;   // Do nothing, are just waiting
    }

    PIR2bits.EEIF = 0;      //Clearing EEIF bit (this MUST be cleared in software after each write)
    EECON1bits.WREN = 0;    // Disable write (for safety, it is re-enabled next time a EEPROM write is performed)
}

const char keys[] = "123A456B789C*0#D";
const char happynewyear[7] = {  0x00, //45 Seconds 
                            0x11, //59 Minutes
                            0x13, //24 hour mode, set to 23:00
                            0x01, //Monday 
                            0x06, //31st
                            0x02, //December
                            0x17};//2016

void main(void) {
    OSCCON = 0xF0; //8Mhz
    
    // <editor-fold defaultstate="collapsed" desc=" STARTUP SEQUENCE ">
    
    TRISA = 0x11101111; // Set Port RA4 as output
    TRISB = 0xFF;
    TRISC = 0x00;
    TRISD = 0x00; //All output mode for LCD
    TRISE = 0x00;    

    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0x0B;  //Set PORTB to be digital instead of analog default  
    
    nRBPU = 0;
    
    // code to set up timer1
    T1CON = 0b10000000; // 16-bit operation
    // bit 6 is unimplemented
    // prescale - 1:8
    T1CKPS1 = 1;
    T1CKPS0 = 1;

    T1OSCEN = 0; // timer1 oscillator shut off
    T1SYNC = 1; // (bit is ignored when TMR1CS=0) do not synchronize with external clock input
    TMR1CS = 0; // timer mode
    TMR1ON = 0; // disable timer1 for now
    TMR1IE = 1; // disable interrupts
    TMR1IF = 0; //clear interrupt flag
    
    volatile unsigned char current_state = 0;
    volatile float up_period = 1.5;
    volatile float down_period = 20 - up_period;

    //</editor-fold>
    
    initLCD();
    
    unsigned char time[7];
    int flag_interrupt = 0;
    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
    set_time();
    
    unsigned char page = 0; //0: Real Time Clock, 1: Start, 2: Sorting Logs
    
    while(1){
        while(PORTBbits.RB1 == 0){ 
            // RB1 is the interrupt pin, so if there is no key pressed, RB1 will be 0
            // the PIC will wait and do nothing until a key press is signaled
            //Reset RTC memory pointer
            flag_interrupt = 0;
            
            initLCD();
            for(unsigned char i=0;i<0x06;i++){
                flag_interrupt = 0;
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
                printf("%02x/%02x/%02x", time[6],time[5],time[4]);    //Print date in YY/MM/DD
                __lcd_newline();
                printf("%02x:%02x:%02x", time[2],time[1],time[0]);    //HH:MM:SS
                
                flag_interrupt = delay_1s_interrupt(1);
                if(flag_interrupt == 1){
                    break;
                }
            }
            if(flag_interrupt == 1){
                break;
            }
            initLCD();
            printf("1: Start");
            flag_interrupt = delay_1s_interrupt(2);
            if(flag_interrupt == 1){
                break;
            }
            
            initLCD();
            printf("2: Logs");
            flag_interrupt = delay_1s_interrupt(2);
            if(flag_interrupt == 1){
                break;
            }

            initLCD();
            printf("3: PC Download");
            flag_interrupt = delay_1s_interrupt(2);
            if(flag_interrupt == 1){
                break;
            }
        }
        unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code
        
        while(PORTBbits.RB1 == 1){
            continue;
            // Wait until the key has been released
        }
        
        Nop();  //Apply breakpoint here because of compiler optimizations
        Nop();
        
        if (keys[keypress] == keys[0]){

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

            unsigned char count_min_initial = time[1];
            unsigned char count_seconds_initial = time[0];
            unsigned char check = 0;
            
            initLCD();
            printf("Operating...");
            
            while(check == 0){//wait until a key is pressed again
                check = key_pressed_check();
                
                
                
                
                
                //pin inputs from ADC
                
                continue;
            }
            check = 0;
            keypress = (PORTB & 0xF0)>>4;

            while(PORTBbits.RB1 == 1){
                continue; // Wait until the key has been released
            }
            
            Nop();  //Apply breakpoint here because of compiler optimizations
            Nop();

            if(keys[keypress] == keys[0]){ //Emergency Stop
               while(PORTBbits.RB1 == 0){
                    initLCD();
                    printf("EMERGENCY");
                    __lcd_newline();
                    printf("STOP");
                    flag_interrupt = delay_1s_interrupt(2);
                    if(flag_interrupt == 1){
                        break;
                    }
                    initLCD();
                    printf("Press Any Key");
                    __lcd_newline();
                    printf("To Reset");
                    flag_interrupt = delay_1s_interrupt(2);
                    if(flag_interrupt == 1){
                        break;
                    }
               }
               initLCD();
               printf("Terminating...");
               __delay_1s();
               __delay_1s();
               __delay_1s();
               return;
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

            unsigned char count_min_final = time[1];
            unsigned char count_seconds_final =  time[0];

            initLCD();
            printf("s:%02x min ", count_min_initial);
            printf("%02x sec", count_seconds_initial);

            __lcd_newline();
            printf("f:%02x min ", count_min_final);
            printf("%02x sec", count_seconds_final);

            __delay_1s();
            __delay_1s();
            __delay_1s();

            count_min_initial = (count_min_initial & 0x0F) + ((count_min_initial & 0xF0)>>4)*10;
            count_seconds_initial = (count_seconds_initial & 0x0F) + ((count_seconds_initial & 0xF0)>>4)*10;
            count_min_final = (count_min_final & 0x0F) + ((count_min_final & 0xF0)>>4)*10;
            count_seconds_final = (count_seconds_final & 0x0F) + ((count_seconds_final & 0xF0)>>4)*10;

            signed char count_min = count_min_final - count_min_initial;
            signed char count_seconds = count_seconds_final - count_seconds_initial;

            if(count_seconds < 0){
                count_min = count_min - 1;
                count_seconds = count_seconds + 60;
            }

            //0x00 0x50
            //0x10 0x60
            //0x20 0x70
            //0x30 0x80

            signed char eeprom_min = Eeprom_ReadByte(0x10);
            signed char eeprom_seconds = Eeprom_ReadByte(0x60);
            
            Eeprom_WriteByte(0x00, eeprom_min);
            Eeprom_WriteByte(0x50, eeprom_seconds);
            eeprom_min = Eeprom_ReadByte(0x20);
            eeprom_seconds = Eeprom_ReadByte(0x70);
            Eeprom_WriteByte(0x10, eeprom_min);
            Eeprom_WriteByte(0x60, eeprom_seconds);
            eeprom_min = Eeprom_ReadByte(0x30);
            eeprom_seconds = Eeprom_ReadByte(0x80);
            Eeprom_WriteByte(0x20, eeprom_min);
            Eeprom_WriteByte(0x70, eeprom_seconds);

            Eeprom_WriteByte(0x30, count_min);
            Eeprom_WriteByte(0x80, count_seconds);

            initLCD();
            printf("%02d", count_min);
            printf(" minutes");
            __lcd_newline();
            printf("%02d", count_seconds);
            printf(" seconds");
            __delay_1s();
            __delay_1s();
            __delay_1s();
            initLCD();
            printf("AA:");
            __delay_1s();
            __delay_1s();
            __delay_1s();
            initLCD();
            printf("9V:");
            __delay_1s();
            __delay_1s();
            __delay_1s();
            initLCD();
            printf("C:");
            __delay_1s();
            __delay_1s();
            __delay_1s();
            initLCD();
            printf("Total:");
            __delay_1s();
            __delay_1s();
            __delay_1s();
        }
        else if (keys[keypress] == keys[1]){
            unsigned char address_min = 0x30;
            unsigned char address_seconds = 0x80;
            signed char past_min = Eeprom_ReadByte(address_min);
            signed char past_seconds = Eeprom_ReadByte(address_seconds);

            for(unsigned char e=0;e<0x04;e++){

                initLCD();
                printf("%02d", past_min);
                printf(" minutes");
                __lcd_newline();
                printf("%02d", past_seconds);
                printf(" seconds");
                __delay_1s();
                __delay_1s();
                __delay_1s();

                address_min = address_min - 0x10;
                address_seconds = address_seconds - 0x10;
                past_min = Eeprom_ReadByte(address_min);
                past_seconds = Eeprom_ReadByte(address_seconds);
            }
        }
        else if (keys[keypress] == keys[2]){
            initLCD();
            printf("Downloading...");
            __delay_1s();
            __delay_1s();
            __delay_1s();
            initLCD();
            printf("Download ");
            __lcd_newline();
            printf("Complete");
            __delay_1s();
            __delay_1s();
            __delay_1s();
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