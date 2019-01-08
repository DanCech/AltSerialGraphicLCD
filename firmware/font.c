/* -*- c++ -*- ***************************************************************
 *
 *  System      : Serial GLCD
 *  Module      : Font Handling
 *  Object Name : $RCSfile: font.c,v $
 *  Revision    : $Revision: 1.15 $
 *  Date        : $Date: 2015/07/05 21:09:09 $
 *  Author      : $Author: jon $
 *  Created By  : Jon Green
 *  Created     : Sun Apr 5 08:43:33 2015 Last Modified : <150705.1932>
 *
 *  Description : Handles all of the font related
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
#include <avr/io.h>
#include "glcd.h"

// This include file stores all the revelant font data. You can make new ones
// with the bmp2header_font utility in the utilities folder
/* #include "font.h"                    -- The original 8x5 font. The 4's and 7's not so good. */
#include "font_alt_5x8.h"               /* A updated 5x8 font which is slightly cleaner */
#include "font_tom_thumb_3x6.h"         /* A small 3x6 font */
// Noted that there is insufficient space to include the 16x10 font

// The number of bytes in the font file header.
#define FONT_FILE_HEADER_LEN      5

// The font drawing mode
static uint8_t font_draw_mode;

// Upper left corner of character, x-coord
static uint8_t x_pos;
// Upper left corner of character, y-coord
static uint8_t y_pos;

// How font interacts with background. (bitblt option)
// 7 = overwrite and is default (no need to erase the background first)
//static uint8_t font_mode;
// Number of bytes in a character (5 for default font)
static uint8_t font_bytes;
// Width of a character in pixels (5 for default font)
static uint8_t font_w;
// Width of a character + space of font in pixels (6 for default font)
static uint8_t font_ws;
// Height of a character in pixels (8 for default font)
static uint8_t font_h;
// Horizontal space to leave between characters
static uint8_t font_space;
// First valid character
static uint8_t font_first_char;
// Last valid character
static uint8_t font_last_char;
// The current font we are using
static const char *font_ptr;
// The last position we are using.
static uint8_t font_xpos;
// The last position we are using.
static uint8_t font_ypos;
// The start position we are using.
static uint8_t font_start_xpos;

//////////////////////////////////////////////////////////////////////////////
/// Initialise the fonts.
/// We install the fonts that we are using.
///
/// @param [in] fptr Pointer to the font file in flash memory.
void
font_init (const char *fptr)
{
    uint8_t temp_w;
    
    // Use the default font if none specified
    if (fptr == NULL)
        fptr = font_alt_5x8;

    // Set up the default font
    font_ptr = fptr;
    temp_w = pgm_read_byte(fptr++);
    font_w = temp_w;
    font_h = pgm_read_byte(fptr++);
    font_bytes = (font_h + 7) / 8;      // 8 pixels/byte with partial rows
    font_space = pgm_read_byte(fptr++);
    font_ws = temp_w + font_space;
    font_first_char = pgm_read_byte(fptr++);
    font_last_char = pgm_read_byte(fptr++);
    font_bytes *= temp_w;               // Need font_w stacks of rows

    // Reset the position
    font_xpos = ~0;
    font_ypos = ~0;
}

//////////////////////////////////////////////////////////////////////////////
/// Changes the current font
///
/// @param [in] font The charaterset to use.
///                  0 = Default font 5x8
///                  1 = Small font 3x6
/// @param [in] cmd  The command. 0x08=store, 0x48
void
font_set (uint8_t font, uint8_t cmd)
{
    const char *fptr;
    
    // Determine the font
    font &= 1;
    if (font == 0)
        fptr = font_alt_5x8;
    else
        fptr = tom_thumb_3x6;
    
    // Initialise the font.
    font_init(fptr);
    
    // Save the setting if required.
    if (cmd == CMD_FONT_SET)
        lcd_set (LCD_SET_CHECKBYTE, EEPROM_ADDR_FONT, font);
}

//////////////////////////////////////////////////////////////////////////////
/// Changes the drawing mode.
///
void
font_mode (uint8_t mode)
{
    font_draw_mode = mode;
}

/////////////////////////////////////////////////////////////////////////////
/// Draw a string on screen
/// 
/// @param [in] stringP Pointer to a PROGMEM string. 
void
font_draw_stringP (const char *stringP)
{
    char cc;
    
    while ((cc = pgm_read_byte (stringP++)) != '\0')
        font_draw (cc);
}
    
/////////////////////////////////////////////////////////////////////////////
/// Draw a on/off string on screen
/// 
/// @param [in] value When '0' draws 'off' else 'on'
void
font_draw_on_off (uint8_t value)
{
    static const char on [] PROGMEM = "on";
    static const char off [] PROGMEM = "off";    
    
    font_draw_stringP ((value == 0) ? off : on);
}
    
