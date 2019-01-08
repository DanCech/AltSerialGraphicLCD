/* -*- c++ -*- ***************************************************************
 *
 *  System      : Serial GLCD
 *  Module      : Main program
 *  Object Name : $RCSfile: main.c,v $
 *  Revision    : $Revision: 1.43 $
 *  Date        : $Date: 2015/07/05 21:06:58 $
 *  Author      : $Author: jon $
 *  Created By  : Jon Green
 *  Created     : Sun Apr 5 08:43:33 2015 Last Modified : <150612.2223>
 *
 *  Description : The main program for driving the serial 160x128 screen
 *
 *  Notes       : Derrived from the 128x64 code by Jennifer Holt and adapted
 *                for both the 128x64 and 160x128 screens.
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

#include <stdlib.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>		// F_CPU is defined in the makefile
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <avr/wdt.h>                    /* Watchdog timer */
#include <avr/io.h>

#include "glcd.h"

//////////////////////////////////////////////////////////////////////////////
// Constants
// 0 - Version major
// 1 - Version minor
// 2 - EEPROM Sprite size
// 3 - EEPROM Sprite number
// 4 - RAM Sprite size
// 5 - RAM Sprite number
const uint8_t consts[6] PROGMEM = 
{
    VERSION_MAJOR,                      // Version major
    VERSION_MINOR,                      // Version minor
    EEPROM_SPRITE_SIZE,                 // EEPROM Sprite size
    EEPROM_SPRITE_NUM,                  // EEPROM Sprite number
    SPRITE_SIZE,                        // RAM Sprite size
    NUM_SPRITES                         // RAM Sprite number
};

/***************************************************************************
 * Command Handling                                                        *
 ***************************************************************************/

// The different screen configurations are driven by a a function look-up
// table which is statically defined in PROGMEM. This saves us a considerable
// amount of memory. The pgm_read_* functions are zero over-head as compared
// with a RAM based table and the pgm_read_* functions disappear to a Flash
// access and generate the same code size.
//
// Different function tables are defined for the T6963 and the KS0108b chips.
// We use a function table pointer to point to the table that we require and
// access the functions indirectly through the table.
//
// The cmdtable structure provides a look-up table for the command. The
// tables are separated on function but are all aligned so they use the same
// index for each table access. The cmdtables are defined as follows:
//
// cmdtable_cmdsP - Contains the serial command character.
//
// cmdtable_argsP - Contains the arguments of the command.
//
// cmdtable_funcsP - Contains the index into the function table of the
// command to execute.
//
// When a serial command is processed then it is looked up in the
// cmdtable_cmdsP table. This lookup is performed with a binary chop on the
// table which gives us a an order of logN() look-up time. If a match is
// found then the index of the entry is then used to determine the argument
// format and the function to invoke is defined in cmdtable_funcsP.

// T6963 function pointers, indexed by the enumerated name
static const vfunc_t t6963_functabP [] PROGMEM =
{
#define DEFFUNC(enum_name, t6963_function, ks0108b_function)     (vfunc_t) t6963_function,
#define ENDFUNC(enum_name, t6963_function, ks0108b_function)     (vfunc_t) t6963_function
#include "func.def"
#undef DEFFUNC
#undef ENDFUNC
};

// KS0108b function pointers, indexed by the enumerated name
static const vfunc_t ks0108b_functabP [] PROGMEM =
{
#define DEFFUNC(enum_name, t6963_function, ks0108b_function)     (vfunc_t) ks0108b_function,
#define ENDFUNC(enum_name, t6963_function, ks0108b_function)     (vfunc_t) ks0108b_function
#include "func.def"
#undef DEFFUNC
#undef ENDFUNC
};

// Pointer to the function table in flash
const vfunc_t* functabP;

// KS0108b function pointers, indexed by the enumerated name
static const vfunc_t cmd_functabP [] PROGMEM =
{
#define DEFCMDFUNC(enum_name, function)     (vfunc_t) function,
#define ENDCMDFUNC(enum_name, function)     (vfunc_t) function
#include "func.def"
#undef DEFCMDFUNC
#undef ENDCMDFUNC
};

// Lookup table of serial command codes
static const uint8_t cmdtable_cmdsP [] PROGMEM =
{
#define DEFCMD(enum_value, enum_name, args, func) enum_value,
#define ENDCMD(enum_value, enum_name, args, func) enum_value
#include "func.def"
#undef DEFCMD
#undef ENDCMD
};

// Look-up table of arguments
static const uint8_t cmdtable_argsP [] PROGMEM =
{
#define DEFCMD(enum_value, enum_name, args, func) args,
#define ENDCMD(enum_value, enum_name, args, func) args
#include "func.def"
#undef DEFCMD
#undef ENDCMD
};

// Lookup table of functions
static const uint8_t cmdtable_funcsP [] PROGMEM =
{
#define DEFCMD(enum_value, enum_name, args, func) func,
#define ENDCMD(enum_value, enum_name, args, func) func
#include "func.def"
#undef DEFCMD
#undef ENDCMD
};

