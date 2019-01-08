/* -*- c++ -*- **************************************************************
 *
 *  System        : GLCD
 *  Module        : External definitions
 *  Object Name   : $RCSfile: glcd.h,v $
 *  Revision      : $Revision: 1.38 $
 *  Date          : $Date: 2015/07/05 21:08:27 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Sun Apr 5 07:26:00 2015
 *  Last Modified : <150705.1644>
 *
 *  Description   : This file includes all of the exports for all functions.
 *
 *  Notes         :
 *
 *  History       : Removed from 128x64 so can re-use in 160x128
 *
 ****************************************************************************
 *
 *  Copyright (c) 2015 Jon Green.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>		// F_CPU is defined in the makefile

#ifndef __GLCD_H
#define __GLCD_H

/***************************************************************************
 * General values                                                          *
 ***************************************************************************/

#define VERSION_MAJOR     1             /* The version major number */
#define VERSION_MINOR    38             /* The version minor number */

#define SCREEN_MAX_WIDTH  160           /* Maximum width of screen */
extern uint8_t x_dim;                   /* The width of the screen */
extern uint8_t y_dim;                   /* The depth of the screen */

/*
 * Bit shift mask - array of masks 0=0xff; 1=0x7f, 2=0x3f etc. Replaces long
 * shift operations with lookup table
 */
extern const uint8_t bit_shift_maskP[8];

/*
 * Single bit shift mask - array of masks 0=0x01; 1=0x02, 2=0x04 etc. Replaces
 * long shift operations with lookup table
 */
extern const uint8_t bit_shift_single_maskP[8];

/*
 * Single bit shift mask - array of masks 0=0x80; 1=0x40, 2=0x30 etc. Replaces
 * long shift operations with lookup table
 */
extern const uint8_t bit_shift_rev_single_maskP[8];

// Swap bytes macro - swap two bytes
#define swap_bytes(x,y) \
do { \
    (x) = (x)  ^(y); \
    (y) = (x) ^ (y); \
    (x) = (x) ^ (y); \
} while(0)

/***************************************************************************
 * EEPROM locations                                                        *
 ***************************************************************************/

// EEPROM magic number to show we are initialised.
#define EEPROM_MAGIC          0xd5

// EEPROM locations
#define EEPROM_ADDR_MAGIC          0    /* Magic number to handle new install */
#define EEPROM_ADDR_BAUDRATE       1    /* Baud rate */
#define EEPROM_ADDR_BACKLIGHT      2    /* Backlight level */
#define EEPROM_ADDR_SPLASH         3    /* Splash screen enabled */
#define EEPROM_ADDR_REVERSE        4    /* Reverse the screen */
#define EEPROM_ADDR_DEBUG          5    /* Debugging mode */
#define EEPROM_ADDR_CRLF           6    /* Line ending CR+LF */
#define EEPROM_ADDR_XON_POS        7    /* XON position */
#define EEPROM_ADDR_XOFF_POS       8    /* XOFF position */
#define EEPROM_ADDR_SCROLL         9    /* Scroll on/off */
#define EEPROM_ADDR_LARGE_SCREEN  10    /* Large screen. */
#define EEPROM_ADDR_FONT          11    /* The font characterset */
#define EEPROM_ADDR_MAX           12    /* Last entry */

// Sprites are allowed from location 32.
#define EEPROM_ADDR_SPRITE_START  32    /* Start address of EEPROM sprite memory. */
#define EEPROM_SPRITE_SIZE        34    /* Size of sprites */
#define EEPROM_SPRITE_NUM         14    /* Number of sprites. */

// RAM based sprites.
// Needs to be at least 22 to hold sparkfun logo
#define SPRITE_SIZE 34
// Make sure that the memory allocated to sprites (SPRITE_SIZE*NUM_SPRITES)
// does not over fill the device (it only has 1K for all variables)
#define NUM_SPRITES 6
//#define NUM_SPRITES 1

//////////////////////////////////////////////////////////////////////////////
// Constants
// 0 - Version major
// 1 - Version minor
// 2 - EEPROM Sprite size
// 3 - EEPROM Sprite number
// 4 - RAM Sprite size
// 5 - RAM Sprite number
extern const uint8_t consts[6];

/***************************************************************************
 * Operating mode                                                          *
 ***************************************************************************/

// Define the user preferences, we use the EEPROM address as the base.
#define PREFS_ADDR_GRAPHICS           (0+EEPROM_ADDR_MAX)                   /* 11 */
#define PREFS_ADDR_MAX                (PREFS_ADDR_GRAPHICS+1)

extern uint8_t prefs [PREFS_ADDR_MAX];

