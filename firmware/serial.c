/* -*- c++ -*- ***************************************************************
 *
 *  System      : Serial GLCD
 *  Module      : Serial Handling
 *  Object Name : $RCSfile: serial.c,v $
 *  Revision    : $Revision: 1.13 $
 *  Date        : $Date: 2015/07/05 21:06:13 $
 *  Author      : $Author: jon $
 *  Created By  : Jon Green
 *  Created     : Sun Apr 5 08:43:33 2015 Last Modified : <150704.0822>
 *
 *  Description : Handles all of the serial input
 *
 *  Notes       : Derrived from the code by Jennifer Holt and adapted for the
 *               160x128 screen.
 *
 *  History     :
 *
 *****************************************************************************
 *
 *  Copyright (c) 2010 Jennifer Holt
 *  Copyright (c) 2015 Jon Green
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/wdt.h>                    /* Watchdog timer */
#include <avr/io.h>

#include "glcd.h"

typedef uint8_t serial_t;

// The current write position; we write to the head.
static volatile serial_t rx_head;

// The current read position, we read from the tail
static volatile serial_t rx_tail;

// Keeps track of the # of bytes in the RX buffer
static volatile serial_t rx_count;

// Flags if RX has been suspended
static volatile uint8_t rx_pause;

// The actual buffer itself
static uint8_t rx_buffer[RX_BUFFER_SIZE];

//////////////////////////////////////////////////////////////////////////////
/// Initialise the serial port.
/// Set up the hardware. This may be invoked
/// multiple times in order to change the serial baud rate.
///
void
serial_init (void)
{
    // Set up the ring buffer.
    rx_count = 0;
    rx_head = 0;
    rx_tail = 0;
    rx_pause = 0;

    // Configure the serial port.
    serial_baudrate (BAUD_RATE_DEFAULT);
}

//////////////////////////////////////////////////////////////////////////////
/// Reconfigure the serial port.
/// Set up the hardware. This may be invoked
/// multiple times in order to change the serial baud rate.
///
/// @param [in] baud The baud rate of the port. A value of zero sets the
///                  default baud rate.
///
///                  The valid baud rates are defined as follows:
///
/// @                baud_rate_4800   = 1
///                  baud_rate_9600   = 2
///                  baud_rate_19200  = 3
///                  baud_rate_38400  = 4
///                  baud_rate_57600  = 5
///                  baud_rate_115200 = 6 [Default]
///
/// @return The value of baud that the system is using.
///
uint8_t
serial_baudrate (uint8_t baud)
{
    uint16_t rate;

    // Allow ASCII characters; convert from ASCII to interger. Binary
    // characters are processed un-modified.
    if (baud >= '1')
        baud -= '0';

    // Ensure that the baud rate is valid, otherwise use the default.
    if (baud_rate_invalid (baud))
        baud = BAUD_RATE_DEFAULT;

    // Get the appropriate setting for the hardware.
    if (baud == BAUD_RATE_4800)
        rate = (uint16_t)(1000000L / 2400L - 1);
    else if (baud == BAUD_RATE_9600)
        rate = (uint16_t)(1000000L / 4800L - 1);
    else if (baud == BAUD_RATE_19200)
        rate = (uint16_t)(1000000L / 9600L - 1);
    else if (baud == BAUD_RATE_38400)
        rate = (uint16_t)(1000000L / 19200L - 1);
    else if (baud == BAUD_RATE_57600)
        rate = (uint16_t)(1000000L / 28800L - 1);
    else // Default to 115200 if nothing is valid.
    {
        rate = (uint16_t)(1000000L / 57600L - 1);
        baud = BAUD_RATE_115200;
    }

    cli();
    // Set baud rate
    UBRR0H = (uint8_t) (rate >> 8);
    UBRR0L = (uint8_t) (rate & 0xff);

    // Enable receiver and transmitter
    UCSR0A = (1 << U2X0);

    // Enable Interrupts on receive character
    UCSR0B = (1 << RXCIE0)|(1 << RXEN0)|(1 << TXEN0);
    UCSR0C = (1 << UCSZ00)|(1 << UCSZ01);
    sei();

    // Save the baud rate in EEPROM
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_BAUDRATE, baud);

    // Return the rate that we are using.
    return baud;
}

//////////////////////////////////////////////////////////////////////////////
///
/// Flush all input in the serial buffer
///
void
serial_flush (void)
{
    // Disable interrupts and reset the ring buffer variables.
    cli();
    rx_head = 0;
    rx_tail = 0;
    rx_count = 0;
    rx_pause = 0;
    sei();

    // Send a XON to tell the host to resume sending and re-enable
    // reception
    serial_putc (CHAR_XON);
}

