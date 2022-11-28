#ifndef F_CPU
    #define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include "lcd.h"
#include "timer.h"
#include "adc.h"
#include "uart.h"

#define ENCODER_CLK PB2
#define ENCODER_DT PB3
#define MAIN_MENU_N_OF_ITEMS 2

#define BTN_DEBOUNCE_TIME_MS 40

uint64_t millis();

void main_menu();

void records();

void game_start();
void game_draw();
void game_scroll();
void game_stop();

volatile uint64_t timer_millis = 0;

volatile bool clk_prev = true;
volatile bool dt_prev = true;
volatile uint8_t val = 0;

volatile bool btn_encoder_prev = true;
volatile uint64_t btn_encoder_prev_ms = 0;
volatile bool btn_joystick_prev = true;
volatile uint64_t btn_joystick_prev_ms = 0;

volatile uint8_t x;
volatile uint8_t y;

volatile uint32_t score = 0;

volatile uint8_t menu_selected_item = 0;

uint32_t EEMEM records_list[10] = {0};

enum state_e {
    MAIN_MENU,
    GAME_RUNNING, 
    GAME_OVER,
    RECORDS,
} state = MAIN_MENU;

uint8_t playing_field[2][16] = {
    {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5},
    {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5}
};

int main(void)
{
    // initialize the pseude-random generator seed
    srand(2318);

    // initialoze UART for debugging
    uart_init(UART_BAUD_SELECT(9600, F_CPU));

    // initialize LCD
    lcd_init(LCD_DISP_ON);

    // save custom character into CGRAM
    uint8_t custom_char[8] = {
        0b00000,
        0b00110,
        0b01111,
        0b11100,
        0b11100,
        0b01111,
        0b00110,
        0b00000
    };
    lcd_command(1 << LCD_CGRAM);
    for (uint8_t i = 0; i < sizeof(custom_char)/sizeof(uint8_t); i++)
    {
        lcd_data(custom_char[i]);
    }
    lcd_command(1 << LCD_DDRAM);

    // Enable A/D converter with internal reference, prescaler value 128 and interrupt after the conversion if complete
    adc_internal_ref();
    adc_select_channel(0);
    adc_enable();
    adc_enable_interrupt();
    adc_prescaler_128();

    // Timer for counting milliseconds since device power up
    TIM0_overflow_1ms();
    TIM0_overflow_interrupt_enable();

    // Set rotary encoder pins as input with pull-up resistors and enable pin change interrupts for these pins
    DDRB &= ~((1 << PB2) | (1 << PB3));
    PORTB |= (1 << PB2) | (1 << PB3);
    PCMSK0 |= (1 << PCINT2) | (1 << PCINT3);
    PCICR |= (1 << PCIE0);
    
    // Set encoder and joystick button pins (INT0/1) as inputs with pull-up resistor
    DDRD &= ~((1 << PD2) | (1 << PD3));
    PORTD |= (1 << PD2) | (1 << PD3);
    // Set activation of both external interrupts to any logical change
    EICRA |= (1 << ISC00) | (1 << ISC10);
    // Enable external interrupts INT0 and INT1 
    EIMSK |= (1 << INT0) | (1 << INT1);

    // enable interrupts
    sei();

    // Set initial playing field
    playing_field[0][5] = 0xFF;
    playing_field[1][10] = 0xFF;
    playing_field[0][15] = 0xFF;

    while (1)
    {
        switch (state)
        {
            case MAIN_MENU:
                main_menu();
                _delay_ms(50);
                break;
            case RECORDS:
                records();
                _delay_ms(50);
            case GAME_RUNNING:
                break;
            case GAME_OVER:
                lcd_clrscr();
                lcd_gotoxy(3, 0);
                lcd_puts("GAME OVER!");
                lcd_gotoxy(0, 1);
                char str[8];
                itoa(score, str, 10);
                lcd_puts("Score: ");
                lcd_puts(str);
                _delay_ms(100);
                break;
        }
    }

    return 0;
}

// Returns number of milliseconds since power up
uint64_t millis()
{
    // disable interrupts
    cli(); // prevents the timer from changing the value while reading the variable
    uint64_t val = timer_millis;
    // enable interrupts
    sei();
    
    return val;
}

void main_menu()
{
    lcd_clrscr();
    char menu_items[MAIN_MENU_N_OF_ITEMS][15] = {"Start game", "Show records"};
    for (uint8_t i = 0; i < MAIN_MENU_N_OF_ITEMS; i++)
    {
        lcd_gotoxy(0, i);
        if (i == menu_selected_item)
        {
            lcd_putc('>');
        }
        lcd_puts(menu_items[i]);
    }
}

void records()
{
    lcd_clrscr();
    lcd_puts("Records");
}

