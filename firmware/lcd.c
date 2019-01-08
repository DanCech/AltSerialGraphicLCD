/* -*- c++ -*- ***************************************************************
 *
 *  System        : Serial GLCD
 *  Module        : Font Handling
 *  Object Name   : $RCSfile: lcd.c,v $
 *  Revision      : $Revision: 1.18 $
 *  Date          : $Date: 2015/07/05 21:07:37 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Sun Apr 5 08:43:33 2015
 *  Last Modified : <150612.2256>
 *
 *  Description   : Handles all of the font related
 *
 *  Notes         :
 *
 *  History     : Derrived from the code by Sparkfun and Jennifer Holt and
 *                adapted for the 160x128 screen.
 *
 *****************************************************************************/

/*****************************************************************************
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

#include <stdlib.h>
#include <string.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <avr/wdt.h>                    /* Watchdog timer */
#include <avr/io.h>

#include "glcd.h"

//////////////////////////////////////////////////////////////////////////////
/// Perform a reset of the screen
///
void
lcd_reset (void)
{
    // Change the watchdog timer for a short period and then allow the
    // processor to reset.
    cli();                              // Disable interrupts

    /* WDTCSR configuration:
     * WDIE = 1 - Interrupt Enable
     * WDE =  1 - Reset Enable
     * See table for time-out variations:
     *@+-----+-----+-----+-----+--------------+
     * |WDP 3|WDP 2|WDP 1|WDP 0|Time-out (ms) |
     * +-----+-----+-----+-----+--------------+
     * |  0  |  0  |  0  |  0  |    16        |
     * |  0  |  0  |  0  |  1  |    32        |
     * |  0  |  0  |  1  |  0  |    64        |
     * |  0  |  0  |  1  |  1  |   125        |
     * |  0  |  1  |  0  |  0  |   250        |
     * |  0  |  1  |  0  |  1  |   500        |
     * |  0  |  1  |  1  |  0  |  1000        |
     * |  0  |  1  |  1  |  1  |  2000        |
     * |  1  |  0  |  0  |  0  |  4000        |
     * |  1  |  0  |  0  |  1  |  8000        |
     * +-----+-----+-----+-----+--------------+
     */

    // Enter Watchdog Configuration mode
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    // Set Watchdog settings
    WDTCSR = ((1 << WDIE)|(1 << WDE)|(0 << WDP3)|(0 << WDP2)|(1 << WDP1)|(0 << WDP0));
    // Enable interrupts
    sei();

    // Spin until we reset on the watchdog.
    for(;;)
        ;

    // NO EXIT - THE WATCHDOG WILL RESTART THE PROCESSOR
}

