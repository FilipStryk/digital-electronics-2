#ifndef F_CPU
    #define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "lcd.h"
#include "timer.h"
#include "adc.h"
#include "uart.h"

#define ENCODER_CLK PB2
#define ENCODER_DT PB3

#define MAIN_MENU_N_OF_ITEMS 2
#define N_OF_RECORDS 10
#define RECORDS_PER_PAGE 2

#define BTN_DEBOUNCE_TIME_MS 40
#define ENCODER_DEBOUNCE_TIME_MS 10

uint64_t millis();

void main_menu();

void records();

void game_start();
void game_draw();
void game_scroll();
void game_stop();

// number of milliseconds since power-on
volatile uint64_t timer_millis = 0;

// variables to store previous values of rotary encoder signals
volatile bool clk_prev = true;
volatile bool dt_prev = true;

// variables to store previous buttons values
volatile bool btn_encoder_prev = true;
volatile bool btn_joystick_prev = true;

// variables to store the time when button values has changed
volatile uint64_t btn_encoder_prev_ms = 0;
volatile uint64_t btn_joystick_prev_ms = 0;

// variables to store the time when the rotary encoder values has changed
volatile uint64_t encoder_clk_prev_ms = 0;
volatile uint64_t encoder_dt_prev_ms = 0;

// coordinates of the players
volatile uint8_t x = 0;
volatile uint8_t y = 0;

// player's score
volatile uint32_t score = 0;

// index of the selected item in the menu
volatile uint8_t menu_selected_item = 0;

// index of the current page when viewing records
volatile uint8_t records_current_page = 0;

// array with record values
uint32_t list_of_records[N_OF_RECORDS] = {0};

// variable to store current program state
enum state_e {
    MAIN_MENU,
    GAME_RUNNING, 
    GAME_OVER,
    RECORDS,
} state = MAIN_MENU;

// initial playing field
uint8_t initial_playing_field[2][16] = {
    {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xFF, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xFF},
    {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xFF, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5}
};

uint8_t playing_field[2][16];

int main(void)
{
    // initialize the pseudo-random generator seed
    srand(5815);

    // initialize UART for debugging
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

    // Enable A/D converter with internal reference, prescaler value 128 and interrupt after the conversion is completed
    adc_internal_ref();
    adc_select_channel(0);
    adc_enable();
    adc_enable_interrupt();
    adc_prescaler_128();

    // Timer for counting milliseconds since device power up
    TIM0_overflow_1ms();
    TIM0_overflow_interrupt_enable();

    // Set rotary encoder pins as input with pull-up resistors and enable pin change interrupts for these pins
    DDRB &= ~((1 << ENCODER_CLK) | (1 << ENCODER_DT));
    PORTB |= (1 << ENCODER_CLK) | (1 << ENCODER_DT);
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

    // Ininite loop
    while (1)
    {
        // Call the corresponding function according to the current state
        switch (state)
        {
            case MAIN_MENU:
                // Show main menu
                main_menu();
                _delay_ms(50);
                break;
            case RECORDS:
                // Show records
                records();
                _delay_ms(50);
            case GAME_RUNNING:
                // no need to call a function, because the game is perodically drawn using timer and interrupts
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
                _delay_ms(50);
                break;
        }
    }

    return 0;
}

// Returns number of milliseconds since power-up
uint64_t millis()
{
    // disable interrupts
    cli(); // prevents the timer from changing the value while reading the variable
    uint64_t val = timer_millis;
    // enable interrupts
    sei();
    
    return val;
}

// Displays the main menu 
void main_menu()
{
    lcd_clrscr();
    char menu_items[MAIN_MENU_N_OF_ITEMS][15] = {"Start game", "Show records"};
    // Loop through menu items
    for (uint8_t i = 0; i < MAIN_MENU_N_OF_ITEMS; i++)
    {
        lcd_gotoxy(0, i);
        // Highlight the selected menu item with '>' character on the beginning of the line
        if (i == menu_selected_item)
        {
            lcd_putc('>');
        }
        lcd_puts(menu_items[i]);
    }
}

// Displays the list of records
void records()
{
    lcd_clrscr();
    char str[12];
    // Loop through the items on the current page
    for (uint8_t i = records_current_page * RECORDS_PER_PAGE; i < records_current_page * RECORDS_PER_PAGE + RECORDS_PER_PAGE; i++)
    {
        itoa(i + 1, str, 10);
        lcd_gotoxy(0, i - records_current_page * RECORDS_PER_PAGE);
        lcd_puts(str);
        lcd_puts(". ");
        itoa(list_of_records[i], str, 10);
        lcd_puts(str);
    }
}

// Starts the game
void game_start()
{
    // set previous score as pseudo-random generator seed to increase randomness
    srand(score);
    // set state and reset variables
    state = GAME_RUNNING;
    score = 0;
    x = 0;
    y = 0;

    // copy the initial playing field into the real playing filed
    memcpy(playing_field, initial_playing_field, sizeof(playing_field));

    // set timer to periodically start the A/D conversion sensing joystick position
    TIM1_overflow_262ms();
    TIM1_overflow_interrupt_enable();

    // set timer to periodically move playing field to the left
    TIM2_overflow_16ms();
    TIM2_overflow_interrupt_enable();
}

// Stops the game
void game_stop()
{
    // set state and disable timers
    state = GAME_OVER;
    TIM1_overflow_interrupt_disable();
    TIM2_overflow_interrupt_disable();
    
    // loop through all the records
    for (uint8_t i = 0; i < N_OF_RECORDS; i++)
    {
        // check whether the current score is higher than the one in the list of records
        if (score > list_of_records[i])
        {
            // insert the new score into position of the old one and push the remaining values to lower positions
            for (uint8_t j = N_OF_RECORDS - 1; j > i; j--)
            {
                list_of_records[j] = list_of_records[j - 1];
            }
            
            list_of_records[i] = score;
            // break the for-loop after inserting the new record
            break;
        }
    }
}

