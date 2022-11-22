/***********************************************************************
 * 
 * Use Analog-to-digital conversion to read push buttons on LCD keypad
 * shield and display it on LCD screen.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2018 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for number conversions
#include "timer.h"
#include "adc.h"

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and start ADC conversion every 100 ms.
 *           When AD conversion ends, send converted value to LCD screen.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    // Initialize display
    lcd_init(LCD_DISP_ON);
    lcd_gotoxy(0, 0); lcd_puts("X:");
    lcd_gotoxy(0, 1); lcd_puts("Y:");
    // Infinite loop

    adc_internal_ref();

    ADCSRA |= (1 << ADEN);

    // Enable conversion complete interrupt
    ADCSRA |= (1 << ADIE);

    // Set clock prescaler to 128
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    TIM1_overflow_33ms();
    TIM1_overflow_interrupt_enable();


    /* set PD2 and PD3 as input */ 
    DDRD &=~ (1 << PD2);				/* PD2 and PD3 as input */ 
    DDRD &=~ (1 << PD3);        
    PORTD |= (1 << PD3)|(1 << PD2);   /* PD2 and PD3 pull-up enabled   */

    EIMSK |= (1<<INT0)|(1<<INT1);	// enable INT0 and INT1
    MCUCR |= (1<<ISC01)|(1<<ISC11)|(1<<ISC10); // INT0 - falling edge, INT1 - raising edge

    sei();

    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}

ISR(TIMER1_OVF_vect)
{
    static uint8_t no_of_overflows = 0;

    no_of_overflows++;
    if (no_of_overflows >= 3)
    {
        no_of_overflows = 0;
        ADCSRA |= (1 << ADSC);
    }
}


ISR(ADC_vect)
{
    uint16_t value;
    char string[4];  // String for converted numbers by itoa()

    // Read converted value
    // Note that, register pair ADCH and ADCL can be read as a 16-bit value ADC
    value = ADC;

    uint8_t channel = ADMUX & 0b00001111;
    switch (channel)
    {
        case 0:
            // Value in decimal
            itoa(value, string, 10);
            lcd_gotoxy(3, 0);
            lcd_puts("     "); // Clear previous value
            lcd_gotoxy(3, 0);
            lcd_puts(string);
            ADMUX |= 1;
            break;
        
        case 1:
            // Value in decimal
            itoa(value, string, 10);
            lcd_gotoxy(3, 1);
            lcd_puts("     "); // Clear previous value
            lcd_gotoxy(3, 1);
            lcd_puts(string);
            ADMUX &= 0b11111110;
            break;
    }
    // itoa(value, string, 10);
    //         lcd_gotoxy(3, 0);
    //         lcd_puts("     "); // Clear previous value
    //         lcd_gotoxy(3, 0);
    //         lcd_puts(string);
}


ISR(INT0_vect ) 
{ 
	if(!bit_is_clear(PIND, PD3)) 
	{ 
		lcd_gotoxy(10, 0); lcd_puts("+");
	} 
	else 
	{ 
		lcd_gotoxy(10, 0); lcd_puts("-");
	} 
} 

//INT1 interrupt 
ISR(INT1_vect) 
{ 
	if(!bit_is_clear(PIND, PD2)) 
	{ 
		lcd_gotoxy(10, 0); lcd_puts("+");
	} 
	else 
	{ 
		lcd_gotoxy(10, 0); lcd_puts("-");
	} 
}