//////////////////////////////////////////////////////////////////////////////
/// Clear the screen
///
void
lcd_screen_clear (void)
{
    // Invoke the screen driver to clear the screen with the reverse
    // preference which will redraw the screen in the normal/reverse mode.
    ((vfunc_i_t)(pgm_read_word(&functabP [F_DRV_SCREEN_CLEAR])))(prefs_reverse);
    font_position (0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////
/// Perform a factory reset on EEPROM.
///
void
lcd_factory_reset (void)
{
    int ii;                             // Loop pointer
    uint8_t *ptr;                       // Memory pointer

    // Define the Sparkfun logo.
    static const uint8_t sparkfun_logo [] PROGMEM =
    {
        0x0A,0x10,                      // Width x height
        0x80,0xc0,0x40,0x0c,0x3e,0xfe,0xf2,0xe0,0xf0,0xe0,
        0xff,0x7f,0x3f,0x1f,0x1f,0x1f,0x1f,0x0f,0x07,0x03
    };

    // Reset the EEPROM to its factory default setting.
    memset (prefs, 0, sizeof (prefs));  // Reset everything to zero.

    // Set magic number to indicate we have written memory
    prefs [EEPROM_ADDR_MAGIC] = EEPROM_MAGIC;
    // Set default baud rate.
    prefs_baudrate = BAUD_RATE_DEFAULT;
    // Assume normal mode operation.
    prefs_reverse = MODE_NORMAL;
    // Set the backlight to full
    prefs_backlight = 100;
    // Set the splash screen on
    prefs_splash = 1;
    // Set the XON/XOFF preferences
    prefs_xon = RX_BUFFER_XON;
    prefs_xoff = RX_BUFFER_XOFF;
    
    // Read the PINB screen configuration
    // The first thing to do is to check is if we have a large or small
    // display. We can tell because PB3 will be pulled high if the display is
    // large (hopefully; that's done at build time). The default
    // configuration is set up for the large display so we only need to
    // change the configuration if it is a small display.
    // 
    // The line needs some time to settle once we have enabled it. We enable
    // the line now and then do some work while it settles and then come back
    // and read it. 
    PORTB |= 0x08;                      // Enable the pull-up on PB3.
    _delay_us (5);
    prefs_large = PINB & 0x08;
    PORTB &= ~0x08;    
    
    // Noted that reading PIN B read is sometimes incorrect. If you want to
    // force the screen size then over-ride the PIN reading and explicitly
    // set the screen size you are using and then the issue will be resolved. 
    // prefs_large = 0x08;              // Large 160x128 screen.
    // prefs_large = 0x00;              // Small 128x64 screen.
    
    // Write back the correct values for factory default.
    for (ii = EEPROM_ADDR_MAGIC; ii < EEPROM_ADDR_MAX; ii++)
        eeprom_write_byte ((uint8_t *)(ii), prefs [ii]);

    // Write the sparkfun logo into EEPROM
    ptr = (uint8_t *)(sparkfun_logo);
    for (ii = 0; ii < sizeof (sparkfun_logo); ii++)
    {
        eeprom_write_byte ((uint8_t *)(EEPROM_ADDR_SPRITE_START + ii),
                           pgm_read_byte (ptr++));
    }

    // Clear the remaining EEPROM sprite memory skipping over the sparkfun
    // logo we have just written.
    ptr = (uint8_t *)(EEPROM_ADDR_SPRITE_START + EEPROM_SPRITE_SIZE);
    for (ii = 0; ii < ((EEPROM_SPRITE_NUM - 1) * EEPROM_SPRITE_SIZE); ii++)
    {
        eeprom_write_byte (ptr++, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Set the value of a EEPROM location. The value is stored in EEPROM.
///
/// @param [in] check A check byte to ensure this is a set command
/// @param [in] id The identiy of the EEPROM location to change.
/// @param [in] value The value to assign to the EEPROM location.
///
void
lcd_set (uint8_t check, uint8_t id, uint8_t value)
{
    // Basic check to ensure that the first byte is the check byte.
    if (check != LCD_SET_CHECKBYTE)
        return;                         // Do not set anything.
    // Check the range of the identity.
    if (id > EEPROM_ADDR_MAX)
        return;                         // Do not set anything.Out of range
    // Set the value, there is no checking of the value used. 
    prefs[id] = value;
    // Read the value from EEPROM, if it is different then write the new
    // value. Keep the compiler quiet by casting twice to the correct size.
    if (eeprom_read_byte ((uint8_t *)((uint16_t)(id))) != value)
        eeprom_write_byte ((uint8_t *)((uint16_t)(id)), value);
}

//////////////////////////////////////////////////////////////////////////////
/// Query the internal state of the system. Return the settings to the caller
/// via the serial. The format is defined as follows:
///
/// Query == 0 -- Screen dimesion
/// byte[0] = Q
/// byte[1] = <x-dim>
/// byte[2] = <y-dim>
///
/// Query == 1 -- Preferences
/// byte[0] = <Baud rate>
/// byte[1] = <Backlight level>
/// byte[2] = <Splash screen>
/// byte[3] = <Reverse screen>
/// byte[4] = <debug>
/// byte[5] = <crlf>
/// byte[6] = <xon position>
/// byte[7] = <xoff position>
/// byte[8] = <scroll>
///
/// Query == 2 -- Version string
/// byte[0] = <version string>0x00
///
/// Query == 3 -- EEPROM sprites
/// byte[0]  = count
/// byte[1..count] = EEPROM width
///
void
lcd_query (uint8_t info)
{
    uint8_t cc = 0xff;
    
    // Put the X andy dimensions.
    serial_putc ('Q');
    // EEPROM locations.
    if (info < EEPROM_ADDR_MAX)
        cc = prefs[info];
    // Constant locations
    else if (info & 0x20)
    {
        info &= ~0x20;
        if (info < sizeof (consts))
            cc = pgm_read_byte (&consts[info]);
    }
    // x & y dimensions
    else if (info & 0x40)
    {
        if (info & 1)
            cc = y_dim;
        else
            cc = x_dim;
    }
    // The sprite EEPROM widths
    else if (info & 0x80)
    {
        const uint8_t *eeprom_addr = (uint8_t *) EEPROM_ADDR_SPRITE_START; // Address in EEPROM
        uint8_t height_offset;
        
        height_offset = info & 1;
        info &= ~0x80;
        info >>= 1;                     // Divide by 2;
        if (info < EEPROM_SPRITE_NUM)
        {
            cc = eeprom_read_byte (&eeprom_addr[(EEPROM_SPRITE_SIZE * info) + height_offset]);
        }
    }
    serial_putc (cc);        
}

//////////////////////////////////////////////////////////////////////////////
/// Reverse the screen
///
/// @param [in] mode  The mode to assign to the reverse preference.
/// @param [in] cmd   The command that we are executing. If it is a
///                   CMD_REVERSE_MODE then save to EEPROM.
void
lcd_screen_reverse (uint8_t mode, uint8_t cmd)
{
    // Save the user preference if this is a persistent command.
    if (cmd == CMD_REVERSE_MODE)
    {
        // Determine which way round the screen is.
        if (is_reverse())
            mode = MODE_NORMAL;
        else
            mode = MODE_REVERSE;
        
        // Set the EEPROM values
        lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_REVERSE, mode);
    }
    else
        prefs_reverse = mode & MODE_NORMAL_MASK;

    // Reverse the screen.
    ((vfunc_p_t)(pgm_read_word(&functabP [F_DRV_SCREEN_REVERSE])))(draw_buffer);
}

/////////////////////////////////////////////////////////////////////////////
/// Demonstration
void
lcd_demo (uint8_t cmd)
{
    // Clear the screen
    lcd_screen_clear ();

    // Only draw the single sprite when the demo is not invoked.
    if ((cmd != CMD_DEMO) && (prefs_splash == 2))
    {
        // XOR'ing the sprite works in normal and reverse modes
        sprite_draw (x_dim/2, y_dim/2, 0x80, MODE_XOR|MODE_SPRITE_CENTER|MODE_NORMAL);
    }
    else
    {
        static const uint16_t baudrates [] PROGMEM =
        {
            48,   /* 1 */
            96,   /* 2 */
            192,  /* 3 */
            384,  /* 4 */
            576,  /* 5 */
            1152  /* 6 */
        };

        static const char label_0[] PROGMEM = "Baudrate:";
        static const char label_1[] PROGMEM = "Splash  :";
        static const char label_2[] PROGMEM = "CRLF    :";
        static const char label_3[] PROGMEM = "Scroll  :";
        static const char label_4[] PROGMEM = "B'Light :";
        static const char label_5[] PROGMEM = "xon/xoff:";
        static const char label_6[] PROGMEM = "Version :";
        static const char * const labels[] PROGMEM =
        {
            label_0, label_1, label_2, label_3, label_4, label_5, label_6
        };
        static const char slash[] PROGMEM = "/";
        static const char zero2[] PROGMEM = "00";
        uint8_t yy = (y_dim/2) - 32;
        uint8_t xx;
        uint8_t ii;
        
        // Set the font to the 6x8.
        font_set (0, CMDX_FONT_SET);
        
        // XOR'ing the sprite works in normal and reverse modes
        sprite_draw (100 + (x_dim - 100) / 2, y_dim/2, 0x80, MODE_XOR|MODE_SPRITE_CENTER|MODE_NORMAL);

        draw_rbox (0, yy, 100, yy + 63, 8, MODE_XOR|MODE_NORMAL);

        xx = 4;
        yy += xx;
        for (ii = 0; ii < 7; ii++)
        {
            font_position (xx, yy, 0);
            font_draw_stringP ((char *)(pgm_read_word(&labels[ii])));

            switch (ii)
            {
                // Baudrate: <value>
            case 0:
                font_draw_number (pgm_read_word (&baudrates[prefs_baudrate-1]));
                font_draw_stringP (zero2);
                break;

                // Splash  : <on>/<off>
            case 1:
                font_draw_on_off (is_splash());
                if (is_splash())
                {
                    font_draw_stringP (slash);
                    font_draw_number (prefs_splash);
                }
                break;

                // CRLF    : <on>/<off>
            case 2:
                font_draw_on_off (is_crlf());
                break;

                // Scroll  : <on>/<off>
            case 3:
                font_draw_on_off (is_scroll());
                break;

                // B'Light : <value>
            case 4:
                font_draw_number (prefs_backlight);
                break;

                // xon/xoff: <xon>/<xoff>
            case 5:
                font_draw_number (prefs_xon);
                font_draw_stringP (slash);
                font_draw_number (prefs_xoff);
                break;

                // Version : <major>.<minor>
            case 6:
                font_draw_number (VERSION_MAJOR);
                font_draw ('.');
                font_draw_number (VERSION_MINOR);
                break;
            }
            yy += 8;
        }
    }

    // Stay on this page until we get a character.
    serial_peek (0);
    // Set the font to the 6x8.
    font_set (prefs_font, CMDX_FONT_SET);
    lcd_screen_clear ();
}