/////////////////////////////////////////////////////////////////////////////
/// Draw a string on screen
/// 
/// @param [in] value The value to render.
void
font_draw_number (int16_t value)
{
    char buf [6];                       // Draw buffer
    char *p = &buf[5];                  // Local pointer
    
    // Nil terminate the string
    *p = '\0';

    // Sort out the sign
    if (value < 0)
    {
        // Draw the sign and make positive
        font_draw ('-');
        value = -value;
    }

    // Convert to a ASCII decimal string representation.
    do
    {
        *--p = '0' + (uint8_t)(value % 10);
        value /= 10;
    }
    while (value > 0);
    
    // Display the output
    do
    {
        font_draw (*p++);
    }
    while (*p != '\0');
}

/////////////////////////////////////////////////////////////////////////////
/// Layout a string in the x-axis for labelling. This is used for
/// proporitional fonts and computes the length of a 0xff terminated string
/// and positions the string either centre or right justified. The call sets
/// the position to the (x,y) corrected for the justification of the text.
/// 
/// The command only works for serial data and leaves the chatacters to be
/// rendered in the input buffer. The characters are looked ahead and the
/// length is computed. When the command finishes then the characters are
/// rendered. Graphics mode is automatically turned off.
/// 
/// @param [in] x The x reference position
/// @param [in] y The y reference position
/// @param [in] justification The rendering position 0=centre, 1=right 
void
font_layout (uint8_t x, uint8_t y, uint8_t justification)
{
    uint8_t length = 0;                 // Length of string in pixels
    uint8_t count = 0;                  // Length of string in chars
    uint8_t txt;                        // The current character
    
    // Iterate until the end of the string. Look ahead in the input buffer
    // and collect characters. 
    while ((txt = serial_peek(count)) != 0xff)
    {
        uint16_t ii;
        uint16_t offset;
        
        // Make sure the text is in bounds otherwise print a question mark.
        if ((txt < font_first_char) || (txt > font_last_char))
            txt = font_first_char;      // Correct out of bounds
        
        // txt-32 is the ascii offset to 'space', font_bytes is the # of
        // bytes/character, and 5 for font width,height,space, first_char and
        // last_char which are stores at the beginning of the array
        offset = (txt - font_first_char) * font_bytes + FONT_FILE_HEADER_LEN;
        
        // Compute the width of the character.
        for (ii = offset; ii < offset + font_bytes; ii++)
        {
            uint8_t cc = pgm_read_byte (&font_ptr[ii]);
            
            // If the font is proportional then remove any blank verticals
            // only if the character is not a space and the font size is less
            // than or equal to 8 pixels. 
            if ((cc != 0) || (txt == 0x20) || ((font_draw_mode & MODE_PROP_FONT) == 0) || (font_h > 8))
            {
                length++;
            }
        }
        
        // Account for intercharacter space.
        if (count > 0)
            length += font_space;
        
        // Another character processed.
        count++;
    }
    
    // Compute the position to render the string. Note we do not correct any
    // user mistakes this results in a rendering anomaly. 
    if (justification == FONT_ALIGN_CENTER)
        length = (length + 1) >> 1;     // Divide by 2
    x -= length;                        // Position x
    
    // Change the position 
    font_position (x, y, 0);
    
    // Explicitly turn off graphics mode.
    prefs_graphics = 0;
}
    
