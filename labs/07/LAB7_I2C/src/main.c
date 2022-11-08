/***********************************************************************
 * 
 * The I2C (TWI) bus scanner tests all addresses and detects chips and
 * modules that are connected to the SDA and SCL signals.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2017 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Defines -----------------------------------------------------------*/
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions

struct dht12_data {
    uint8_t hum_int;
    uint8_t hum_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t checksum;
} air;

struct ds3231_data {
    uint8_t secs;
    uint8_t mins;
    uint8_t hours;
} date;

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1, send one I2C (TWI) address every 33 ms,
 *           and test all combinations. Send information about scanning
 *           process to UART.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    // Initialize I2C (TWI)
    twi_init();

    // Initialize USART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));

    // Configure 16-bit Timer/Counter1 to test one I2C address
    // Set prescaler to 33 ms and enable interrupt
    TIM1_overflow_4s();
    TIM1_overflow_interrupt_enable();

    // Enables interrupts by setting the global interrupt mask
    sei();

    // Put strings to ringbuffer for transmitting via UART
    uart_puts("Scan I2C bus for devices:\r\n");

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
 * Purpose:  Test one I2C Slave address from the range 8 to 119.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    static uint8_t sla = 8;  // I2C Slave address
    uint8_t ack;             // ACK response from Slave
    char string[8];          // String for converting numbers by itoa()

    // if (sla < 120)
    // {
    //     // Start communication, transmit I2C Slave address, get result,
    //     // and Stop communication
    //     ack = twi_start(sla, TWI_WRITE);
    //     twi_stop();

    //     itoa(sla, string, 10);
    //     uart_puts("SLA: ");
    //     uart_puts(string);
    //     uart_puts(" (0x");
    //     itoa(sla, string, 16);
    //     uart_puts(string);
    //     uart_puts(")");
    //     uart_puts(" - ");

    //     if (ack == 0)
    //     {
    //         uart_puts("ACK\n");
    //     }
    //     else
    //     {
    //         uart_puts("NACK\n");
    //     }

    //     sla++;
    // }

    ack = twi_start(0x5C, TWI_WRITE);
    if (ack == 0)
    {
        twi_write(0x00);
        twi_stop();

        twi_start(0x5C, TWI_READ);
        air.hum_int = twi_read_ack();
        air.hum_dec = twi_read_ack();
        air.temp_int = twi_read_ack();
        air.temp_dec = twi_read_nack();
        twi_stop();

        uart_puts("DHT12:\n");
        uart_puts("Humidity: ");
        itoa(air.hum_int, string, 10);
        uart_puts(string);
        uart_puts(".");
        itoa(air.hum_dec, string, 10);
        uart_puts(string);
        uart_puts(" %\n");

        uart_puts("Temperature: ");
        itoa(air.temp_int, string, 10);
        uart_puts(string);
        uart_puts(".");
        itoa(air.temp_dec, string, 10);
        uart_puts(string);
        uart_puts(" °C\n");
    }

    ack = twi_start(0x68, TWI_WRITE);
    if (ack == 0)
    {
        twi_write(0x00);
        twi_stop();

        twi_start(0x68, TWI_READ);
        date.secs = twi_read_ack();
        date.mins = twi_read_ack();
        date.hours = twi_read_nack();
        twi_stop();

        uart_puts("DS3231:\n");
        uart_puts("Time: ");
        itoa((date.hours >> 4) & 0b00000111, string, 10);
        uart_puts(string);
        itoa(date.hours & 0b00001111, string, 10);
        uart_puts(string);
        uart_puts(":");
        itoa((date.mins >> 4) & 0b00000111, string, 10);
        uart_puts(string);
        itoa(date.mins & 0b00001111, string, 10);
        uart_puts(string);
        uart_puts(":");
        itoa((date.secs >> 4) & 0b00000111, string, 10);
        uart_puts(string);
        itoa(date.secs & 0b00001111, string, 10);
        uart_puts(string);
        uart_puts("\n");
    }
    
}