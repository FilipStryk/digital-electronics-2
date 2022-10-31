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
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for number conversions
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
    lcd_gotoxy(1, 0); lcd_puts("value:");
    lcd_gotoxy(0, 1); lcd_puts("key:");
    // lcd_gotoxy(8, 0); lcd_puts("a");  // Put ADC value in decimal
    lcd_gotoxy(13,0); lcd_puts("b");  // Put ADC value in hexadecimal
    lcd_gotoxy(8, 1); lcd_puts("c");  // Put button name here

    // Configure Analog-to-Digital Convertion unit
    // Select ADC voltage reference to "AVcc with external capacitor at AREF pin"
    adc_internal_ref();

    // Select input channel ADC0 (voltage divider pin)
    // ADC0 - default values

    // Enable ADC module
    ADCSRA |= (1 << ADEN);

    // Enable conversion complete interrupt
    ADCSRA |= (1 << ADIE);

    // Set clock prescaler to 128
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Configure 16-bit Timer/Counter1 to start ADC conversion
    // Set prescaler to 33 ms and enable overflow interrupt
    TIM1_overflow_33ms();
    TIM1_overflow_interrupt_enable();


    // Enables interrupts by setting the global interrupt mask
    sei();

    // Infinite loop
    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Use single conversion mode and start conversion every 100 ms.
 **********************************************************************/
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

/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Display converted value on LCD screen.
 **********************************************************************/
ISR(ADC_vect)
{
    uint16_t value;
    char string[4];  // String for converted numbers by itoa()

    // Read converted value
    // Note that, register pair ADCH and ADCL can be read as a 16-bit value ADC
    value = ADC;

    // Value in decimal
    itoa(value, string, 10);
    lcd_gotoxy(8, 0);
    lcd_puts("     "); // Clear previous value
    lcd_gotoxy(8, 0);
    lcd_puts(string);

    // Value in hexadecimal
    itoa(value, string, 16);
    lcd_gotoxy(13, 0);
    lcd_puts("   "); // Clear previous value
    lcd_gotoxy(13, 0);
    lcd_puts(string);

    // Voltage
    int voltage = 5*value;
    itoa(voltage, string, 10);
    lcd_gotoxy(12, 1);
    lcd_puts("     "); // Clear previous value
    lcd_gotoxy(12, 1);
    lcd_puts(string);

    // Button name
    lcd_gotoxy(5, 1);
    lcd_puts("      "); // Clear previous value
    lcd_gotoxy(5, 1);
    if (value < 10) // Right
    {
        lcd_puts("Right");
    } 
    else if (value > 138 && value < 148) // Up
    {
        lcd_puts("Up");
    }
    else if (value > 337 && value < 347) // Down
    {
        lcd_puts("Down");
    }
    else if (value > 506 && value < 516) // Left
    {
        lcd_puts("Left");
    }
    else if (value > 726 && value < 736) // Select
    {
        lcd_puts("Select");
    }
    else
    {
        lcd_puts("None");
    }
}