//////////////////////////////////////////////////////////////////////////////
/// Draw a character on the screen. The x_pos, y_pos define the top/left
/// of the corner of the character and are automatically updated for the next
/// character.
///
/// @param [in] txt The character to draw.
///                 If the character is not present we present a square box.
///
void
font_draw (char txt)
{
    uint16_t offset;                    // Offset into the text array
    uint16_t ii;                        // Loop counter.
    uint8_t jj;                         // Buffer position.
    uint8_t mode;                       // The rendering mode.
    uint8_t bltpos;                     // The bit blit position
    uint8_t actual_width;               // The actual width 
    
    // Quit quickly on character 0xff which is a terminal character for the
    // text layout. 
    if (~txt == 0)
        return;
    
    // y_pos counts pixels from the top of the screen
    // x_pos counts pixels from the left side of the screen
    
    // A delayed LF, if the character position is off screen then wrap the
    // position to the top of the screen or scroll the screen up. 
    if (y_pos > (y_dim - font_h))
    {
        // See if we need to scroll
        if (is_scroll())
        {
            // Scroll the screen up by 1 line.
            lcd_vscroll (draw_buffer, -8, prefs_reverse);
            
            // Decrement the font position.
            y_pos -= font_h;
        }
        else
        {
            // Make sure that the line restarted at the top will overlap the
            // old one 
            y_pos = y_pos % font_h;
        }
        
        // Invalidate the previous position
        font_xpos = ~0;
        font_ypos = ~0;
    }

    // Compute the current mode based on the reverse preference.
    mode = (~font_draw_mode ^ prefs_reverse) & MODE_NORMAL_MASK;
    mode |= font_draw_mode & ~MODE_NORMAL_MASK;

    // Make sure the text is in bounds otherwise print a question mark.
    if ((txt < font_first_char) || (txt > font_last_char))
        txt = font_first_char;          // Correct out of bounds

    // Get data for character. Put in upper half of buffer, lower half is
    // used by bitblt
    bltpos = SCREEN_MAX_WIDTH - font_bytes;
    jj = bltpos;

    // txt-32 is the ascii offset to 'space', font_bytes is the # of
    // bytes/character, and 5 for font width,height,space, first_char and
    // last_char which are stores at the beginning of the array
    offset = (txt - font_first_char) * font_bytes + FONT_FILE_HEADER_LEN;
    
    // loop for one character worth of bytes
    actual_width = 0;
    for (ii = offset; ii < offset + font_bytes; ii++)
    {
        uint8_t cc = pgm_read_byte (&font_ptr[ii]);
        
        // If the font is proportional then remove any blank verticals only
        // if the character is not a space and the font size is less than or
        // equal to 8 pixels. 
        if ((cc != 0) || (txt == 0x20) || ((font_draw_mode & MODE_PROP_FONT) == 0) || (font_h > 8))
        {
            font_buffer[jj++] = cc;
            actual_width++;
        }
    }

    // See if we need to insert some space between the last character.
    if ((font_space > 0) && (x_pos > font_space) &&
        (font_xpos == x_pos) && (font_ypos == y_pos))
    {
        uint8_t data = 0x00;
        lcd_vbitblt (x_pos - font_space, y_pos,
                     font_space, font_h, mode | MODE_FILL, &data);
    }

    // Render the character to the screen.
    lcd_vbitblt (x_pos, y_pos, actual_width, font_h, mode, &font_buffer[bltpos]);

    // Advance the current cursor position.
    x_pos += actual_width + font_space;

    // Check x offset and do necessary wrapping
    if ((x_pos + font_w) > x_dim - 1)
    {
        // Make sure text on the next line will line up with the previous
        // line and perform a line feed operation.
        x_pos = font_start_xpos; /*%= font_ws*/;
        font_lf ();
    }
    else
    {
        // Save the last position.
        font_xpos = x_pos;
        font_ypos = y_pos;
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Deletes a full character space previous to the current location
/// (backspace).
///
void
font_backspace (void)
{
    if (x_pos < font_ws)
    {
        // If previous char wouldn't have fit
        x_pos = (x_dim - font_ws - ((x_dim - x_pos) % font_ws));

        if (y_pos < font_h)
        {
            // If we run off the top of the screen
            y_pos = (y_dim - font_h - ((y_dim - y_pos) % font_h));
        }
        else
            y_pos -= font_h;
    }
    else
    {

        // Back x_pos up by the font width + 1 pixel space between characters
        x_pos -= font_ws;
    }

    // Erase the block
    fill_box (x_pos, y_pos,
              x_pos + font_w,
              y_pos + font_h - 1,
              ~prefs_reverse & MODE_NORMAL_MASK);
}

//////////////////////////////////////////////////////////////////////////////
/// Performs a CR operation. This moves the cursor back to the start position.
/// If the CRLF option is enabled then a new line will be performed.
///
void
font_cr (void)
{
    // Perform a CR i.e. move the cursor back to the start of the line. 
    x_pos = font_start_xpos; /* %= font_ws */

    // Invalidate the previous position
    font_xpos = ~0;
    font_ypos = ~0;
}

//////////////////////////////////////////////////////////////////////////////
/// Performs a LF operation.
///
void
font_lf (void)
{
    // Handle a new line additional scroll, determine if we need to scroll.
    if (y_pos >= y_dim) 
    {
        if (is_scroll())
        {
            // There is a scroll pending when we render the next character. To
            // provide an interactive feedback then perform a scroll now. Scroll
            // the screen up by 1 line. 
            lcd_vscroll (draw_buffer, -8, prefs_reverse);
        }
        else
        {
            // Make sure that the line restarted at the top will overlap the
            // old one 
            y_pos %= font_h;
        }
    }
    else
    {
        // Advance the line offset, note that this is not enacted until the next
        // character is draw so that we keep as much information on the screen as
        // possible in the case that we scroll.
        y_pos += font_h;
    }
    
    // If there is LF preference then enact.
    if (is_crlf())
        font_cr ();
    
    // Invalidate the previous position
    font_xpos = ~0;
    font_ypos = ~0;
}

//////////////////////////////////////////////////////////////////////////////
/// Modify the x and y position
///
void
font_position (uint8_t arg1, uint8_t arg2, uint8_t cmd)
{
    if (cmd == CMD_SET_Y_OFFSET)
    {
        // Y offset change only
        y_pos = arg1;
    }
    else
    {
        // Set the x position
        x_pos = arg1;
        font_start_xpos = arg1;
        
        // If this is not an x offset only then set the y position with the
        // 2nd argument. 
        if (cmd != CMD_SET_X_OFFSET)
            y_pos = arg2;
    }
    
    // Invalidate the previous position
    font_xpos = ~0;
    font_ypos = ~0;
}