// Preference values
#define PREFS_DEBUG_OFF      0x00
#define PREFS_DEBUG_BINARY   0x01
#define PREFS_DEBUG_TEXT     0x02
#define PREFS_CRLF_OFF       0x01     /* A LF does not perform a CR */
#define PREFS_CRLF_ON        0x00     /* A LF also performs a CR */
#define PREFS_SCROLL_ON      0x00     /* Scroll is enabled */

// Preferences macros
#define is_invalid_magic()(prefs[EEPROM_ADDR_MAGIC] != EEPROM_MAGIC)
#define is_valid_magic()  (prefs[EEPROM_ADDR_MAGIC] == EEPROM_MAGIC)
#define is_debug_text()   (prefs[EEPROM_ADDR_DEBUG] == OP_DEBUG_TEXT)
#define is_debug_binary() (prefs[EEPROM_ADDR_DEBUG] == OP_DEBUG_BINARY)
#define is_debug()        (prefs[EEPROM_ADDR_DEBUG] == OP_DEBUG_OFF)
#define is_reverse()      (prefs[EEPROM_ADDR_REVERSE] == MODE_REVERSE)
#define is_splash()       (prefs[EEPROM_ADDR_SPLASH] != 0)
#define is_crlf()         (prefs[EEPROM_ADDR_CRLF] == PREFS_CRLF_ON)
#define is_graphics()     (prefs[PREFS_ADDR_GRAPHICS] != 0)
#define is_scroll()       (prefs[EEPROM_ADDR_SCROLL] == PREFS_SCROLL_ON)
#define is_large()        (prefs[EEPROM_ADDR_LARGE_SCREEN] != 0)
#define prefs_splash      prefs[EEPROM_ADDR_SPLASH]
#define prefs_scroll      prefs[EEPROM_ADDR_SCROLL]
#define prefs_crlf        prefs[EEPROM_ADDR_CRLF]
#define prefs_debug       prefs[EEPROM_ADDR_DEBUG]
#define prefs_font        prefs[EEPROM_ADDR_FONT]
#define prefs_backlight   prefs[EEPROM_ADDR_BACKLIGHT]
#define prefs_baudrate    prefs[EEPROM_ADDR_BAUDRATE]
#define prefs_reverse     prefs[EEPROM_ADDR_REVERSE]
#define prefs_graphics    prefs[PREFS_ADDR_GRAPHICS]
#define prefs_xon         prefs[EEPROM_ADDR_XON_POS]
#define prefs_xoff        prefs[EEPROM_ADDR_XOFF_POS]
#define prefs_large       prefs[EEPROM_ADDR_LARGE_SCREEN]

/***************************************************************************
 * Drawing Modes                                                           *
 ***************************************************************************/

// Macros for Drawing
#define MODE_REVERSE          0x00      /* Reverse drawing (~pixel) */
#define MODE_NORMAL           0x01      /* Normal drawing */
#define MODE_OR               0x02      /* OR mode (screen | pixel) */
#define MODE_XOR              0x04      /* XOR mode (screen ^ pixel) */
#define MODE_NAND             0x06      /* AND mode (screen & ~pixel) */

#define MODE_MERGE            0x10      /* A merge is required. */
#define MODE_PROP_FONT        0x20      /* Proportional font */

/* Modifier in effect */
#define MODE_OP_MASK          (MODE_OR|MODE_XOR|MODE_NAND)
#define MODE_MODIFIER         (MODE_MERGE|MODE_OR|MODE_XOR|MODE_NAND)
/* Aliases for normal drawing (pixel) */
#define MODE_COPY             MODE_NORMAL
#define MODE_NORMAL_MASK      MODE_NORMAL

/* Special mode to indicate a fill operation */
#define MODE_FILL             0x08      /* Perform a fill */

/* Special mode to indicate centering sprite around the x,y position. When
 * used with sprite_draw() then the sprite is centred around the x,y position
 * rather than using the x,y as the top corner reference */
#define MODE_SPRITE_CENTER    0x08      /* Only valid with draw sprite. */

/* Mode indicates an invalid command */
#define MODE_INVALID          0xff      /* Invalid mode */

// Definitions for the mode field when drawing lines
#define MODE_CLEAR        MODE_REVERSE  /* Clear - Write zero's */
#define MODE_SET          MODE_NORMAL   /* Set - Write 1s */
#define MODE_SET_MASK     MODE_NORMAL   /* Mask of bit */

// Definitions for the mode field when drawing lines
#define MODE_LINE_SKIP_LAST   0x80      /* Skip the last pixel on line */
#define MODE_LINE_MASK        (MODE_LINE_SKIP_LAST)

/* Validate the coordinates */
#define x_valid(x)   (((x) >= 0) && ((x) < x_dim))
#define y_valid(y)   (((y) & ~(y_dim - 1)) == 0)
#define x_invalid(x) (!x_valid(x))
#define y_invalid(y) (((y) & ~(y_dim - 1)) != 0)

