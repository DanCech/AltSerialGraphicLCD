/* -*- c++ -*- ***************************************************************
 *
 *  System      : Serial GLCD
 *  Module      : Sprite functions
 *  Object Name : $RCSfile: sprite.c,v $
 *  Revision    : $Revision: 1.13 $
 *  Date        : $Date: 2015/05/31 19:12:12 $
 *  Author      : $Author: jon $
 *  Created By  : Jon Green
 *  Created     : Sun Apr 5 08:43:33 2015 Last Modified : <150530.1117>
 *
 *  Description : The main program for driving the serial 160x128 screen
 *
 *  Notes       : Derrived from the 128x64 code by Jennifer Holt and adapted
 *                for the 160x128 screen.
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
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include "glcd.h"

// Note: sprites all take SPRITE_SIZE bytes and there are NUM_SPRITES of
// them, but default I set these to 34 and 8 for a total of 272 bytes. this
// is sufficient for 8 16x16 sprites. These can be changed, but watch the
// total memory used. There is only 1k total on the ATmega168.

// Sprite is an array to hold sprites, each is SPRITE_BYTES long, each sprite
// has [width],[height],[bunch O' bytes for bitblk] user must make sure data
// is ok, code does not check
//
// Logo modified to use with bitblt, and placed in first sprite.
static uint8_t
sprite [SPRITE_SIZE * NUM_SPRITES];

/////////////////////////////////////////////////////////////////////////////
/// Draws the nth sprite at (x,y) using mode.
///
/// @param [in] x is the first x-coordinate to start drawing.
/// @param [in] y is the first y-coordinate to start drawing.
/// @param [in] sprite_id identifies the sprite to draw.
/// @param [in] s_r The drawing mask.
///
void
sprite_draw (uint8_t x, uint8_t y, uint8_t sprite_id, uint8_t mode)
{
    uint8_t *sprite_ptr;                // Pointer to sprite
    uint8_t width;                      // Sprite width
    uint8_t height;                     // Sprite height

    // See what kind of sprite we are dealing with.
    if (sprite_id & 0x80)
    {
        // EEPROM based sprite
        uint8_t *eeprom_addr;           // Address in EEPROM
        uint8_t num_bytes;              // The number of bytes in sprite
        uint8_t offset;                 // Offset to write buffer

        // Ensure that the sprite_id is valid
        sprite_id &= ~0x80;
        if (sprite_id >= EEPROM_SPRITE_NUM)
            return;

        // Get the EEPROM start address of the sprite
        eeprom_addr = (uint8_t *)(((int)(sprite_id) * EEPROM_SPRITE_SIZE) + EEPROM_ADDR_SPRITE_START);

        // Read the sprite into a buffer.
        width = eeprom_read_byte ((const uint8_t *)(eeprom_addr++));
        height = eeprom_read_byte ((const uint8_t *)(eeprom_addr++));

        // Copy the sprite data into the end of the draw_buffer, this should
        // not be used by the bitblt operation.
        num_bytes = width * ((height + 7) >> 3);
        offset = (sizeof(draw_buffer) - 2) - num_bytes;
        sprite_ptr = &draw_buffer [offset];

        // Copy in the data from EEPROM
        *sprite_ptr++ = width;
        *sprite_ptr++ = height;
        while (num_bytes-- > 0)
        {
            *sprite_ptr++ = eeprom_read_byte ((const uint8_t *)(eeprom_addr++));
        }
        // Point at the start of the buffer.
        sprite_ptr = &draw_buffer [offset];
    }
    else
    {
        // Ensure that the sprite_id is valid
        if (sprite_id >= NUM_SPRITES)
            return;

        // Point to the sprite
        sprite_ptr = &sprite [sprite_id * SPRITE_SIZE];
    }

    // If the mode is centre then centre the sprite at the x,y corrdinates.
    if (mode & MODE_SPRITE_CENTER)
    {
        // Centre the sprite at x,y by adjusting the x,y coordinates.
        width = (*sprite_ptr++) >> 1;   // Get the width/2
        if (x > width)
            x -= width;

        height = (*sprite_ptr--) >> 1;  // Get the height/2
        if (y > height)
            y -= height;

        // Remove the sprite flag so bitblt does not see it.
        mode &= ~MODE_SPRITE_CENTER;
    }

    // Display the sprite.
    draw_vbitblt (x, y, mode, sprite_ptr);
}

/////////////////////////////////////////////////////////////////////////////
/// Upload the nth sprite. The command collects the data from the serial.
///
/// Send sprite # first 0..(NUM_SPRITES-1), then sprite data
///
/// The sprite format is (char) width (char) height (SPRITE_SIZE-2 char's)
/// data
///
/// The sprite data is in native bitblt format, ie rows of bytes representing
/// 8-pixel high vertical stripes
void
sprite_upload (uint8_t sprite_id, uint8_t width, uint8_t height)
{
    uint8_t sprite_bytes;

    sprite_bytes = width * ((height + 7) >> 3);

    if (sprite_id & 0x80)
    {
        // This is a EEPROM based sprite.
        uint8_t *eeprom_addr;

        // Ensure that the sprite_id is valid
        sprite_id &= 0x7f;
        if (sprite_id >= EEPROM_SPRITE_NUM)
            sprite_id = EEPROM_SPRITE_NUM - 1;

        eeprom_addr = (uint8_t *)(((int)(sprite_id) * EEPROM_SPRITE_SIZE) + EEPROM_ADDR_SPRITE_START);

        // Write the sprite into a buffer.
        eeprom_write_byte (eeprom_addr++, width);
        eeprom_write_byte (eeprom_addr++, height);

        // Read the rest of the data from serial and write to eeprom.
        if (sprite_bytes > 0)
        {
            do
            {
                eeprom_write_byte (eeprom_addr++, serial_getc ());
            }
            while (--sprite_bytes > 0);
        }
    }
    else
    {
        // This is a RAM based sprite.
        uint8_t *sprite_addr;

        if (sprite_id >= NUM_SPRITES)
            sprite_id = NUM_SPRITES - 1;  // Coerce to valid sprite

        sprite_addr = &sprite[(int)(sprite_id) * SPRITE_SIZE];

        // Write the sprite into the buffer
        *sprite_addr++ = width;
        *sprite_addr++ = height;

        // Read the rest of the data from serial and write to store.
        if (sprite_bytes > 0)
        {
            do
            {
                *sprite_addr++ = serial_getc ();
            }
            while (--sprite_bytes > 0);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Toggle the sprite splash screen state
///
/// Toggles whether or not the splash screen is displayed on startup. The
/// setting is persistent over power cycles.
void
sprite_splash (void)
{
    uint8_t splash;

    // Increment the splash preference, if it reaches 2 then disable the
    // splash.
    splash = prefs_splash;
    if (++splash > 2)
        splash = 0;

    // Write the splash screen value to the preference and EEPROM.
    lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_SPLASH, splash);
}
