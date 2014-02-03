#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include "spi.h"

void spi_init(void){
    /* Set MOSI and SCK output, all others input */
    DDRB = (1<<PB5)|(1<<PB7)|(1<<PB4);
    /* Enable SPI, Master, set clock rate fck/16 */
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPI2X);//|(1<<CPHA);//|(1<<DORD);
}


void spi_write(char cData){
    /* Start transmission */
    SPDR = cData;
    /* Wait for transmission complete */
    while(!(SPSR & (1<<SPIF)));
}