/***************************************************************************
 * Command Handling                                                        *
 ***************************************************************************/

// Define the types of the different functiosn that are used in the command table.
typedef void (*vfunc_t)(void);
typedef void (*vfunc_p_t)(uint8_t *);
typedef void (*vfunc_i_t)(uint8_t);
typedef void (*vfunc_ii_t)(uint8_t, uint8_t);
typedef void (*vfunc_iii_t)(uint8_t, uint8_t, uint8_t);
typedef void (*vfunc_psi_t)(uint8_t *, int8_t, uint8_t);
typedef void (*vfunc_iiip_t)(uint8_t, uint8_t, uint8_t, uint8_t *);
typedef void (*vfunc_iiii_t)(uint8_t, uint8_t, uint8_t, uint8_t);
typedef void (*vfunc_iiiii_t)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
typedef void (*vfunc_iiiiip_t)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t *);
typedef void (*vfunc_iiiiii_t)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
typedef void (*vfunc_iiiiiii_t)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

// Function table
extern const vfunc_t *functabP;

// Define the enumerated device function names.
enum
{
#define DEFFUNC(enum_name, t6963_function, ks0108b_function) enum_name,
#define ENDFUNC(enum_name, t6963_function, ks0108b_function) enum_name
#include "func.def"
#undef DEFFUNC
#undef ENDFUNC
};

// Define the enumerated command function names.
enum
{
#define DEFCMDFUNC(enum_name, function) enum_name,
#define ENDCMDFUNC(enum_name, function) enum_name
#include "func.def"
#undef DEFCMDFUNC
#undef ENDCMDFUNC
};

// Define the enumerated command function names.
enum
{
#define DEFCMD(enum_value, enum_name, args, func) enum_name = enum_value,
#define ENDCMD(enum_value, enum_name, args, func) enum_name = enum_value
#include "func.def"
#undef DEFCMD
#undef ENDCMD
};

/***************************************************************************
 * Serial Handling                                                         *
 ***************************************************************************/

// Define the size of the RX buffer. This should be <= 265 as it is a uint8_t
#define RX_BUFFER_SIZE 256

// Define the high and low water mark positions to send XON (buffer
// sufficiently empty) and XOFF (buffer is nearing full). We need at least
// screen width bytes in the buffer so do not allow to drain below this
// threshold.
//
// For the XOFF position then my OSX Max can or-run by ~70 characters so set
// the threshold to be a little larger than this to catch those bytes without
// over-running.
#define RX_BUFFER_XON     20
#define RX_BUFFER_XOFF    (256 - 90)

// Define the Baud rates
#define BAUD_RATE_4800    1
#define BAUD_RATE_9600    2
#define BAUD_RATE_19200   3
#define BAUD_RATE_38400   4
#define BAUD_RATE_57600   5
#define BAUD_RATE_115200  6

// Default baud rate
#define BAUD_RATE_DEFAULT   BAUD_RATE_115200

// The current baud rate
extern uint8_t serial_baud_rate;

// Test if the baud_rate is valid or invalid
#define baud_rate_valid(x)   (((x) > 0) && ((x) <= BAUD_RATE_115200))
#define baud_rate_invalid(x) (((x) <= 0) || ((x) > BAUD_RATE_115200))

//////////////////////////////////////////////////////////////////////////////
/// Initialise the serial port.
/// Set up the hardware. This may be invoked
/// multiple times in order to change the serial baud rate.
///
extern void
serial_init (void);

//////////////////////////////////////////////////////////////////////////////
/// Reconfigure the serial port. Set up the hardware. This may be invoked
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
extern uint8_t
serial_baudrate (uint8_t baud);

//////////////////////////////////////////////////////////////////////////////
///
/// Flush all input in the serial buffer
///
extern void
serial_flush (void);

//////////////////////////////////////////////////////////////////////////////
///
/// Flush n bytes from the RX_buffer at the head of the queue.
///
/// @param [in] bytes The number of bytes to remove from the queue.
///
/// @return the number of bytes flushed.
///
uint8_t
serial_flushc (uint8_t bytes);

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
extern char
serial_peek (uint16_t offset);

//////////////////////////////////////////////////////////////////////////////
///
/// Read a byte from the RX_buffer from the head of the queue. The method
/// blocks until a character has been received.
///
/// @return The character at the read position.
///
extern char
serial_getc (void);

//////////////////////////////////////////////////////////////////////////////
///
/// Put a character to the serial.
///
/// @param [in] cc The character to write on the TX.
///
extern void
serial_putc (char cc);

/***************************************************************************
 * Backlight Handling                                                      *
 ***************************************************************************/

