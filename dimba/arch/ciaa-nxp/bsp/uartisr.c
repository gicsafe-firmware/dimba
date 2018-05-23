/**
 *  \file       uart_isr.c
 *  \brief      ...
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.09  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Bali�a db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "uartisr.h"
#include "sapi_uart.h"

/* ----------------------------- Local macros ------------------------------ */
#define UART_485_LPC LPC_USART0  /* UART0 (RS485/Profibus) */
#define UART_USB_LPC LPC_USART2  /* UART2 (USB-UART) */
#define UART_232_LPC LPC_USART3  /* UART3 (RS232) */

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static uartIsrCb uart_usb = NULL;
static uartIsrCb uart_232 = NULL;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
uartIsr_rxEnable( uartMap_t uart, uartIsrCb cb )
{
   switch(uart)
   {
       case UART_USB:
            //Enable UART Rx Interrupt
            //Receiver Buffer Register Interrupt
            Chip_UART_IntEnable(UART_USB_LPC,UART_IER_RBRINT );   
            // Enable UART line status interrupt
            //LPC43xx User manual page 1118
            Chip_UART_IntEnable(UART_USB_LPC,UART_IER_RLSINT ); 
            NVIC_SetPriority(USART2_IRQn, 6);
            // Enable Interrupt for UART channel
            NVIC_EnableIRQ(USART2_IRQn);
            uart_usb = cb;
            break;

        case UART_232:
            //Enable UART Rx Interrupt
            //Receiver Buffer Register Interrupt
            Chip_UART_IntEnable(UART_232_LPC,UART_IER_RBRINT );   
            // Enable UART line status interrupt
            //LPC43xx User manual page 1118
            Chip_UART_IntEnable(UART_232_LPC,UART_IER_RLSINT ); 
            NVIC_SetPriority(USART3_IRQn, 6);
            // Enable Interrupt for UART channel
            NVIC_EnableIRQ(USART3_IRQn);
            uart_232 = cb;
            break;
   case UART_485:
   default:
            break;
   }
}

__attribute__ ((section(".after_vectors")))

/* 0x2a 0x000000A8 - Handler for ISR UART2 (IRQ 26) */
void UART2_IRQHandler(void)
{
    char receivedByte;

    if (Chip_UART_ReadLineStatus(UART_USB_LPC) & UART_LSR_RDR)
    {
        receivedByte = Chip_UART_ReadByte(UART_USB_LPC);
        if(uart_usb != NULL)
        	(uart_usb)(receivedByte);
    }
}

/* 0x2b 0x000000AC - Handler for ISR UART3 (IRQ 27) */
void UART3_IRQHandler(void)
{
    char receivedByte;

    if (Chip_UART_ReadLineStatus(UART_232_LPC) & UART_LSR_RDR)
    {
        receivedByte = Chip_UART_ReadByte(UART_232_LPC);
        if(uart_232 != NULL)
        	(uart_232)(receivedByte);
    }
}

/* ------------------------------ End of file ------------------------------ */
