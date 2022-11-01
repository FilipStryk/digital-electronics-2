/***********************************************************************
 * 
 * Use USART unit and transmit data between ATmega328P and computer.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2018 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Defines -----------------------------------------------------------*/
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif


/* Includes ----------------------------------------------------------*/
#include <stdlib.h>         // C library. Needed for number conversions
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "uart.h"           // Peter Fleury's UART library


/*
 * parity_type = 0 => even parity
 * parity_type = 1 => odd parity
 */
uint8_t get_parity(uint8_t val, uint8_t parity_type)
{
    uint8_t parity = (val & 1) ^ ((val >> 1) & 1);
    // parity ^= (val >> 2) & 1;
    // parity ^= (val >> 3) & 1;
    // parity ^= (val >> 4) & 1;
    // parity ^= (val >> 5) & 1;
    // parity ^= (val >> 6) & 1;
    // parity ^= (val >> 7) & 1;

    for (uint8_t i = 2; i < 8; i++)
    {
        parity ^= (val >> i) & 1;
    }

    if (parity_type == 1)
    {
        parity ^= 1;
    }

    return parity;
}


/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and transmit UART data four times 
 *           per second.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    // Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    
    // Configure 16-bit Timer/Counter1 to transmit UART data
    // Set prescaler to 262 ms and enable overflow interrupt
    TIM1_overflow_262ms();
    TIM1_overflow_interrupt_enable();

    // Enables interrupts by setting the global interrupt mask
    sei();

    // Put strings to ringbuffer for transmitting via UART
    uart_puts("Print one line... ");
    uart_puts("done\r\n");

    uart_puts("Char\tDec\tHex\tBin\t\tEven\tOdd\r\n");

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
 * Purpose:  Transmit UART data four times per second.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    // uart_puts("Paris\r\n");
    uint8_t val;
    uint8_t parity;
    char str[8];

    val = uart_getc();
    if (val != '\0') {
        uart_putc(val);
        uart_putc('\t');

        itoa(val, str, 10);
        uart_puts(str);

        uart_putc('\t');

        itoa(val, str, 16);
        uart_puts(str);

        uart_putc('\t');

        itoa(val, str, 2);
        uart_puts(str);
        uart_putc('\t');
        uart_putc('\t');
        
        parity = get_parity(val, 0);
        itoa(parity, str, 10);
        uart_puts(str);

        uart_putc('\t');

        parity = get_parity(val, 1);
        itoa(parity, str, 10);
        uart_puts(str);

        uart_puts("\r\n");
    }
}