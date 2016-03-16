/* Main.c file generated by New Project wizard
 * Author : Ajit Jadhav
 * Created:   Thu Mar 3 2016
 * Processor: PIC18F46K22
 * Compiler:  MPLAB C18
 */

#include <p18f46k22.h>
#include <delays.h>
#include <stdio.h>
#include "config.h"
#include "lcd.h"
#include "adc.h"

// Green LED pin 
#define LED_DIR TRISBbits.TRISB5 
#define LED LATBbits.LATB5

// Ultrasonic Module pin definations
#define US_TRIG_DIR TRISAbits.TRISA4
#define US_TRIG LATAbits.LATA4
#define US_ECHO_DIR TRISBbits.TRISB0
#define US_ECHO PORTBbits.RB0

// PIR Module Pin
#define PIR ANSELAbits.ANSA1

// LDR Pin
#define LDR ANSELAbits.ANSA0

char line1[16];
char line2[16];
unsigned int t;

void lcdinit();
void lcdstring(unsigned char *);

void msDelay(unsigned int i)
{
    for(; i>0; i--)
        Delay1KTCYx(4);
}

void usDelay(unsigned int i)
{
    for(; i>0; i--)
    {
        Nop();
        Nop();
        Nop();
        Nop();
    }
}

void main(void) {
    unsigned int regL, regH, resTMR, resLDR, resPIR;
    short pir_f,i,f;
    OSCCON = 0x76;
    VREFCON1 = 0xE0;
    T0CON = 0x01;
    LED_DIR = 0;
    US_ECHO_DIR = 1;
    US_TRIG_DIR = 0;
    US_ECHO = 1;
    WPUBbits.WPUB0 = 1;
    msDelay(1000);
    lcdinit();
    initADC(0);
    lcdcmd(0x01);
    lcdcmd(0x80);
    i = 0;
    f=0;
    while(1)
    {
        TMR0H = 0;
        TMR0L = 0;
        lcdcmd(0x80);
        if(i >= 0x1C)
            f = 1;
        if(i <= 17)
            f = 0; 
        VREFCON2 = i;
        sprintf(line1,"BRIGHT. LVL - %d ",i);
        lcdstring(line1);
        US_TRIG = 1;
        Delay10TCYx(4);
        US_TRIG = 0;
        while(!US_ECHO);
        T0CONbits.TMR0ON = 1;
        while(US_ECHO);
        T0CONbits.TMR0ON = 0;
        regL = TMR0L;
        regH = TMR0H;
        resTMR = (regH << 8) | regL;
        resLDR = readADC();
        initADC(1);
        resPIR = readADC();
        lcdcmd(0xC0);
        sprintf(line2,"T-%i,P-%i,L-%i",resTMR,resPIR,resLDR);
        lcdstring(line2);
        if(f)
            i--;
        else
            i++;
    }
    
}