void game_start()
{
    score = 0;

    // Timer to periodically start of A/D conversion sensing joystick position
    TIM1_overflow_262ms();
    TIM1_overflow_interrupt_enable();

    // Timer to periodically move playing field to the left
    TIM2_overflow_16ms();
    TIM2_overflow_interrupt_enable();
}

void game_stop()
{
    state = GAME_OVER;
    TIM1_overflow_interrupt_disable();
    TIM2_overflow_interrupt_disable();
}

void game_draw()
{
    for (uint8_t i = 0; i < 2; i++)
    {
        lcd_gotoxy(0, i);
        for (uint8_t j = 0; j < 16; j++)
        {
            if (i == y && j == x)
            {
                if (playing_field[i][j] == 0xFF)
                {
                    game_stop();
                    return;
                }
                else
                {
                    playing_field[i][j] = ' ';
                    score++;
                    lcd_putc(0x00);
                    char str[8];
                    itoa(score, str, 10);
                    uart_puts(str); uart_putc('\n');
                }
            }
            else
            {
                lcd_putc(playing_field[i][j]);
            }
        }
    }
}

void game_scroll()
{
    for (uint8_t i = 0; i < 2; i++)
    {
        for (uint8_t j = 0; j < 15; j++)
        {
            playing_field[i][j] = playing_field[i][j+1];
        }
    }

    uint8_t rnd = rand();
    if (rnd % 5 == 0)
    {
        uint8_t x_coord = rnd % 2;
        if (playing_field[(rnd + 1) % 2][14] != 0xFF)
        {
            playing_field[x_coord][15] = 0xFF;
        }
        else
        {
            playing_field[x_coord][15] = 0xA5;
        }
        playing_field[(rnd + 1) % 2][15] = 0xA5;
    }
    else
    {
        playing_field[0][15] = 0xA5;
        playing_field[1][15] = 0xA5;
    }
}

// increments the number of milliseconds since power up
ISR(TIMER0_OVF_vect)
{
    timer_millis++;
}

// starts the A/D conversion
ISR(TIMER1_OVF_vect)
{
    adc_start_conversion();
}

// scrolls the playing field to the left approx. every  500 ms
ISR(TIMER2_OVF_vect)
{
    static uint8_t n_of_overflows = 0;
    
    n_of_overflows++;
    if (n_of_overflows >= 31)
    {
        n_of_overflows = 0;
        game_scroll();
        game_draw();
    }
}


ISR(ADC_vect)
{

    uint16_t value;
    value = ADC;

    uint8_t channel = ADMUX & 0xF;
    switch (channel)
    {
        case 0:
            if (value > 750 && x < 15) 
            {
                x++;
                game_draw();
            }
            else if (value < 250 && x > 0)
            {
                x--;
                game_draw();
            }

            adc_select_channel(1);
            adc_start_conversion();
            break;
        
        case 1:
            if (value < 250 && y == 0) 
            {
                y++;
                game_draw();
            }
            else if (value > 750 && y == 1)
            {
                y--;
                game_draw();
            }
            adc_select_channel(0);
            break;
    }
}


ISR(PCINT0_vect)
{
    bool clk = PINB & (1 << ENCODER_CLK);
    bool dt = PINB & (1 << ENCODER_DT);

    if (clk != clk_prev && clk_prev == false) // left rotation
    {
        if (state == MAIN_MENU && menu_selected_item > 0)
        {
            menu_selected_item--;
        }
    }
    else if (dt != dt_prev && dt_prev == false) // right rotation
    {
        if (state == MAIN_MENU && menu_selected_item < MAIN_MENU_N_OF_ITEMS - 1)
        {
            menu_selected_item++;
        }
    }

    clk_prev = clk;
    dt_prev = dt;
}


ISR(INT0_vect)
{
    bool btn_encoder = PIND & (1 << PD2);
    if (btn_encoder_prev != btn_encoder && btn_encoder == false && (millis() - btn_encoder_prev_ms) >= BTN_DEBOUNCE_TIME_MS)
    {
        if (state == MAIN_MENU)
        {
            if (menu_selected_item == 0)
            {  
                state = GAME_RUNNING;
                game_start();
            }
            else if (menu_selected_item == 1)
            {
                state = RECORDS;
            }
        }
        else if (state == RECORDS)
        {
            state = MAIN_MENU;
        }
    }

    btn_encoder_prev = btn_encoder;
    btn_encoder_prev_ms = millis();
}


ISR(INT1_vect)
{
    bool btn_joystick = PIND & (1 << PD3);
    if (btn_joystick_prev != btn_joystick && btn_joystick == false && (millis() - btn_joystick_prev_ms) >= BTN_DEBOUNCE_TIME_MS)
    {
        if (state == GAME_OVER)
        {
            state = MAIN_MENU;
        }
    }

    btn_joystick_prev = btn_joystick;
    btn_joystick_prev_ms = millis();
}