// Renders the game onto the LCD
void game_draw()
{
    // loop through the rows
    for (uint8_t i = 0; i < 2; i++)
    {
        lcd_gotoxy(0, i);
        // loop through the columns
        for (uint8_t j = 0; j < 16; j++)
        {
            // check whether the position of the currently drawn item equals to user's position
            if (i == y && j == x)
            {   
                // check whether there is an obstacle on the player's current position and stop the game if yes
                if (playing_field[i][j] == 0xFF)
                {
                    game_stop();
                    return;
                }
                else // or increase the score if not
                {
                    playing_field[i][j] = ' ';
                    score++;
                    // draw the custom character of the player
                    lcd_putc(0x00);
                }
            }
            else // draw the field's item character
            {
                lcd_putc(playing_field[i][j]);
            }
        }
    }
}

// Scrolls the playing field to the left by one position
void game_scroll()
{
    // loop thgrough the rows
    for (uint8_t i = 0; i < 2; i++)
    {
        // loop through the columns
        for (uint8_t j = 0; j < 15; j++)
        {
            // move the field item to the left
            playing_field[i][j] = playing_field[i][j+1];
        }
    }

    // generate random number
    uint8_t rnd = rand();
    // probability 20 % that there will be an obstacle in the newly generated column
    if (rnd % 5 == 0)
    {
        // insert an obstacle in the row according to whether the number is divisible by 2 or not 
        // (row indexes are 0 and 1, as well as the result of % 2)
        uint8_t x_coord = rnd % 2;
        // check if there isn't an obstacle in the diagonally adjacent item
        // (that would prevent the player from overcoming the obstacle)
        if (playing_field[(rnd + 1) % 2][14] != 0xFF)
        {
            // insert the new obstacle if there isn't the adjecent one
            playing_field[x_coord][15] = 0xFF;
        }
        else
        {
            // insert regular character otherwise
            playing_field[x_coord][15] = 0xA5;
        }

        // insert regular character into the remaining row
        playing_field[(rnd + 1) % 2][15] = 0xA5;
    }
    else
    {
        // insert regular characters without the obstacle if the number is not divisible by 5
        playing_field[0][15] = 0xA5;
        playing_field[1][15] = 0xA5;
    }
}

// Increments the number of milliseconds elapsed since the power-up
ISR(TIMER0_OVF_vect)
{
    timer_millis++;
}

// Starts the A/D conversion
ISR(TIMER1_OVF_vect)
{
    adc_start_conversion();
}

// Scrolls the playing field to the left and redraws it approx. every  500 ms (3x16 = 496 ms)
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

// Senses the joystick position
ISR(ADC_vect)
{

    uint16_t value;
    value = ADC;

    uint8_t channel = adc_get_current_channel();
    // Move the player according to sensed value from the current channel 
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

            // Switch ADC to the channel 1 and start the conversion again
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
            // Switch the ADC back to the channel 0
            adc_select_channel(0);
            break;
    }
}


ISR(PCINT0_vect)
{
    bool clk = PINB & (1 << ENCODER_CLK);
    bool dt = PINB & (1 << ENCODER_DT);

    if (clk != clk_prev && clk_prev == false && dt == false && (millis() - encoder_clk_prev_ms) >= ENCODER_DEBOUNCE_TIME_MS) // left rotation
    {
        if (state == MAIN_MENU && menu_selected_item > 0)
        {
            menu_selected_item--;
        }
        else if (state == RECORDS && records_current_page > 0)
        {
            records_current_page--;
        }
    }
    else if (dt != dt_prev && dt_prev == false && clk == false && (millis() - encoder_dt_prev_ms) >= ENCODER_DEBOUNCE_TIME_MS) // right rotation
    {
        if (state == MAIN_MENU && menu_selected_item < MAIN_MENU_N_OF_ITEMS - 1)
        {
            menu_selected_item++;
        }
        else if (state == RECORDS && records_current_page < N_OF_RECORDS / RECORDS_PER_PAGE - 1)
        {
            records_current_page++;
        }
    }

    if (clk != clk_prev && clk == true)
    {
        encoder_clk_prev_ms = millis();
    }
    else if (dt != dt_prev && dt == true)
    {
        encoder_dt_prev_ms = millis();
    }

    clk_prev = clk;
    dt_prev = dt;
}

// Handles the INT0 interrupt (encoder button)
ISR(INT0_vect)
{
    // read the current pin value (0 if pressed)
    bool btn_encoder = PIND & (1 << PD2);
    // check whether the button has been pushed and whether the passed time between the two interrupts was greater than BTN_DEBOUNCE_TIME_MS
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
        else if (state == GAME_OVER)
        {
            state = MAIN_MENU;
        }
    }

    btn_encoder_prev = btn_encoder;
    btn_encoder_prev_ms = millis();
}

// Handles the INT1 interrupt (joystick button)
ISR(INT1_vect)
{
    // read the current pin value (0 if pressed)
    bool btn_joystick = PIND & (1 << PD3);
    // check whether the button has been pushed and whether the passed time between the two interrupts was greater than BTN_DEBOUNCE_TIME_MS
    if (btn_joystick_prev != btn_joystick && btn_joystick == false && (millis() - btn_joystick_prev_ms) >= BTN_DEBOUNCE_TIME_MS)
    {
        if (state == GAME_OVER)
        {
            game_start();
        }
    }

    btn_joystick_prev = btn_joystick;
    btn_joystick_prev_ms = millis();
}