//////////////////////////////////////////////////////////////////////////////
///
/// Change the backlight level.
///
/// @param [in] level A value between 0 and 100 where 0 is off, 100 is
///                   the brightest level.
///
/// @param [in] cmd   The command that we are executing. If it is a
///                   CMD_SET_BACKLIGHT then save to EEPROM.
///
/// @return The current level.
///
extern uint8_t
backlight_level (uint8_t level, uint8_t cmd);

//////////////////////////////////////////////////////////////////////////////
///
/// Initialise the backlight hardware.
///
extern void
backlight_init (void);

/***************************************************************************
 * Drawing                                                                 *
 * Base level drawing operations                                           *
 ***************************************************************************/

////////////////////////////////////////////////////////////////////////////////////
/// First unitialisation of the device. Set up the display hardware.
///
extern void
ks0108b_init (void);

/////////////////////////////////////////////////////////////////////////////
/// Clearing the display. All we're *really* doing is writing a one or zero
/// to all the memory locations for the display.
///
/// @param [in] mode The mode to clear the screen
///                  0x00 MODE_REVERSE - clears with 1's
///                  0x01 MODE_NORMAL  - clears with 0's
extern void
ks0108b_screen_clear (uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Scroll the display up by 1 line.
///
/// @param [in] buf The buffer to use for reading and writing
/// @param [in] pixels The number of pixels to scroll where -ve is up
/// @param [in] mode The current mode.
///
extern void
ks0108b_vscroll (uint8_t *buf, int8_t pixels, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Reverse the display. We read all of the screen values, invert them and
/// then write them back.
///
/// @param [in] buffer A buffer to use for 8 lines of screen data.
///
extern void
ks0108b_screen_reverse (uint8_t *buf);

/////////////////////////////////////////////////////////////////////////////
/// Set a single pixel. Set assumes that the pixel is being set. This is
/// important as the mode flag set to '1' means reverse which is clear a
/// pixel. The logic outwardly is reverse from the user interface but it is
/// up to the upper levels to resolve this at this level we do not
/// interrogate the reverse flag so there is no ambiguity as to what flag is
/// applied at what level.
///
/// The pixels are in big endian format which is logical and the screen is
/// effectivelly addressed from left to right. That means pixel 0,0 occupies
/// the most significant bit (bit 7) and is set to 0x80.
///
/// @param [in] x_column The column to read (x % 8)
/// @param [in] y The row to re-write.
/// @param [in] mode Merging modification operation to perform.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x86 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x80 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x82 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
extern void
ks0108b_set_pixel (uint8_t x, uint8_t y, uint8_t mode);

//////////////////////////////////////////////////////////////////////////////
/// Vertical bitblt does a bit transfer from data to display memory. If NULL
/// is passed as data, bitblt assumes the data is to come from the serial
/// port, and will take it from there. Bitblt will not return until it gets
/// all the bytes it wants.
///
/// @param [in] x,y is upper left corner of image in pixels. Bitblt counts
///             coordinates in the standard fashion. ie (0,0) is upper left,
///             +x it to the right +y is down width is width in pixels.
///
/// @param [in] mode determines how the bits in the image combine with the
///             bits already present on the display.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x02 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x04 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
///             0x08 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x10 - MODE_FILL
///                    Interpret the data as a mask and fill.
extern void
ks0108b_vbitblt (uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t *data);

/////////////////////////////////////////////////////////////////////////////
/// Draw a horizontal line
///
/// @param [in] x The first x-coordinate.
/// @param [in] y The first y-coordinate.
/// @param [in] x1 The 2nd x-coordinate.
/// @param [in] mode The drawing mode.
///
extern void
ks0108b_hline (uint8_t x, uint8_t y, uint8_t x1, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Draw a vertical line.
///
/// @param [in] x The first x-coordinate.
/// @param [in] y The first y-coordinate.
/// @param [in] y1 The 2nd y-coordinate.
/// @param [in] mode The drawing mode.
///
extern void
ks0108b_vline (uint8_t x, uint8_t y, uint8_t y1, uint8_t mode);

/***************************************************************************
 * Drawing                                                                 *
 * Base level drawing operations                                           *
 ***************************************************************************/

////////////////////////////////////////////////////////////////////////////////////
/// First unitialisation of the device. Set up the display hardware.
///
extern void
t6963_init (void);

/////////////////////////////////////////////////////////////////////////////
/// Clearing the display. All we're *really* doing is writing a one or zero
/// to all the memory locations for the display.
///
/// @param [in] mode The mode to clear the screen
///                  0x01 MODE_NORMAL  - clears with 0's
///                  0x00 MODE_REVERSE - clears with 1's
extern void
t6963_screen_clear (uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Scroll the display up by 1 character line.
///
/// @param [in] buf The buffer to use for reading and writing
/// @param [in] pixels The number of pixels to scroll where -ve is up
/// @param [in] mode The current mode.
///
extern void
t6963_vscroll (uint8_t *buf, int8_t pixels, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Reverse the display. We read all of the screen values, invert them and
/// then write them back.
///
/// @param [in] buffer A buffer to use for 8 lines of screen data.
///
extern void
t6963_screen_reverse (uint8_t *buf);

/////////////////////////////////////////////////////////////////////////////
/// Set a single pixel. Set assumes that the pixel is being set. This is
/// important as the mode flag set to '0' means reverse which is clear a
/// pixel. The logic outwardly is reverse from the user interface but it is
/// up to the upper levels to resolve this at this level we do not
/// interrogate the reverse flag so there is no ambiguity as to what flag is
/// applied at what level.
///
/// The pixels are in big endian format which is logical and the screen is
/// effectivelly addressed from left to right. That means pixel 0,0 occupies
/// the most significant bit (bit 7) and is set to 0x80.
///
/// @param [in] x_column The column to read (x % 8)
/// @param [in] y The row to re-write.
/// @param [in] mode Merging modification operation to perform.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x86 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x80 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x82 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
extern void
t6963_set_pixel (uint8_t x, uint8_t y, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Read a row of from the display.
/// The row is read colum wise where x will start at (x & 0xf8)
///
/// @param [in] x_column The column to read (x % 8)
/// @param [in] y The row to read.
/// @param [in] length The number of columns to read.
/// @param [in] buf Location to read into.
/// @param [in] mode Merging modification operation to perform.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x86 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x80 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x82 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
extern void
t6963_read_row (uint8_t x_column, uint8_t y, uint16_t length, uint8_t *buf, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Write a row to the display.
/// The row is written colum wise where x will start at (x & 0xf8)
///
/// @param [in] x_column The column to write (x % 8)
/// @param [in] y The row to read.
/// @param [in] length The number of columns to read.
/// @param [in] buf Location to write from.
/// @param [in] mode Merging modification operation to perform.
///                  Only performs the reverse.
///
extern void
t6963_write_row (uint8_t x_column, uint8_t y, uint16_t length, uint8_t *buf, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Read and then write a single row of from the display.
/// The row is read colum wise where x is (x & 0xf8)
///
/// This methof is really provided for the single row (length=1) however for
/// consistancy then we provide a multi-length version.
///
/// @param [in] x_column The column to read (x % 8)
/// @param [in] y The row to re-write.
/// @param [in] length The number of columns to re-write.
/// @param [in] buf Location to read into.
/// @param [in] mode Merging modification operation to perform.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x86 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x80 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x82 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
extern void
t6963_rewrite_row (uint8_t x_column, uint8_t y, uint16_t length, uint8_t *buf, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Sets/Draws a single row to the screen.
/// The row is read colum wise where x is (x & 0xf8)
///
/// @param [in] x_column The column to read (x % 8)
/// @param [in] y The row to re-write.
/// @param [in] data The data to write to the screen.
/// @param [in] mask The bit mask of the valid bits of the data.
/// @param [in] mode Merging modification operation to perform.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x86 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x80 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x82 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
extern void
t6963_set_row (uint8_t x_column, uint8_t y, uint8_t data, uint8_t mask, uint8_t mode);

//////////////////////////////////////////////////////////////////////////////
/// Vertical bitblt does a bit transfer from data to display memory. If NULL
/// is passed as data, bitblt assumes the data is to come from the serial
/// port, and will take it from there. Bitblt will not return until it gets
/// all the bytes it wants.
///
/// The width and height are the first 2 bytes of the data passed.
///
/// @param [in] x,y is upper left corner of image in pixels. Bitblt counts
///             coordinates in the standard fashion. ie (0,0) is upper left,
///             +x it to the right +y is down width is width in pixels.
///
/// @param [in] mode determines how the bits in the image combine with the
///             bits already present on the display.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x02 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x04 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
///             0x08 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x10 - MODE_FILL
///                    Interpret the data as a mask and fill.
extern void
t6963_vbitblt (uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t* data);

/////////////////////////////////////////////////////////////////////////////
/// Draw a horizontal line
///
/// @param [in] x The first x-coordinate.
/// @param [in] y The first y-coordinate.
/// @param [in] x1 The 2nd x-coordinate.
/// @param [in] mode The drawing mode.
///
extern void
t6963_hline (uint8_t x, uint8_t y, uint8_t x1, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Draw a vertical line. The fastest way to draw this is to set pixels.
///
/// @param [in] x The first x-coordinate.
/// @param [in] y The first y-coordinate.
/// @param [in] y1 The 2nd y-coordinate.
/// @param [in] mode The drawing mode.
///
extern void
t6963_vline (uint8_t x, uint8_t y, uint8_t y1, uint8_t mode);

/***************************************************************************
 * LCD abstraction                                                         *
 * Renames the commands from the device - should we want to change devices *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
/// Demonstration
///
/// Draw the splash screen for the demo. The splash screen preference
/// determines what is drawn at start up.
///
/// @param [in] cmd Parameter identifies if called from the command line
///                 and is passed CMD_DEMO
extern void
lcd_demo (uint8_t cmd);

//////////////////////////////////////////////////////////////////////////////
/// Perform a factory reset on EEPROM.
///
extern void
lcd_factory_reset (void);

//////////////////////////////////////////////////////////////////////////////
/// Set the value of a EEPROM location. The value is stored in EEPROM
///
/// @param [in] check A check byte to ensure this is a set command
/// @param [in] id The identiy of the EEPROM location to change.
/// @param [in] value The value to assign to the EEPROM location.
///
extern void
lcd_set (uint8_t check, uint8_t id, uint8_t value);

// The value used to confirm that this is a set value. This is a bit
// primative the aim is to prevent a miss command from changing the value
// inadvertently.
#define LCD_SET_CHECKBYTE   0xc5

//////////////////////////////////////////////////////////////////////////////
/// Query the internal state of the system. Return the settings to the caller
/// via the serial. The format is defined as follows:
///
/// @param [in] info The identity of the information to retrieve.
extern void
lcd_query (uint8_t info);

//////////////////////////////////////////////////////////////////////////////
/// Clear the screen
///
extern void
lcd_screen_clear (void);

// Set a pixel on the screen
#define lcd_set_pixel(x, y, mode)  \
((vfunc_iii_t)(pgm_read_word(&functabP[(uint8_t)F_DRV_SET_PIXEL])))(x, y, mode)

// Vertical bitblt
#define lcd_vbitblt(x, y, width, height, mode, data)  \
((vfunc_iiiiip_t)(pgm_read_word(&functabP[(uint8_t)F_DRV_VBITBLT])))(x, y, width, height, mode, data)

// Vertical scroll
#define lcd_vscroll(buf, pixels, mode) \
((vfunc_psi_t)(pgm_read_word(&functabP[(uint8_t)F_DRV_VSCROLL])))(buf, pixels, mode)

//////////////////////////////////////////////////////////////////////////////
/// Hard reset the screen.
///
extern void
lcd_reset (void);

//////////////////////////////////////////////////////////////////////////////
/// Clear the screen
///
extern void
lcd_screen_clear (void);

//////////////////////////////////////////////////////////////////////////////
/// Reverse the screen
///
/// @param [in] mode  The mode to assign to the reverse preference.
/// @param [in] cmd   The command that we are executing. If it is a
///                   CMD_REVERSE_MODE then save to EEPROM.
extern void
lcd_screen_reverse (uint8_t mode, uint8_t cmd);

/////////////////////////////////////////////////////////////////////////////
/// Set the graphics mode
///
/// @param [in] cmd The command that invoked the call.
///
extern void
graphics_mode (uint8_t cmd);

/***************************************************************************
 * Graphics commands                                                       *
 ***************************************************************************/

// Buffer used for line blitting; this is 8 pairs of coordinates. The line
// buffer is also used for expanding the fonts redy for rendering.
#define LINE_BUFFER_MAX   16

uint8_t line_buffer [LINE_BUFFER_MAX];
#define font_buffer line_buffer

// Buffer used for bitblt and other draw operations.
extern uint8_t draw_buffer [SCREEN_MAX_WIDTH];

// The current draw mode.
extern uint8_t drawing_mode;

//////////////////////////////////////////////////////////////////////////
/// Change the current draw mode
///
/// @param [in] mode The new drawing mode.
///
extern void
draw_mode (uint8_t mode);

//////////////////////////////////////////////////////////////////////////////
/// Draws a box. The box is described by a diagonal line from x, y1 to x2, y2.
///
/// @param [in] x1 The upper left x-coordinate.
/// @param [in] y1 The upper left y-coordingate.
/// @param [in] x2 The lower right x-coordinate.
/// @param [in] y2 The lower right y-coordinate
/// @param [in] mode The drawing mode of the line.
///
extern void
draw_box (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t s_r);

//////////////////////////////////////////////////////////////////////////////
/// Draw a circle.
///
/// @param [in] x The x-coordinate of the centre of the circle.
/// @param [in] y The x-coordinate of the centre of the circle.
/// @param [in] r The radius of the circle.
/// @param [in] s_r The drawing mode of the line.
///
extern void
draw_circle (uint8_t x, uint8_t y, uint8_t r, uint8_t s_r);

//////////////////////////////////////////////////////////////////////////////
/// Draw a horizontal line to the screen.
///
/// @param [in] x0 The first x-coordinate.
/// @param [in] y0 The first y-coordinate.
/// @param [in] x1 The second y-coordinate.
/// @param [in] s_r The drawing mode of the line.
///
#define draw_hline(x0,y0,x1,s_r)\
((vfunc_iiii_t)(pgm_read_word(&functabP [(uint8_t)F_DRV_HLINE])))(x0,y0,x1,s_r)

//////////////////////////////////////////////////////////////////////////////
/// Draws a line.
///
/// @param [in] x1 The start x-coordinate.
/// @param [in] y1 The start y-coordingate.
/// @param [in] x2 The end x-coordinate.
/// @param [in] y2 The end y-coordinate
/// @param [in] mode The drawing mode of the line.
///
extern void
draw_line (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t s_r);

//////////////////////////////////////////////////////////////////////////////
/// Draws a single pixel.
///
/// @param [in] x1 The x-coordinate.
/// @param [in] y1 The y-coordingate.
/// @param [in] mode The drawing mode of the pixel.
///
extern void
draw_pixel (uint8_t x, uint8_t y, uint8_t s_r);

//////////////////////////////////////////////////////////////////////////////
/// Draws a rounded corner box. The box is described by a diagonal line from
/// x, y1 to x2, y2
///
/// @param [in] x1 The upper left x-coordinate.
/// @param [in] y1 The upper left y-coordingate.
/// @param [in] x2 The lower right x-coordinate.
/// @param [in] y2 The lower right y-coordinate
/// @param [in] radius The radius of the corner.
/// @param [in] s_r The mode to draw the line
///
extern void
draw_rbox (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t radius, uint8_t s_r);

//////////////////////////////////////////////////////////////////////////////
/// Draw a vertical line to the screen.
///
/// @param [in] x0 The first x-coordinate.
/// @param [in] y0 The first y-coordinate.
/// @param [in] y1 The second y-coordinate.
/// @param [in] s_r The drawing mode of the line.
///
#define draw_vline(x0,y0,y1,s_r) \
((vfunc_iiii_t)(pgm_read_word(&functabP [(uint8_t)F_DRV_VLINE])))(x0,y0,y1,s_r)

//////////////////////////////////////////////////////////////////////////////
/// Draw a series of connected lines.
///
/// @param [in] x The x-coordinate of the first line.
/// @param [in] y The y-coordinate of the first line.
/// @param [in] s_r The drawing mode of the line.
/// @param [in] data A pointer to a list of (x,y) coordinate pairs. The
///                  end of the list is terminated with y|0x80. A value of
///                  NULL then the coordinate list is read from the serial
///                  input.
///
extern void
draw_lines (uint8_t x, uint8_t y, uint8_t s_r, uint8_t *data);

//////////////////////////////////////////////////////////////////////////////
/// Draw a polygon.
///
/// @param [in] x The x-coordinate of the first line.
/// @param [in] y The y-coordinate of the first line.
/// @param [in] s_r The drawing mode of the line.
/// @param [in] data A pointer to a list of (x,y) coordinate pairs. The
///                  end of the list is terminated with y|0x80. A value of
///                  NULL then the coordinate list is read from the serial
///                  input.
///
extern void
draw_polygon (uint8_t x, uint8_t y, uint8_t s_r, uint8_t *data);

//////////////////////////////////////////////////////////////////////////////
/// Erase a rectangular block of the screen to the background colour.
///
/// @param [in] x1 The top left x-coordinate.
/// @param [in] y1 The top left y-coordinate.
/// @param [in] x2 The bottom right x-coordinate.
/// @param [in] y2 The bottom right y-coordinate.
extern void
erase_box (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

//////////////////////////////////////////////////////////////////////////////
/// Fills a rectangular block of the screen.
///
/// @param [in] x1 The top left x-coordinate.
/// @param [in] y1 The top left y-coordinate.
/// @param [in] x2 The bottom right x-coordinate.
/// @param [in] y2 The bottom right y-coordinate.
/// @param [in] mode The drawing mode.
extern void
fill_box (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode);

//////////////////////////////////////////////////////////////////////////////
/// Draws a box and fills it with a pattern. The box is described by a
/// diagonal line from x, y1 to x2, y2
///
/// @param [in] x1 The upper left x-coordinate.
/// @param [in] y1 The upper left y-coordingate.
/// @param [in] x2 The lower right x-coordinate.
/// @param [in] y2 The lower right y-coordinate
/// @param [in] pattern The vertical pattern to use as fill pattern.
///
extern void
fill_vbox (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t data);

//////////////////////////////////////////////////////////////////////////////
/// Vertical bitblt does a bit transfer from data to display memory. If NULL
/// is passed as data, bitblt assumes the data is to come from the serial
/// port, and will take it from there. Bitblt will not return until it gets
/// all the bytes it wants.
///
/// @param [in] x,y is upper left corner of image in pixels. Bitblt counts
///             coordinates in the standard fashion. ie (0,0) is upper left,
///             +x it to the right +y is down width is width in pixels.
///
/// @param [in] mode determines how the bits in the image combine with the
///             bits already present on the display.
///
///             0x00 - MODE_REVERSE
///                    No merge required, reverse the data.
///                    buffer[x] = ~read_data
///                    Reverse is applied irrespective of the
///                    combinational modes (OR, XOR, NAND).
///                    So the data is returned un-reversed.
///
///             0x01 - MODE_COPY
///                    No merge required.
///                    buffer[x] = read_data
///
///             0x02 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x04 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
///             0x08 - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
///             0x10 - MODE_FILL
///                    Interpret the data as a mask and fill.
extern void
draw_vbitblt (uint8_t x, uint8_t y, uint8_t mode, uint8_t *data);

/***************************************************************************
 * Font Handling                                                           *
 ***************************************************************************/

// Define the characters we process
#define CHAR_BACKSPACE             0x08
#define CHAR_LF                    0x0a
#define CHAR_CR                    0x0d
#define CHAR_XON                   0x11
#define CHAR_XOFF                  0x13
#define CHAR_COMMAND               0x7c

// Define the font justification
#define FONT_ALIGN_CENTER          0x00
#define FONT_ALIGN_RIGHT           0x01

//////////////////////////////////////////////////////////////////////////////
/// Initialise the fonts.
/// We install the fonts that we are using.
///
/// @param [in] fptr Pointer to the font file in flash memory.
extern void
font_init (const char *fptr);

//////////////////////////////////////////////////////////////////////////////
/// Changes the current font
///
/// @param [in] font The charaterset to use.
///                  0 = Default font 5x8
///                  1 = Small font 3x6
/// @param [in] cmd  The command. 0x08=store, 0x48
///
extern void
font_set (uint8_t font, uint8_t cmd);

//////////////////////////////////////////////////////////////////////////////
/// Draw a character on the screen. The x_offset, y_offset define the top/left
/// of the corner of the character and are automatically updated for the next
/// character.
///
/// @param [in] txt The character to draw.
///                 If the character is not present we present a square box.
///
extern void
font_draw (char txt);

/////////////////////////////////////////////////////////////////////////////
/// Draw a string on screen
///
/// @param [in] stringP Pointer to a PROGMEM string.
extern void
font_draw_stringP (const char *stringP);

/////////////////////////////////////////////////////////////////////////////
/// Draw a number string on screen
///
/// @param [in] value The value to render.
extern void
font_draw_number (int16_t value);

/////////////////////////////////////////////////////////////////////////////
/// Draw a on/off string on screen
///
/// @param [in] value When '0' draws 'off' else 'on'
extern void
font_draw_on_off (uint8_t value);

//////////////////////////////////////////////////////////////////////////////
/// Deletes a full character space previous to the current location
/// (backspace).
///
extern void
font_backspace (void);

//////////////////////////////////////////////////////////////////////////////
/// Performs a CR operation. This moves the cursor back to the start position.
/// If the CRLF option is enabled then a new line will be performed.
///
extern void
font_cr (void);

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
extern void
font_layout (uint8_t x, uint8_t y, uint8_t justification);

//////////////////////////////////////////////////////////////////////////////
/// Modify the x and y position
///
/// @param [in] arg1 The x or y position with a single argument command.
/// @param [in] arg2 The y position with multiple arguments.
/// @param [in] cmd  The command that involked the position change.
///
extern void
font_position (uint8_t arg1, uint8_t arg2, uint8_t cmd);

//////////////////////////////////////////////////////////////////////////////
/// Performs a LF operation.
///
extern void
font_lf (void);

//////////////////////////////////////////////////////////////////////////////
/// Changes the drawing mode.
///
/// @param [in] mode The drawing mode for characters.
extern void
font_mode (uint8_t mode);

/***************************************************************************
 * Sprite Handling                                                         *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
/// Draws the nth sprite at (x,y) using mode.
///
/// @param [in] x is the first x-coordinate to start drawing.
/// @param [in] y is the first y-coordinate to start drawing.
/// @param [in] sprite_id identifies the sprite to draw.
/// @param [in] s_r The drawing mask.
///
extern void
sprite_draw (uint8_t x, uint8_t y, uint8_t sprite_id, uint8_t mode);

/////////////////////////////////////////////////////////////////////////////
/// Upload the nth sprite. The command collects the data from the serial.
///
/// @param [in] sprite_id The identity of the sprite.
/// @param [in] width The width of the sprite in bits
/// @param [in] height The height of the sprite in bits.
///
///
/// The sprite data is in native bitblt format, i.e. rows of bytes
/// representing 8-pixel high vertical stripes which is collected from
/// serial.
extern void
sprite_upload (uint8_t sprite_id, uint8_t width, uint8_t height);

/////////////////////////////////////////////////////////////////////////////
/// Toggle the sprite splash screen state
///
extern void
sprite_splash (void);

#endif /* __GLCD_H */
