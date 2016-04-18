/* Main.c file generated by New Project wizard
 * Author : Ajit Jadhav
 * Created:   Thu Mar 15 2016
 * Processor: PIC18F46K22
 * Compiler:  MPLAB C18
 */

/* 
 |--------------------------------------|
 | Peripheral -	Function    -	PIC Pin |
 |--------------------------------------|
 | Ultrasonic -	Trig        -	RA4	|
 |	      - ECHO        -   RB0	|
 |--------------------------------------|
 | PIR 	      - OUT     -	AN1	|
 |--------------------------------------|
 | LDR        - OUT     -	AN0     |
 |--------------------------------------|
 | LED*	      - INPUT       - DACOUT	|
 |--------------------------------------|
 | LCD 	      - RS 	    -	RE0	|
 |	      - RW    	    -   RE1	|
 |	      - E 	    -   RE2 	|
 | 	      - D4	    -	RD0	| 	
 |	      - D5	    -	RD1	|
 |	      - D6	    -	RD2	|
 |	      - D7 	    -	RD3	|
 |--------------------------------------|
 | Indicator  - INPUT 	    -   RB5     |
 | LED 					|
 |--------------------------------------|

	*LED (Power LED) is interfaced to PIC using mosfet. 
		DACOUT pin is connected to mosfet gate.  
*/

#include <p18f46k22.h>
#include <delays.h>
#include <stdio.h>
#include "config.h"
#include "lcd.h"

// Ultrasonic Module pin definations
#define US_TRIG_DIR TRISAbits.TRISA4
#define US_TRIG LATAbits.LATA4
#define US_ECHO_DIR TRISBbits.TRISB0
#define US_ECHO PORTBbits.RB0

// LED Pin
#define LED_DIR TRISBbits.TRISB5 
#define LED LATBbits.LATB5

// LCD Line Buffers
char line1[16];
char line2[16];

unsigned int t;

// Results
unsigned int regL,regH,resADC,resPIR,resUS,resLDR,tmr_2 = 0;

// Timer
int sec=0,mn=0,hr=0;

// Flags
short f1,f2,f3;

void lcdinit();
void lcdstring(unsigned char *);

// Timer 2 Interrupt
#pragma interrupt chk_isr
void chk_isr(void)
{
    if(PIR1bits.TMR2IF == 1)
    {
        tmr_2++;
        PIR1bits.TMR2IF = 0;
        if(tmr_2 >= 250)
        {
            sec++;
            tmr_2 = 0;
        }    
    }
}

#pragma code My_Prio_Int=0x0008
void My_Prio_Int(void)
{
    _asm
    goto chk_isr
    _endasm
}
#pragma code

// Millisecond Delay for 16MHz Crystal
void msDelay(unsigned int i)
{
    for(; i>0; i--)
        Delay1KTCYx(16);
}

// Microsecond Delay
void usDelay(unsigned int i)
{
    for(; i>0; i--)
    {
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
    }
}

// ADC Initialization with Channel
void initADC(unsigned char ch)
{
  ADCON0 = 0x00;
  if(ch < 28)
    ADCON0 |=  (ch << 2);
  ADCON2 = 0xAD;
  ADCON0bits.ADON = 1;
}

// ADC Conversion 
void readADC(void)
{
  msDelay(1);  
  ADCON0bits.GO_DONE = 1;
  msDelay(1);
  while(ADCON0bits.GO_DONE);
  regL = ADRESL;
  regH = ADRESH;
  resADC = ((int) (regH<<8)|regL);
}

// Initializing Ultrasonic with Timer 0 16bit mode and 
// Weak-Pullup resistor enabled on RB0
void initUltrasonic(void)
{
    T0CON = 0x01;
    US_ECHO_DIR = 1;
    US_TRIG_DIR = 0;
    US_ECHO = 1;
    WPUBbits.WPUB0 = 1;
}

// Reading distance from US module in cm. using timer0
void readUltrasonic(void)
{
    TMR0H = 0;
    TMR0L = 0;
    US_TRIG = 1;
    Delay10TCYx(16);    // 10 uSec Delay for Trig pulse
    US_TRIG = 0;
    while(!US_ECHO);
    T0CONbits.TMR0ON = 1;
    while(US_ECHO);
    T0CONbits.TMR0ON = 0;
    regL = TMR0L;
    regH = TMR0H;
    resUS = (regH << 8) | regL;
    resUS = (int) (((resUS*0.00000025)*34000)/2);
}

