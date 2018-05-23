/*
===============================================================================
 Name        : dimba.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC43xx.h"
#endif

//#include <cr_section_macros.h>

#include "sapi.h"
#include "uartisr.h"


CONSOLE_PRINT_ENABLE

void
log(unsigned char c)
{
	uartWriteByte(UART_USB, c);
}

int main(void) {

	volatile static int i = 0 ;

    // Inicializar y configurar la plataforma
    boardConfig();

    gpioConfig( GPIO0, GPIO_OUTPUT );
    gpioWrite( GPIO0, 1 );
    delay( 500 );
    gpioWrite( GPIO0, 0 );
    delay( 2000 );
    gpioWrite( GPIO0, 1 );



    // Inicializar UART_USB como salida de consola
    consolePrintConfigUart( UART_USB, 19200 );

    consolePrintString( "hello" );

    uartConfig( UART_232, 19200 );
    uartIsr_rxEnable( UART_232, log);

    uartWriteString( UART_232, "AT\r\n" );

    // Force the counter to be placed into memory

    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
    return 0 ;
}
