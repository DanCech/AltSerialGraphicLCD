/* -*- c++ -*- ***************************************************************
 *
 *  System      : Serial GLCD
 *  Module      : Backlight handling
 *  Object Name : $RCSfile: backlight.c,v $
 *  Revision    : $Revision: 1.9 $
 *  Date        : $Date: 2015/05/31 19:12:12 $
 *  Author      : $Author: jon $
 *  Created By  : Jon Green
 *  Created     : Sun Apr 5 08:43:33 2015 Last Modified : <150530.1110>
 *
 *  Description : Handles the backlight function
 *
 *  Notes       : Derrived from the code by Jennifer Holt and adapted for the
 *               160x128 screen. The backlight timer code was lifted from Mike
 *               Hord, Sparkfun which uses a neat trick with the timers that
 *               I struggle to understand but it seems to work nicely.
 *
 *  History     :
 *
 *****************************************************************************
 *
 *  02 May 2013 - Mike Hord, SparkFun Electronics
 *
 * This code is released under the Creative Commons Attribution Share-Alike 3.0
 * license. You are free to reuse, remix, or redistribute it as you see fit,
 * so long as you provide attribution to SparkFun Electronics.
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
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/io.h>

#include "glcd.h"

// PB2 is backlight enable, and is active low
#define nBL_EN  2

//////////////////////////////////////////////////////////////////////////////
///
/// Initialise the backlight hardware.
///
void
backlight_init (void)
{
    // Turn the backlight of
    PORTB |= 1 << nBL_EN;               // Set backlight off.
    DDRB  |= 1 << nBL_EN;               // Set PB2 as output
}

//////////////////////////////////////////////////////////////////////////////
///
/// Change the backlight level.
///
/// @param [in] level A value between 0 and 100 where 0 is off, 100 is
///                   the brightest level.
/// @param [in] cmd   The command that we are executing. If it is a
///                   CMD_SET_BACKLIGHT then save to EEPROM.
///
/// @return The current level.
///
uint8_t
backlight_level (uint8_t level, uint8_t cmd)
{
    // Accept the new value and make sure it is in a valid range.
    if (level > 100)
        level = 100;

    // Full brightness
    if (level >= 100)
    {
        // Disable Timer1
        TCCR1A = 0;
        TCCR1B = 0;

        // Turn backlight on
        PORTB &= ~(1 << nBL_EN);
    }
    // Turn backlight off
    else if (level == 0)
    {
        // Disable Timer1
        TCCR1A = 0;
        TCCR1B = 0;

        // Set the backlight off
        PORTB |= 1 << nBL_EN;
    }
    // Some setting in the middle.
    else
    {
        // Backlight on
        PORTB &= ~(1 << nBL_EN);

        // Timer1 initialization
        // We use timer 1 fast PWM mode to dim the backlight on the display.
        // OC1B (PB2) is connected to a BJT for controlling the backlight; the
        // BJT is PNP so we want to use inverting mode.
        //
        // PWM frequency is fclk/(N*(1+TOP)), where TOP is, in this case 100,
        // N=1, and fclk is 16MHz. Thus, Fpwm ~ 160kHz.
        //
        // TCCR1A-  7:6 - Channel A compare output mode
        //                 Set to 00 for normal pin operation
        //          5:4 - Channel B compare output mode
        //                 Set to 01 for inverting PWM output mode
        //          3:2 - Don't care/no use
        //          1:0 - Waveform generation mode bits 1:0
        //                 Along with WGM1 3:2 (In TCCR1B), set to 1111 to enable
        //                 fast PWM mode. TCNT1 will increment until it reaches ICR1,
        //                 then reset, and the pin will change when TCNT1 == 0 and
        //                 when TCNT1 == OCR1B.
        TCCR1A = 0b00110010;

        // TCCR1B-  7   - Input noise canceler (Don't care)
        //          6   - Input capture edge select (Don't care)
        //          5   - Don't care/no use
        //          4:3 - Waveform generation mode bits 3:2
        //                 See above; set to 11 for fast PWM
        //          2:0 - Timer 1 clock source
        //                 Set to 001 for no clock divisor.
        TCCR1B = 0b00011001;

        // ICR1- Really implemented as two 8-bit registers (ICR1L and ICR1H), the
        // value in this register (in this mode) marks the point at which the
        // timer quits counting and returns to zero. By making it 100, we can
        // then really easily set our backlight intensity from 0-100.
        ICR1 = 100;

        // OCR1B- Really implemented as two 8-bit registers (OCR1BL and OCR1BH),
        // the value in this register is the point where the output pin will
        // transition from low to high, turning the backlight off.
        OCR1B = level;
    }

    // Save the setting if required.
    if (cmd == CMD_SET_BACKLIGHT)
        lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_BACKLIGHT, level);

    // Return the level to the caller.
    return level;
}