// Reading PIR status using ADC on channel AN1
void readPIR(void)
{
    initADC(1);
    readADC();
    resPIR = resADC; 
    if(resPIR > 500)
        resPIR = 1;
    else
        resPIR = 0;
}

// LED indicator 
void eventL(void)
{
    LED = 1;
    Delay10KTCYx(10);
    LED = 0;
    Delay10KTCYx(10);
    LED = 1;
    Delay10KTCYx(10);
    LED = 0;
    Delay10KTCYx(10);
}

// LED indication for PIR true 
void pirIndicate(void)
{
    LED = 1;
    Delay10KTCYx(20);
    LED = 0;
    Delay10KTCYx(20);
}

// Reading LDR on ADC channel AN0
void readLDR(void)
{
    initADC(0);
    readADC();
    resLDR = (int) resADC/100;
}

// Setting Brightness of LED on DACOUT pin using 5bit DAC
void setBrightness(void)
{
    readLDR();
    if(resLDR <= 9 && resLDR > 8)
        VREFCON2 = 23;
    if(resLDR <= 8 && resLDR > 7)
        VREFCON2 = 23;
    if(resLDR <= 7 && resLDR > 6)
        VREFCON2 = 21;
    if(resLDR <= 6 && resLDR > 5)
        VREFCON2 = 20;
    if(resLDR <= 5 && resLDR > 4)
        VREFCON2 = 19;
    if(resLDR <= 4 && resLDR > 3)
        VREFCON2 = 18;
    if(resLDR <= 3 && resLDR > 2)
        VREFCON2 = 17;
}

// Initial brightness control testing
void initBrightness()
{
    for(t=17;t<24;t++)
    {
        VREFCON2 = t;
        msDelay(1200);
    }
    for(t=24;t>16;t--)
    {
        VREFCON2 = t;
        msDelay(1200);
    }
}

// Main routine
void main(void) {
    OSCCON = 0x7C;          // Internal Oscillator 16MHz Enabled
    OSCTUNE = 0x5F;	    
    OSCCON2bits.PLLRDY = 1;
    VREFCON1 = 0xE0;		
    T2CON = 0x7B;		
    INTCONbits.GIE = 1;     // Global, peripheral & timer 2 interrupt enable 
    INTCONbits.PEIE = 1;
    PIE1bits.TMR2IE = 1;
    PR2 = 0xff;				// period registor 2 with max value
    VREFCON2 = 0;			// zero brightness
    LED_DIR = 0;			// LED pin on output 
    LED = 1;
    lcdinit();				// Initialize LCD in 4bit mode, ultrasonic and brightness
    lcdcmd(0x01);
    initUltrasonic();
    initBrightness();
    LED = 0;
    while(1)
    {
        readPIR();
        if(resPIR)
        {
            pirIndicate();
            readUltrasonic();
            f1 = 0;
            f2 = 1;
            while(resUS < 100 && resUS > 10)
            {
                T2CONbits.TMR2ON = 1;
                setBrightness();
                if(f2)
                {
                    lcdcmd(0x80);
                    sprintf(line1,"    Welcome !   ");
                    lcdstring(line1);
                    sprintf(line2,"   Chaitanya.   ");
                    lcdcmd(0xc0);
                    lcdstring(line2);
                    msDelay(1000);
                    f2 = 0;
                }
                if(sec >= 60)
                {
                    mn++;
                    sec = 0;
                }
                if(mn >= 60)
                {
                    hr++;
                    mn = 0;
                }
                sprintf(line1,"   Study Time   ");
                sprintf(line2,"    %i:%i:%i    ",hr,mn,sec);
                lcdcmd(0x80);
                lcdstring(line1);
                lcdcmd(0xc0);
                lcdstring(line2);
                eventL();
                eventL();
                readUltrasonic();
                f1 = 1;
            }
            T2CONbits.TMR2ON = 0;
            if(f1)
            {
                sprintf(line1,"   Thank You !  ");
                sprintf(line2,"   Good Bye...! ");
                lcdcmd(0x80);
                lcdstring(line1);
                lcdcmd(0xc0);
                lcdstring(line2);
                msDelay(3000);
                VREFCON2 = 0;
            }
            lcdcmd(0x01);
        }
    }
}