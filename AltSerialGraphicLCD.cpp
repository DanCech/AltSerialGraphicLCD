/* -!- C++ -!- ********************************************* Alternative
 * Graphic Serial LCD Libary Header File
 * 
 * Parts of this library are based on the original Sparkfun library by:
 *
 * Joel Bartlett 
 * SparkFun Electronics
 * 9-25-13
 * 
 * Jon Green - 205-04-01 
 * New interface and in-line definitions added 2015-04-01 and library
 * re-formatted
 * 
 * Copyright (c) 2015 Jon Green
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 ***************************************************************************/

#include <stdarg.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "AltSerialGraphicLCD.h"

static char const _crlf[] PROGMEM = "\r\n";

//----------------------------------------------------------------------------
// Constructor
GLCD::GLCD(SoftwareSerial& software_serial)
      : serial(software_serial)
{
    blocked = 0;                        // Assume unblocked.
    graphics_on = 0;                    // Graphics sending is off.
    crlf = _crlf;                       // The end of line string
}

#ifdef GLCD_GET_IS_REQUIRED             // We do not use this disable.
//----------------------------------------------------------------------------
// Gat a character from the screen, if there is nothing available then return
// -1.
int
GLCD::get ()
{
    int cc;

    // Silently consume the XON/OFF and return anything else to the caller.
    while ((cc = serial.read()) != -1)
    {
        // mask out any top bit.
        uint8_t rc = cc & 0x7f;

        // Check for a XON/XOFF signal
        if (rc == GLCD_CHAR_XON)
        {
            // Unblocked, reset the flag and countdown.
            blocked = 0;
        }
        else if (rc == GLCD_CHAR_XOFF)
        {
            blocked = 1;                // Blocking
        }
        break;
    }

    // Return the character
    return cc;
}
#endif                                  // End we do not use this disable

//----------------------------------------------------------------------------
// Wait until the  screen is ready to receive a character.
void
GLCD::ready ()
{
    int rc;

    // Consume all of the input pending.
    while ((rc = serial.read()) != -1)
    {
        uint8_t uc = rc & 0x7f;

        // Process the XON/XOFF characters
        if (uc == GLCD_CHAR_XON)
            this->blocked = 0;
        else if (uc == GLCD_CHAR_XOFF)
            this->blocked = 1;
    }

    // Test to ensure that the screen is not requesting us to stop sending.
    if (this->blocked != 0)
    {
        unsigned long startMillis = millis();
        unsigned long nowMillis;

        // Keep polling the serial while we are blocked. Make sure we do not
        // block indefinitely by using a timer. There is a chance that we
        // migth drop an XON so we do not want to wait forever when nothing
        // might arrive.
        for (;;)
        {
            // Read the serial
            if ((rc = serial.read ()) != -1)
            {
                uint8_t uc = rc & 0x7f;

                // Process the XON/XOFF characters
                if (uc == GLCD_CHAR_XON)
                {
                    this->blocked = 0;
                    // Get out quickly
                    break;
                }
                else if (uc == GLCD_CHAR_XOFF)
                {
                    // Still blocked, reset the timer.
                    this->blocked = 1;
                    startMillis = millis();
                }
            }

            // Make sure we have not missed a XON character, if XON is not
            // received in 20 milliseconds then continue, there should be
            // enough buffer space to hold the command, we will get a XOFF if
            // the buffer is still full.
            nowMillis = millis();
            if ((nowMillis - startMillis) > 20)
            {
                // Assume we are unblocked.
                this->blocked = 0;
                break;
            }
            // Make sure that the counter does not wrap, if it does
            // (unlikely) then get another snapshot of the timer.
            else if (nowMillis < startMillis)
                startMillis = nowMillis;
        }
    }
}    

//----------------------------------------------------------------------------
// Put a character to the screen. Check that we are not blocked.
void
GLCD::put (uint8_t cc)
{
    // Wait for the screen to be ready.
    this->ready();
    // Send the character, we are not blocked.
    serial.write (cc);
}