//////////////////////////////////////////////////////////////////////////////
///
/// Peek into the RX_buffer and retrieve a character from the RX_buffer before
/// it is at the head of the queue. The method blocks until a character has
/// been received.
///
/// @param [in] offset The offset from the read position. A value of '0' would
///                    be the next character to be read with rx_read();
///
/// @return The character at the peek position.
///
char
serial_peek (uint16_t offset)
{
    // Wait for the the character to enter the RX_buffer.
    while (rx_count <= offset)
    {
        // Reset the watchdog so it does not fire
        wdt_reset(); 

        // We do not have enough characters. If we are blocking then initiate
        // an unblock. Because the peek is requested in internally then we
        // assume that the buffer is large enough for the peek that we
        // require so we do not need to check the XON threshold.
        if (rx_pause != 0)
        {
            // Send a XON to tell the host to resume sending and re-enable
            // reception
            serial_putc (CHAR_XON);
            rx_pause = 0;
        }
    }

    // The byte has arrived in the rx_buffer, calculate the position to read
    // the buffer.
    offset += rx_tail;
    if (offset >= RX_BUFFER_SIZE)
        offset -= RX_BUFFER_SIZE;

    // Reset the watchdog so it does not fire
    wdt_reset(); 
    
    // Return the character.
    return rx_buffer [offset];
}

//////////////////////////////////////////////////////////////////////////////
///
/// Read a byte from the RX_buffer from the head of the queue. The method
/// blocks until a character has been received.
///
/// @return The character at the read position.
///
char
serial_getc (void)
{
    char cc;

    // Wait for data to be available
    while (rx_count == 0)
    {
        // Reset the watchdog so it does not fire
        wdt_reset(); 
    }
        
    // Get char from buffer and increment read pointer. If the read pointer
    // reaches the end of the buffer, wrap back to the beginning.
    cc = rx_buffer [rx_tail++];
#if RX_BUFFER_SIZE != 265
    if (rx_tail >= RX_BUFFER_SIZE)
        rx_tail = 0;
#endif
    // Interrupts must be disabled when changing rx_count, since it can be
    // changed here and in the ISR.
    cli();
    rx_count--;
    sei();

    // Check to see if we need to re-enable reception
    if (rx_pause != 0)
    {
        // USART reception is currently suspended re-enable reception if the
        // RX_buffer is suitably empty
        if (rx_count < prefs_xon)
        {
            // Send a XON to tell the host to resume sending and re-enable
            // reception
            serial_putc (CHAR_XON);
            rx_pause = 0;
        }
    }

    // Reset the watchdog so it does not fire
    wdt_reset(); 
    
    // Return the character to the caller.
    return cc;
}

//////////////////////////////////////////////////////////////////////////////
///
/// Flush n bytes from the RX_buffer at the head of the queue. 
///
/// @param [in] bytes The number of bytes to remove from the queue.
///
/// @return the number of bytes flushed.
///
uint8_t 
serial_flushc (uint8_t bytes)
{
    // Interrupts must be disabled when changing rx_count, since it can be
    // changed here and in the ISR. 
    cli();
    if (rx_count >= bytes)
        rx_count -= bytes;
    else
    {
        bytes = rx_count;
        rx_count = 0;
    }
    sei();
    
    // Adjust the tail to match
    rx_tail += bytes;
#if RX_BUFFER_SIZE != 265
    if (rx_tail >= RX_BUFFER_SIZE)
        rx_tail -= RX_BUFFER_SIZE;
#endif
    // Check to see if we need to re-enable reception
    if (rx_pause != 0)
    {
        // USART reception is currently suspended re-enable reception if the
        // RX_buffer is suitably empty
        if (rx_count < prefs_xon)
        {
            // Send a XON to tell the host to resume sending and re-enable
            // reception
            serial_putc (CHAR_XON);
            rx_pause = 0;
        }
    }

    // Return the number of bytes removed to the caller.
    return bytes;
}

//////////////////////////////////////////////////////////////////////////////
///
/// Put a character to the serial.
///
/// @param [in] cc The character to write on the TX.
///
void
serial_putc (char cc)
{
    // Wait for empty transmit buffer.
    while (!( UCSR0A & (1 << UDRE0)))
        /* Do nothing */ ;

    // Put data into buffer, sends the data
    UDR0 = cc;
}

/////////////////////////////////////////////////////////////////////////////
///
/// USART interrupt handler.
///
/// Read the data from the serial port and add to the rx_buffer.
///
ISR (USART_RX_vect)
{
    cli();                              // Disable Interrupts

    rx_buffer [rx_head++] = UDR0;       // Get recieved byte
#if RX_BUFFER_SIZE != 265
    if (rx_head >= RX_BUFFER_SIZE)
        rx_head = 0;                    // Wrap to start of buffer
#endif
    if (rx_count == 255)
        serial_putc (0xff);

    rx_count++;                         // Keep count of bytes in buffer

    // Test for the receive buffer close to full, if we can transmit without
    // blocking the ISR then send an XOFF .
    if (rx_count > prefs_xoff)
    {
        if ((UCSR0A & (1 << UDRE0)))
        {
            UDR0 = CHAR_XOFF;           // Send XOFF
            rx_pause = 1;               // Flag reception suspended
        }
    }

    sei();                              // Enable Interrupts
}
