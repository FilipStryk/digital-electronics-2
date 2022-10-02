# Lab 1: Filip Stryk

### Morse code

1. Listing of C code which repeats one "dot" and one "comma" (BTW, in Morse code it is letter `A`) on a LED. Always use syntax highlighting, meaningful comments, and follow C guidelines:

```c
int main(void)
{
    // Set pin where on-board LED is connected as output
    pinMode(LED_GREEN, OUTPUT);

    // Infinite loop
    while (1)
    {

        digitalWrite(LED_GREEN, HIGH);
        _delay_ms(500); // Length of dot - 500 ms
        digitalWrite(LED_GREEN, LOW);
        _delay_ms(500); // Space between symbols - 500 ms
        digitalWrite(LED_GREEN, HIGH);
        _delay_ms(1500); // Length od dash - 1500 ms
        digitalWrite(LED_GREEN, LOW);
        _delay_ms(1500); // Space between letters - 1500 ms
    }

    // Will never reach this
    return 0;
}
```

2. Scheme of Morse code application, i.e. connection of AVR device, LED, resistor, and supply voltage. The image can be drawn on a computer or by hand. Always name all components and their values!

   ![Schematic](schematic.png)