uint8_t prefs [PREFS_ADDR_MAX];         // EEPROM preferences.
uint8_t x_dim;                          // X dimension
uint8_t y_dim;                          // Y dimension

/*
 * Bit shift mask - array of masks 0=0xff; 1=0x7f, 2=0x3f etc. Replaces long
 * shift operations with lookup table
 */
const uint8_t bit_shift_maskP[8] PROGMEM =
{
    0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
};

/*
 * Single bit shift mask - array of masks 0=0x01; 1=0x02, 2=0x04 etc. Replaces
 * long shift operations with lookup table
 */
const uint8_t bit_shift_single_maskP[8] PROGMEM =
{
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

/*
 * Single bit shift mask - array of masks 0=0x80; 1=0x40, 2=0x30 etc. Replaces
 * long shift operations with lookup table
 */
const uint8_t bit_shift_rev_single_maskP[8] PROGMEM =
{
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

/////////////////////////////////////////////////////////////////////////////
/// Set the graphics mode
void
graphics_mode (uint8_t cmd)
{
    prefs_graphics = (cmd == CMDX_GRAPHICS_ON) ? 1 : 0;
}

// The main loop. This performs the initialisation and general operation.
int
main (void)
{
    int cc;

    // Set the watchdog timer for 2 seconds. Note that the lcd_reset()
    // function changes the timer to 64ms so this must be reset on input.
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
    WDTCSR = ((1 << WDIE)|(1 << WDE)|(0 << WDP3)|(1 << WDP2)|(1 << WDP1)|(1 << WDP0));
    // Enable interrupts
    sei();

    // Read in the EEPROM
    memset (prefs, 0, sizeof (prefs));
    for (cc = EEPROM_ADDR_MAGIC; cc < EEPROM_ADDR_MAX; cc++)
        prefs [cc] = eeprom_read_byte ((const uint8_t *)(cc));

    // Check for a valid magic, if so then we will accept the content
    if (is_invalid_magic() ||
        baud_rate_invalid(prefs_baudrate) ||
        ((prefs_reverse & ~MODE_NORMAL_MASK) != 0))
    {
        lcd_factory_reset ();
    }

#if 0
    // These are the default values, usually these should get read from
    // EEPROM, if that does not work for you uncomment these and set them to
    // whatever you want.
    lcd_factory_reset ();
    memset (prefs, 0, sizeof (prefs));
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_REVERSE, MODE_NORMAL);
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_BAUDRATE, BAUD_RATE_DEFAULT);
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_BACKLIGHT, 100);
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_XON_POS, RX_BUFFER_XON);
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_XOFF_POS, RX_BUFFER_XOFF);
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_LARGE_SCREEN, 0x08);
    // 0x08 = Large screen; 0x00 = Small screen