/////////////////////////////////////////////////////////////////////////////
/// Print a flash string
///
/// @param [in] s The flash string to print.
/// const __FlashStringHelper *
void
GLCD::putstr_P (const char PROGMEM *s)
{
    uint8_t cc;
    uint8_t ii = 0;

    // Read to the end of the string.
    while ((cc = pgm_read_byte(s++)) != '\0')
    {
        // Test for XON/XOFF
        if ((ii & xoff_mask) == 0)
            this->ready ();             // XON/XOFF check
        serial.write (cc);              // Write the data.
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Print a regular string
///
/// @param [in] s The string to print.
///
void
GLCD::putstr (char *s)
{
    uint8_t cc;
    uint8_t ii = 0;

    // Read to the end of the string.
    while ((cc = *s++) != '\0')
    {
        // Test for XON/XOFF
        if ((ii & xoff_mask) == 0)
            this->ready ();             // XON/XOFF check
        serial.write (cc);              // Write the data
    }
}

//----------------------------------------------------------------------------
// Write a character block from RAM to the screen.
void
GLCD::write (uint8_t *data, int length)
{
    // Write out 'length' bytes of data from RAM
    while (length > 0)
    {
        uint8_t cc = *data++;

        // Periodically test for XON/XOFF
        if ((length & xoff_mask) == 0)
            this->ready ();
        serial.write (cc);
        length--;
    }
}

//----------------------------------------------------------------------------
// Write a character block from program memory to the screen.
void
GLCD::write_P (const uint8_t PROGMEM *data, int length)
{
    // Write out 'length' bytes of data from RAM
    while (length > 0)
    {
        uint8_t cc = pgm_read_byte(data++);

        // Periodically test for XON/XOFF
        if ((length & xoff_mask) == 0)
            this->ready ();
        serial.write (cc);
        length--;
    }
}

//----------------------------------------------------------------------------
// Put a command to the screen.
void
GLCD::putcmd (uint8_t cmd, uint8_t argc, ...)
{
    // Wait for the screen to be ready.
    this->ready ();
    
    // Check for graphics mode.
    if (graphics_on == 0)
        serial.write (GLCD_CHAR_CMD);
    
    // Push out the command.
    serial.write (cmd);

    // Pick up any arguments and send them.
    if (argc > 0)
    {
        uint8_t argm;                   // Argument mask
        uint8_t argd;                   // Dynamic argument
        va_list ap;                     // Variable argument list

        // Get the argument pointer.
        va_start (ap, argc);

        // Save the argument mask
        argm = argc;
        argc = argc & 7;                // There are max 6 arguments

        if (argc > 0)
        {
            do
            {
                uint8_t cc;                 // Temporary character

                cc = va_arg (ap, int) & 0xff; // Get variable argument.
                serial.write (cc);          // Send to the serial.
            }
            while (--argc > 0);
        }

        // Process the argument mask.
        if ((argd = (argm & GLCD_ARG_TYPE_MASK)) != 0)
        {
            uint8_t cc;
            int length;
            uint8_t *ptr;

            // Argument is (...., width, height, ptr)
            if (argd == GLCD_ARG_SPRITE_WH)
            {
                // Get the width;
                cc = (uint8_t)(va_arg (ap, int) & 0xff);
                serial.write (cc);
                length = cc;

                // Get the height. The height is specified in pixels and must
                // be converted to bytes
                cc = (uint8_t)(va_arg (ap, int) & 0xff);
                serial.write (cc);
                length *= (uint8_t)((cc + 7) >> 3);

                // Change the type to GLCD_ARG_SIZEOF for the reading of
                // data and process as (..., length, ptr).
                argd = GLCD_ARG_SIZEOF;
                ptr = va_arg (ap, uint8_t *);
            }
            // Argument is (...., length, ptr)
            else if (argd == GLCD_ARG_SIZEOF)
            {
                // Get the length.
                length = va_arg (ap, int);
                ptr = va_arg (ap, uint8_t *);
            }
            // Argument is (...., ptr) which point to sprite data.
            else if (argd == GLCD_ARG_SPRITE)
            {
                // Get the pointer.
                ptr = va_arg (ap, uint8_t *);

                // Get the width
                if ((argm & GLCD_ARG_PROGMEM) != 0)
                    cc = pgm_read_byte (ptr++);
                else
                    cc = *ptr++;
                serial.write (cc);
                length = cc;

                // Get the height. The height is specified in pixels and must
                // be converted to bytes
                if ((argm & GLCD_ARG_PROGMEM) != 0)
                    cc = pgm_read_byte (ptr++);
                else
                    cc = *ptr++;
                serial.write (cc);
                length *= (cc + 7) >> 3;

                // Change the type to GLCD_ARG_SIZEOF for the reading of
                // data and process as (..., length, ptr).
                argd = GLCD_ARG_SIZEOF;
            }
            // Argument is (..., ptr) where ptr is a list of (x,y)
            // coordinates terminated with the marker (y & 0x80 != 0)
            else if (argd == GLCD_ARG_XY_LIST)
            {
                // Handle PROGMEM
                if ((argm & GLCD_ARG_PROGMEM) != 0)
                {
                    const uint8_t PROGMEM *p;

                    p = va_arg (ap, const uint8_t PROGMEM *);
                    do
                    {
                        // Make sure we can write
                        this->ready();
                        
                        // Get x
                        cc = pgm_read_byte (p++);
                        serial.write (cc);
                        
                        // Get y
                        cc = pgm_read_byte (p++);
                        serial.write (cc);
                    }
                    while ((cc & 0x80) == 0);
                }
                // Handle regular memory
                else
                {
                    char *p = va_arg (ap, char *);
                    do
                    {
                        // Make sure we can write
                        this->ready();
                        
                        // Get x
                        serial.write (*p++);
                        
                        // Get y
                        cc = *p++;
                        serial.write (cc);
                    }
                    while ((cc & 0x80) == 0);
                }
            }
            else if (argd == GLCD_ARG_FFSTRING)
            {
                // Put the string
                if ((argm & GLCD_ARG_PROGMEM) != 0)
                    this->putstr_P (va_arg (ap, const char PROGMEM *));
                else
                    this->putstr (va_arg (ap, char *));
                // Terminate the string with 0xff
                serial.write (0xff);
            }

            // Argument is (..., length, ptr)
            if (argd == GLCD_ARG_SIZEOF)
            {
                // Perform a write operation.
                if ((argm & GLCD_ARG_PROGMEM) != 0)
                    this->write_P ((const uint8_t PROGMEM *) ptr, length);
                else
                    this->write (ptr, length);
            }
        }

        // Close the variable argument list.
        va_end (ap);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Wait for a character with a timeout.
//
int
GLCD::waitc (uint8_t expected, int msdelay)
{
    int cc;
    int ii;

    // Wait for a response of 'expected', we allow 2 seconds of inactivity to retrieve.
    ii = msdelay;
    for (;;)
    {
        // Break out if this is the query response.
        if ((cc = serial.read()) != -1)
        {
            uint8_t uc8 = cc & 0xff;

            // See if this is the character we are looking for
            if (expected != 0)
            {
                // If we are looking for a specific character then return
                // when we have a match.
                if (uc8 == expected)
                    break;
            }
            else
            {
                // We are looking for any character - return what we read.
                break;
            }

            // Handle XON/XOFF
            if (uc8 == GLCD_CHAR_XON)
                this->blocked = 0;
            else if (uc8 == GLCD_CHAR_XOFF)
                this->blocked = 1;

            // Reset the inactivity counter.
            ii = msdelay;
        }

        // Make sure we have not expired the loop.
        if (ii == 0)
            break;                      // Return error.

        // Delay 1ms and decrement the wait counter.
        delay (1);
        ii--;
    }

    // Must have timed out.
    return cc;
}

/////////////////////////////////////////////////////////////////////////////
/// Reset the screen.
///
void
GLCD::reset()
{
    int cc;                             // Working character

    // First make sure that we can communicate with the screen. If a bitblt
    // or polygon operation was interrrupted accross out reset then the
    // screen will be waiting for more characters so we need to feed it
    // before we perform the reset.
    do
    {
        // See if the screen is responsive. We use a non-drawable and
        // non-command character
        if ((cc = this->echoWait (0xf7, 1)) == -1)
        {
            // Un-responsive, push a dummy pixel. This will feed any bitblt
            // in addtion to terminating any polygon line, otherwise the
            // pixel draw will be clipped off-screen and do nothing.
            this->drawPixel (0xff, 0xff);
        }
    }
    while (cc != 0xf7);

    // Flush any pending write data.
    while (serial.read() != -1)
        /* Do nothing */;

    // We have re-established control of the screen, turn graphics off.
    this->graphics_on = 0;              // Graphics sending is off.

    // Get the size of the screen, we will be lazy and simply initialise by
    // getting the values straight from the screen and not deduce anything.
    // xdim = this->query (0x40);
    // ydim = this->query (0x41);
    while ((cc = this->query (10)) == -1)
        ;

    // Set up the dimensions
    if (cc == 0)
    {
        xdim = 128;
        ydim = 64;
    }
    else
    {
        xdim = 160;
        ydim = 128;
    }

    // Send a reset.
    this->putcmd (GLCD_CMD_RESET, 0);

    // Wait for up to 2s for the XON to indicate the screen has started. This
    // is lots of time and should be a lot quicker than this.
    this->waitc (GLCD_CHAR_XON, 2000);
    this->blocked = 0;
    // Finished - we are now in a usable initial state and can send commands.
}

/////////////////////////////////////////////////////////////////////////////
/// Query the screen for information.
///
/// @param [in] id The identitiy of the datum to retrieve
///
/// @return The data associated with the identity or -1 on error.
int
GLCD::query (uint8_t id)
{
    int cc;
    int dd;

    // Put the query command.
    this->putcmd (GLCD_CMD_QUERY, 1, id);

    // Wait for a 'Q' response to the query and return the next character
    // read.
    if ((cc = this->waitc ('Q', 2000)) == 'Q')
        cc = this->waitc (0, 500);
    return cc;
}

/////////////////////////////////////////////////////////////////////////////
/// Echo to the screen and wait for the character to come back
///
/// @param [in] echar The character to echo
/// @param [in] msdelay The millisecond delay
///
/// @return The character received.
int
GLCD::echoWait (uint8_t echar, int msdelay)
{
    int cc;

    // Put the query command.
    echo (echar);

    // Wait for the echo character to come back.
    return waitc (echar, msdelay);
}

//-------------------------------------------------------------------------------------------
// Change the baud rate.
//
// '1'/0x31/49 or 0x01 = 4800bps
// '2'/0x32/50 or 0x02 = 9600bps
// '3'/0x33/51 or 0x03 = 19,200bps
// '4'/0x34/52 or 0x04 = 38,400bps
// '5'/0x35/53 or 0x05 = 57,600bps
// '6'/0x36/54 or 0x06 = 115,200bps
void
GLCD::setBaud (byte baud)
{
    // Changes the baud rate.
    this->putcmd (GLCD_CMD_CHANGE_BAUD_RATE, 1, baud);
    delay(100);

    // Allow an integer argument.
    if (baud >= '0')
        baud -= '0';

    // These statements change the SoftwareSerial baud rate to match the baud
    // rate of the LCD.
    if ((baud >= 1) && (baud <= 6))
    {
        uint32_t rate;;

        // Stop the current serial session.
	serial.end();

        switch (baud)
        {
        case 1:
            rate = 4800;
            break;
        case 2:
            rate = 9600;
            break;
        case 3:
            rate = 19200;
            break;
        case 4:
            rate = 38400;
            break;
        case 5:
            rate = 57600;
            break;
        default:
            rate = 115200;
            break;
        }
        serial.begin (rate);
    }
}

//-------------------------------------------------------------------------------------------
void
GLCD::restoreDefaultBaud()
{
    //This function is used to restore the default baud rate in case you change it
    //and forget to which rate it was changed.

    serial.end();//end the transmission at whatever the current baud rate is

    // Cycle through every other possible buad rate and attempt to change the
    // rate back to 115200
    serial.end();
    serial.begin(4800);
    this->setBaud (6); //set back to 115200

    serial.end();
    serial.begin(9600);
    this->setBaud (6); //set back to 115200

    serial.end();
    serial.begin(19200);
    this->setBaud (6); //set back to 115200

    serial.end();
    serial.begin(38400);
    this->setBaud (6); //set back to 115200

    serial.end();
    serial.begin(57600);
    this->setBaud (6); //set back to 115200

    serial.end();
    serial.begin(115200);
    this->clearScreen();
    this->putstr (F("Baud restored to 115200"));
}
