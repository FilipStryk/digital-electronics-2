#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

uint8_t get_parity(uint8_t data, uint8_t type);

int main()
{
    uint8_t number;
    scanf("%d", &number);

    uint8_t odd_par = get_parity(number, 1);
    uint8_t even_par = get_parity(number, 0);

    printf("%d = 0b"BYTE_TO_BINARY_PATTERN" (Odd parity: %d, Even parity: %d)\n", number, BYTE_TO_BINARY(number), odd_par, even_par);

    return EXIT_SUCCESS;
}


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