#endif
    
    // The PINB is tempermental on the watchdog reset and returns the
    // incorrect value. This has been moved to the lcd_factory_reset() and is
    // now written to EEPROM.
    if (is_large())
    {
        // The pin is high, this is a large display. Install the large
        // function table.
        functabP = t6963_functabP;
    }
    else
    {
        // The pin is low, this is a small display. Install the small
        // function table.
        functabP = ks0108b_functabP;
    }

    // Set the backlight to the correct level
    backlight_init ();                  // Initialise the backlight.
    prefs_backlight = backlight_level (prefs_backlight, 0);

    // Initialise display
    ((vfunc_t)(pgm_read_word(&functabP [F_DRV_INIT])))();

    // Initialise the fonts.
    font_set (prefs_font, CMDX_FONT_SET);
    font_mode (MODE_NORMAL);

    // Set the drawing modes to an initialised state of normal
    drawing_mode = MODE_NORMAL;

    // Set the baud rate to the user preference
    prefs_baudrate = serial_baudrate (prefs_baudrate);

    // Flush the serial and send the user an XON
    serial_flush ();

    // Display the splash screen logo this is sprite zero
    if (is_splash())
    {
        // Invoke demo mode to display the splash screen. This is not the
        // demo command.
        lcd_demo (0);
    }
    else
    {
        // Clear the screen
        lcd_screen_clear ();
    }

    /***********************************************************************
     *                                                                     *
     * Main loop                                                           *
     *                                                                     *
     ***********************************************************************/

    // The main loop waits for characters on the serial port, and either
    // prints them, or interprets them as a command
    for (;;)
    {
next_command:
        // Get a byte from the serial port and parse it.
        cc = serial_getc();

        // Deterimine our operational mode, if we are in a grapics mode then
        // we only process commands, otherwise we look for text to render.
        if (!is_graphics())
        {
            // We are in non-graphcis mode, process the character
            if (cc < ' ')
            {
                // Process any special control characters.
                if (cc >= CHAR_LF)
                {
                    // LF move to next row
                    if (cc == CHAR_LF)
                        font_lf ();
                    // CR, reset to start of row
                    else if (cc == CHAR_CR)
                        font_cr ();
                }
                else
                {
                    // Backspace
                    if (cc == CHAR_BACKSPACE)
                        font_backspace ();
                }
                // We ignore anything else and do not draw it.
            }
            else
            {
                // See if this is a control character.
                if (cc == CHAR_COMMAND)
                {
                    // Received character indicated a coming command. Get the
                    // command character and process it.
                    cc = serial_getc();

                    // If the character is not the command character then process
                    // as a command.
                    if (cc != CHAR_COMMAND)
                        goto graphics_command;
                }

                // Otherwise draw the character
                font_draw (cc);
            }
        }

        /////////////////////////////////////////////////////////////////////
        // Graphics command                                                //
        /////////////////////////////////////////////////////////////////////
        else
        {
graphics_command:

            // Find the command by performing a binary chop operation on the
            // command table. This gives us the index of the function. We
            // then use the argument table to determine the arguments of the
            // call before finalling invoking the function indirectly via the
            // function table.
            {
                int8_t hi = sizeof(cmdtable_cmdsP) - 1; // Hi watermark
                int8_t lo = 0;                          // Lo watermark

                do
                {
                    // Find the entry from the middle of the command table
                    int8_t  mid  = (lo + hi) >> 1;      // Get the mid value
                    uint8_t tcmd = pgm_read_byte (&cmdtable_cmdsP[mid]);

                    if (cc == tcmd)
                    {
#define argc hi                         /* Re-use the variable */
#define argf lo                         /* Re-use the variable */
                        void *func = pgm_read_word(&cmd_functabP[pgm_read_byte (&cmdtable_funcsP[mid])]);
                        uint8_t argv[6];

                        argf = pgm_read_byte (&cmdtable_argsP[mid]);
                        argc = 0;

                        // Get any pre arguments that need to be pushed
                        // before arguments acquired over the serial port.
                        if (argf & FUNC_PRE_DRAW_MODE)
                            argv[argc++] = drawing_mode;

                        // Get the arguments from serial. */
                        while (argc < (argf & FUNC_ARGC_MASK))
                            argv[argc++] = serial_getc ();

                        // Set any default arguments
                        if (argf & FUNC_DRAW_ZERO)
                            argv[argc++] = 0;
                        if (argf & FUNC_FILL_CMD)
                            argv[argc++] = cc;
                        if (argf & FUNC_DRAW_MODE)
                            argv[argc++] = drawing_mode;

                        // Manually perform a binary chop on the number of
                        // arguments to set up the function call.
                        // Unfortunately there is no way that I know of in
                        // 'C' to manually buld a stack frame. I would have
                        // to drop down to assembler to do this a bit more
                        // efficiently and it is not that critical to devote
                        // my time to sorting this out.
                        if (argc <= 3)
                        {
                            // 3 or less arguments.
                            if (argc <= 1)
                            {
                                // Zero or 1 argument.
                                if (argc == 0)
                                    ((vfunc_t) func)();
                                else
                                    ((vfunc_i_t) func)(argv[0]);
                            }
                            else
                            {
                                // 2 or 3 arguments. Noted a special case of
                                // arguments if we have a FUNC_DRAW_NULL
                                // which is a special pointer argument of
                                // NULL
                                if (argc == 2)
                                    ((vfunc_ii_t) func)(argv[0], argv[1]);
                                else
                                {
                                    // All of the NULL terminated commands are 4 bytes long.
                                    if (argf & FUNC_DRAW_NULL)
                                        ((vfunc_iiip_t) func)(argv[0], argv[1], argv[2], NULL);
                                    else
                                        ((vfunc_iii_t) func)(argv[0], argv[1], argv[2]);
                                }
                            }
                        }
                        else
                        {
                            // 4 or more arguments
                            if (argc <= 5)
                            {
                                // 4 or 5 arguments
                                if (argc == 5)
                                    ((vfunc_iiiii_t) func)(argv[0], argv[1], argv[2], argv[3], argv[4]);
                                else
                                    ((vfunc_iiii_t) func)(argv[0], argv[1], argv[2], argv[3]);
                            }
                            else
                            {
                                // 6 arguments.
                                ((vfunc_iiiiii_t) func)(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
                            }
                        }

                        // The command has been executed. Move onto the next command.
                        goto next_command;
                    }
                    else if (cc > tcmd) // Binary chop - work out which part of table to keep
                        lo = mid + 1;   // Discard top half
                    else
                        hi = mid - 1;   // Discard bottom half
#undef argc                             /* Re-use the variable */
#undef argf                             /* Re-use the variable */
                }
                // Continue the binary chop until we run out of entries to
                // chop.
                while (lo <= hi);
            }

            // At this point the command character is not recognised or
            // processed. Where we have received a command character
            // indicated a coming command. We are in the wrong mode, flip
            // back to a non-graphics mode and process.
            if (cc == CHAR_COMMAND)
            {
                // Drop out of graphics mode as we have received a command character.
                prefs_graphics = 0;

                // Get the command character and then go and process it.
                cc = serial_getc();
                goto graphics_command;
            }

            /////////////////////////////////////////////////////////////
            // Ignore anything else and simply loop
        }
    }
}

/* Watchdog timer interrupt */
ISR (WDT_vect)
{
    ; /* Do nothing - we do not have anything to do, just let the system reset. */
}
