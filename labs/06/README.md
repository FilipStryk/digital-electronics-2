# Lab 6: Filip Stryk

### ASCII

1. Complete the table with selected ASCII codes.

   | **Char** | **Decimal** | **Hexadecimal** | **Binary** |
   | :-: | :-: | :-: | :-: |
   | `a` | 97 | 0x61 | `0b0110_0001` |
   | `b` | 98 | 0x62 | `0b0110_0010` |
   | `c` | 99 | 0x63 | `0b0110_0011` |
   | `0` | 48 | 0x30 | `0b0011_0000` |
   | `1` | 49 | 0x31 | `0b0011_0001` |
   | `2` | 50 | 0x32 | `0b0011_0010` |
   | `Esc` | 27 | 0x1B | `0b0001_1011` |
   | `Space` | 32 | 0x20 | `0b0010_0000` |
   | `Tab` | 9 | 0x09 | `0b0000_1001` |
   | `Backspace` | 8 | 0x08 | `0b0000_1000` |
   | `Enter` | 10 +  13 | 0x0A + 0x0D | `0b0000_1010` + `0b0000_1101` |

### UART communication

2. Draw timing diagram of the output from UART/USART when transmitting three character data `De2` in 4800 7O2 mode (7 data bits, odd parity, 2 stop bits, 4800&nbsp;Bd). The image can be drawn on a computer or by hand. Name all parts timing diagram.

   ![your figure](timing_diagram.png)

   Each tick is approx. 208 microseconds.

3. Draw a flowchart for function `uint8_t get_parity(uint8_t data, uint8_t type)` which calculates a parity bit of input 8-bit `data` according to parameter `type`. The image can be drawn on a computer or by hand. Use clear descriptions of individual algorithm steps.

   ![your figure](flowchart.png)

```c
/**
 * type = 0 => Even parity
 * type = 1 => Odd parity
 */
uint8_t get_parity(uint8_t data, uint8_t type)
{
   if (type != 0 && type != 1) 
   {
      return 0xFF;
   }

   uint8_t parity = (data & 1) ^ type;

   for (uint8_t i = 1; i < 8; i++) 
   {
      parity ^= (data >> i) & 1;
   }

   return parity;
}
```