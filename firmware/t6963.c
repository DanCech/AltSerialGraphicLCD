/* -*- c++ -*- **************************************************************
 *
 *  System        : SerialGLCD
 *  Module        : T6963 driver
 *  Object Name   : $RCSfile: t6963.c,v $
 *  Revision      : $Revision: 1.23 $
 *  Date          : $Date: 2015/06/08 20:40:33 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Thu Apr 16 21:24:20 2015
 *  Last Modified : <150608.2140>
 *
 *  Description   : Toshiba T6963 LCD screen driver.
 *
 *  Notes         : Ground up implementation of the T6963 driver with some
 *                  ispiration from:
 *
 *                  Code by Mike Hord, SparkFun Electronics.
 *
 *                  Code by Jennifer Holt
 *
 *  History       :
 *
 ****************************************************************************/
/****************************************************************************
 * t6963.c
 *
 * t6963 controller driver file. Handles the hardware-level interfacing for
 * the t6963 LCD controller.
 *
 * 02 May 2013 - Mike Hord, SparkFun Electronics
 *
 * This code is released under the Creative Commons Attribution Share-Alike
 * 3.0 license. You are free to reuse, remix, or redistribute it as you see
 * fit, so long as you provide attribution to SparkFun Electronics.
 ***************************************************************************/
/****************************************************************************
 * Copyright (c) 2010 Jennifer Holt
 * Copyright (c) 2015 Jon Green
 *
 * This is a ground up implementation of the Sparkfun T6963 driver. which has
 * been influenced by the original Sparkfun implementation and the Jennifier
 * Holt version of the KS0108b version.
 *
 * Compared with the Samsung KS0108b this chipset is not so fussy on the
 * control line settings and is extreamly fast. The bitblt function was
 * derrived from the original implementation of Jennifer Holt and has been
 * completely re-designed to deal with the pixels organised as rows rather
 * than columns.
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
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include "glcd.h"

/* Define the number of columns */
#define SCREEN_WIDTH   160               /* Screen width */
#define SCREEN_HEIGHT  128               /* Screen height */
#define SCREEN_COLUMNS (SCREEN_WIDTH/8)  /* Screen columns */

// Pins for the t6963 (160x128) display
#define WR      0       /* PC0 */
#define RD      1       /* PC1 */
#define CE      2       /* PC2 */
#define CD      3       /* PC3 */
#define HALT    4       /* PC4 */
#define RST     5       /* PC5 */

/* Define the status bits */
#define STA0  0x01
#define STA1  0x02
#define STA01 (STA0|STA1)
#define STA2  0x04
#define STA3  0x08
#define STA5  0x20
#define STA6  0x40
#define STA7  0x80

/* Macro to define the command and the next status check value */
#define DEFCMD(sta,id)             (((sta) << 8) | (id))

/* Registers setting */
#define CMD_SET_CURSOR_POINTER     DEFCMD(STA01, 0x21)  /* Set cursor pointer */
#define CMD_SET_OFFSET_REGISTER    DEFCMD(STA01, 0x22)  /* Set offset register */
#define CMD_SET_ADDR_POINTER       DEFCMD(STA01, 0x24)  /* Set address pointer */
/* Set Control Word */
#define CMD_SET_TEXT_HOME_ADDR     DEFCMD(STA01, 0x40)  /* Set text home address */
#define CMD_SET_TEXT_AREA          DEFCMD(STA01, 0x41)  /* Set text area */
#define CMD_SET_GRAPIC_HOME_ADDR   DEFCMD(STA01, 0x42)  /* Set graphic home address */
#define CMD_SET_GRAPHIC_AREA       DEFCMD(STA01, 0x43)  /* Set graphic area */
/* Mode set */
#define CMD_MODE_SET               DEFCMD(STA01, 0x80)  /* Mode Set */
#define CMD_MODE_OR                DEFCMD(STA01, 0x80)  /* OR mode */
#define CMD_MODE_EXOR              DEFCMD(STA01, 0x81)  /* EXOR mode */
#define CMD_MODE_AND               DEFCMD(STA01, 0x83)  /* AND mode */
#define CMD_TEXT_ATTRIBUTE_MODE    DEFCMD(STA01, 0x84)  /* Text Attribute mode */
#define CMD_INTERNAL_CG_ROM_MODE   DEFCMD(STA01, 0x80)  /* Internal CG ROM mode */
#define CMD_EXTERNAL_CG_ROM_MODE   DEFCMD(STA01, 0x88)  /* External CG ROM mode */
/* Display mode */
#define CMD_DISPLAY_OFF            DEFCMD(STA01, 0x90)  /* Display off */
#define CMD_DISPLAY_CURSOR_BLINK   DEFCMD(STA01, 0x92)  /* Cursor on, blink on/off */
#define CMD_DISPLAY_TEXT           DEFCMD(STA01, 0x94)  /* Text on, graphic off */
#define CMD_DISPLAY_GRAPHIC        DEFCMD(STA01, 0x98)  /* Text off, graphic on */
#define CMD_TEXT_GRAPHIC           DEFCMD(STA01, 0x9c)  /* Text on, graphic on */
/* Cursor pattern */
#define CMD_CURSOR_PATTERN_SELECT  DEFCMD(STA01, 0xa0)  /* Cursor pattern selection */
/* Data auto read/write */
#define CMD_DATA_AUTO_WRITE        DEFCMD(STA3,  0xb0)  /* Set data auto write */
#define CMD_DATA_AUTO_READ         DEFCMD(STA2,  0xb1)  /* Set data auto read */
#define CMD_DATA_AUTO_RESET        DEFCMD(STA01, 0xb2)  /* Auto reset */
/* Data read/write */
#define CMD_DATA_WRITE_INC         DEFCMD(STA01, 0xc0)  /* Data write and increment ADP */
#define CMD_DATA_READ_INC          DEFCMD(STA01, 0xc1)  /* Data read abd decrement ADP */
#define CMD_DATA_WRITE_DEC         DEFCMD(STA01, 0xc2)  /* Data write and decrement ADP */
#define CMD_DATA_READ_DEC          DEFCMD(STA01, 0xc3)  /* Data read and decrement ADP */
#define CMD_DATA_WRITE             DEFCMD(STA01, 0xc4)  /* Data write and ADP same */
#define CMD_DATA_READ              DEFCMD(STA01, 0xc5)  /* Data read and ADP same */
/* Miscellaneous */
#define CMD_SCREEN_PEEK            DEFCMD(STA01, 0xe0)  /* Screen peek */
#define CMD_SCREEN_COPY            DEFCMD(STA01, 0xe8)  /* Screen copy */
#define CMD_BIT_SET_RESET          DEFCMD(STA01, 0xf0)  /* Bit set/reset */

// The next status check value
static uint8_t status_value = STA01;

//////////////////////////////////////////////////////////////////////////////
/// Perform a STA1 status check. This blocks until the status is set by the
/// controller.
///
static void
status_check (void)
{
    uint8_t expected_status = status_value;   // Let compiler know it is read once!
    uint8_t bus_status;                       // Status on the bus

    // Status check: Wait for the controller to be ready.
    do
    {
        PORTC &= ~((1 << CE) | (1 << RD));  // Chip enable + read

        // We need a minimum 150ns delay here.
        DDRB &= ~0x03;                  // Alternative to NOPs
        DDRD &= ~0xfc;

        // Pull the data in.
        bus_status = PINB & 0x03;
        bus_status |= PIND & 0xfc;

        PORTC |= (1 << CE) | (1 << RD); // Deselect the chip.
    }
    while ((bus_status & expected_status) != expected_status);
}

/////////////////////////////////////////////////////////////////////////////
/// Write a data byte to the controller.
///
/// @param [in] data The data to write.
///
static void
data_write (uint8_t data)
{
    // Status check 1: Wait for the controller to be ready.
    status_check ();

    // Set the port direction registers to make data pins outputs.
    DDRB |= 0x03;
    DDRD |= 0xfc;

    // Clear PB7:2 and PD1:0 in preparation for data.
    PORTD &= 0x03;
    PORTB &= 0xfc;

    // Set up the data onto the lines.
    // Mask off PB1:0 and PD7:2 so we don't change them and then write the
    // bits.
    PORTB |= data & 0x03;
    PORTD |= data & 0xfc;
    PORTC &= ~((1 << CD) |              // Data command
               (1 << WR) | (1 << CE));  // Write + Chip enable

    // We need a minimum 80ns delay here.
    asm volatile ("nop");
    asm volatile ("nop");

    PORTC |= ((1 << WR) | (1 << CE) |   // Deselect the chip
              (1 << CD));               // Set default state.
}

/////////////////////////////////////////////////////////////////////////////
/// Read a data byte from the controller.
///
/// @return the byte read from the controller
///
static uint8_t
data_read (void)
{
    uint8_t data;

    // Status check 1: Wait for the controller to be ready.
    status_check ();

    // Perform the read
    PORTC &= ~((1 << CD) |              // Data transaction
               (1 << CE) | (1 << RD));  // Chip enable + read

    // We need a minimum 150ns delay here.
    DDRB &= ~0x03;                      // +62.5ns
    DDRD &= ~0xfc;                      // +125.0ns

    // Pull the data in.
    data = PINB & 0x03;
    data |= PIND & 0xfc;

    // CD delay 10ns - not required.
    // Go back to our known state for the signal lines.
    PORTC |= ((1 << CE) | (1 << RD) |   // Deselect the chip.
              (1 << CD));

    // Return the data to the caller.
    return data;
}

/////////////////////////////////////////////////////////////////////////////
/// Write a command to the controller. Note that "reading" a command is
/// nonsensical and no cmd_read() function is provided.
///
/// @param [in] command The command to write.
///                     The lower 8-bits contain the command.
///                     The upper 8-bits contain next status to be read.
///
static void
cmd_write (uint16_t command)
{
    // Status check 1: Wait for the controller to be ready.
    status_check ();

    // Set the port direction registers to make data pins outputs.
    DDRB |= 0x03;
    DDRD |= 0xfc;

    // Clear PB7:2 and PD1:0 in preparation for data.
    PORTD &= 0x03;
    PORTB &= 0xfc;

    // Set up the data onto the lines.
    // Mask off PB1:0 and PD7:2 so we don't change them and then write the
    // bits.
    PORTB |= (uint8_t)(command) & 0x03;
    PORTD |= (uint8_t)(command) & 0xfc;

    // Note we are expecting CD to be set
    PORTC &= ~((1 << WR)|(1 << CE));    // Chip enable + write

    // We need a minimum 80ns delay here so we do something useful with our
    // time time which is only 2 instructions. Set up the status value for
    // this command to be checked next time.
    status_value = (command >> 8) & 0xff;

    // Deselect the chip.
    PORTC |= ((1 << WR) | (1 << CE));
    // CD delay 10ns - not required.
}

/////////////////////////////////////////////////////////////////////////////
/// Set the pointer to the byte which contains an arbirary x, y point. For
/// our 160 x 128 pixel display, there are 20*128 memory address, so we need
/// a 16-bit value address to refer to the whole graphics area array.
static void
set_pointer (uint8_t x, uint8_t y)
{
    uint16_t address;

    // Calculate which address in our memory space contains the pixel. For
    // each increase in y, we increase by 20 locations. For each 8 pixels in
    // x, we increase by one location. Using a 3-right-shift is a cheap way
    // of doing divide by 8 in a processor without a divide operation. Maybe
    // the compiler knows that, maybe not.
    address = (y * SCREEN_COLUMNS) + (x >> 3);

    // Now that we have our address, we can write our data out. This is the
    // low byte of the address
    data_write ((uint8_t)(address & 0xff));

    // This is the high byte of the address
    data_write ((uint8_t)(address >> 8));

    // This is the command for "set pointer address".
    cmd_write (CMD_SET_ADDR_POINTER);
}

/////////////////////////////////////////////////////////////////////////////
/// Set the pointer to the byte which contains an arbirary x_column, y point.
/// For our 160 x 128 pixel display, there are 20*128 memory address, so we
/// need a 16-bit value address to refer to the whole graphics area array.
static void
set_column_pointer (uint8_t x_column, uint8_t y)
{
    uint16_t address;

    // Calculate which address in our memory space contains the pixel. For
    // each increase in y, we increase by 20 locations. For each 8 pixels in
    // x, we increase by one location. Using a 3-right-shift is a cheap way
    // of doing divide by 8 in a processor without a divide operation. Maybe
    // the compiler knows that, maybe not.
    address = (y * SCREEN_COLUMNS) + x_column;

    // Now that we have our address, we can write our data out. This is the
    // low byte of the address
    data_write ((uint8_t)(address & 0xff));

    // This is the high byte of the address
    data_write ((uint8_t)(address >> 8));

    // This is the command for "set pointer address".
    cmd_write (CMD_SET_ADDR_POINTER);
}

/////////////////////////////////////////////////////////////////////////////
/// Clearing the display. All we're *really* doing is writing a one or zero
/// to all the memory locations for the display.
///
/// @param [in] mode The mode to clear the screen
///                  0x00 MODE_REVERSE - clears with 1's
///                  0x01 MODE_NORMAL  - clears with 0's
void
t6963_screen_clear (uint8_t mode)
{
    uint16_t ii;                        // Local loop counter.
    uint8_t data;                       // The data to write to screen.

    // Determine if the screen is reversed or not. In normal mode we write
    // 0x00 when reversed we write 0xff. Note: 0x00-0x01 = 0xff !!
    data = (mode & MODE_NORMAL_MASK) - 1;

    // Start from the top of memory.
    set_pointer (0,0);

    // Clear the whole memory using auto write.
    cmd_write (CMD_DATA_AUTO_WRITE);

    // Iterate over all of the data
    for (ii = 0; ii < (SCREEN_COLUMNS * SCREEN_HEIGHT); ii++)
        data_write (data);
    cmd_write (CMD_DATA_AUTO_RESET);  // End of auto mode.
}

/////////////////////////////////////////////////////////////////////////////
/// Scroll the display up by 1 character line.
///
/// @param [in] buf The buffer to use for reading and writing
/// @param [in] pixels The number of pixels to scroll where -ve is up
/// @param [in] mode The current mode.
///
void
t6963_vscroll (uint8_t *buf, int8_t pixels, uint8_t mode)
{
    uint8_t yy;                         // The y coordinate.
    
    // TODO: Currently we ignore pixels and simple do -8.
    
    // Determine if the screen is reversed or not. In normal mode we write
    // 0x00 when reversed we write 0xff. Note: 0x00-0x01 = 0xff !!
    mode = (mode & MODE_NORMAL_MASK) - 1;
    
    // Iterate over the whole screen.
    for (yy = 0; yy < SCREEN_HEIGHT; yy += 8)
    {
        uint8_t xx;
        
        // If we are at the last line then fill with the background colour 
        if (yy == (SCREEN_HEIGHT - 8))
        {
            memset (buf, mode, SCREEN_WIDTH);
        }
        else
        {
            // Reset our position to the start of the row.
            set_pointer (0, yy + 8);

            // Read in the existing lines.
            
            // Iterate over a block of data
            cmd_write (CMD_DATA_AUTO_READ);
            for (xx = 0; xx < SCREEN_WIDTH; xx++)
                buf[xx] = data_read ();
            cmd_write (CMD_DATA_AUTO_RESET);  // End of auto mode.
        }
        
        // Reset our position to the start of the row.
        set_pointer (0, yy);

        // Write the reversed lines.
        cmd_write (CMD_DATA_AUTO_WRITE);
        for (xx = 0; xx < SCREEN_WIDTH; xx++)
            data_write (buf[xx]);
        cmd_write (CMD_DATA_AUTO_RESET); // End of auto mode.
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Reverse the display. We read all of the screen values, invert them and
/// then write them back.
///
/// @param [in] buffer A buffer to use for 8 lines of screen data.
///
void
t6963_screen_reverse (uint8_t *buf)
{
    uint16_t yy;                        // Local loop counter.

    // Iterate over the whole screen.
    for (yy = 0; yy < SCREEN_HEIGHT; yy += 8)
    {
        uint8_t xx;

        // Reset our position to the start of the row.
        set_pointer (0, yy);

        // Read in the existing lines.

        // Iterate over a block of data
        cmd_write (CMD_DATA_AUTO_READ);
        for (xx = 0; xx < SCREEN_WIDTH; xx++)
            buf[xx] = data_read ();
        cmd_write (CMD_DATA_AUTO_RESET);  // End of auto mode.

        // Reset our position to the start of the row.
        set_pointer (0, yy);

        // Write the reversed lines.
        cmd_write (CMD_DATA_AUTO_WRITE);
        for (xx = 0; xx < SCREEN_WIDTH; xx++)
            data_write (~buf[xx]);
        cmd_write (CMD_DATA_AUTO_RESET); // End of auto mode.
    }
}

////////////////////////////////////////////////////////////////////////////////////
/// First unitialisation of the device. Set up the display hardware.
///
void
t6963_init (void)
{
    // Port C is the control
    PORTC = ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
    DDRC =  ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));

    // Set up the screen size.
    x_dim = SCREEN_WIDTH;
    y_dim = SCREEN_HEIGHT;

    // The first part of display initialization is to set the start location
    // of the graphics in memory. We'll set it to 0x0000.

    // Write the low byte of the graphics home address.
    data_write (0x00);
    // Write the high byte of the graphics home address.
    data_write (0x00);
    // "Write graphics home address" command.
    cmd_write (CMD_SET_GRAPIC_HOME_ADDR);

    // Next, we need to set the graphics area. This is the length of each
    // line before the line wraps to the next one. Note that it does not have
    // to equal the actual number of pixels in the display- just equal to or
    // greater than.

    // Number of bytes per line (160 pixels/8 bits per byte)
    data_write (SCREEN_COLUMNS);
    // Always zero.
    data_write (0);
    // "Write graphics area" command.
    cmd_write (CMD_SET_GRAPHIC_AREA);

    // Now we need to write the mode set command; most likely, this is not
    //  needed, because the defaults should work, but never trust the defaults.
    //  Also, I *think* this only really affects the way text is rendered, and
    //  since we're only doing graphics rendering, it probably doesn't matter.
    //  Register takes the form
    //    1  0  0  0  CG  MD2  MD1  MD0
    //  CG -    0   = internal ROM character generation
    //          1   = RAM character generation
    //  MD2-0 - 000 = OR mode
    //          001 = XOR mode
    //          010 = AND mode
    //          100 = TEXT ATTRIBUTE mode
    cmd_write (CMD_MODE_SET);

    // There's a DISPLAY MODE command, too. This seems more likely to be useful
    //  to us. The register takes the form
    //     1  0  0  1  GRPH  TEXT  CUR  BLK
    //  GRPH - 1/0 graphics on/off
    //  TEXT - 1/0 text display on/off
    //  CUR  - 1/0 text cursor displayed/not displayed
    //  BLK  - 1/0 text cursory blink on/off
   cmd_write (CMD_DISPLAY_GRAPHIC);
}

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
void
t6963_set_pixel (uint8_t x, uint8_t y, uint8_t mode)
{
    uint8_t xbit = x & 7;               // The sub-column
    
    // NOTE: There are savings to be made here as we probably do not always
    // need to set the address as we can write 8-pixels with 1 address set.

    // If this is a copy over then simply
    if ((mode & MODE_OP_MASK) == 0)
    {
        uint16_t data;

        // This is a straight forward copy over operation.

        // Step one: select the byte in question.
        set_pointer(x, y);

        data = CMD_BIT_SET_RESET;       // Bitset command.

        // Apply any reverse setting
        if ((mode & MODE_NORMAL_MASK) != MODE_REVERSE)
            data |= 0x08;

        // Figure out which bit we're interested in setting/clearing
        data |= 7 - xbit;              // Add the bit index into byte

        cmd_write(data);                // Send the command.
    }
    else
    {
        // The caller has requested a modification of the bit based on the
        // screen contents. Perform a read-modify-write operation to set the
        // pixel to the right value.
        //
        // Convert x to a column value
        // Set data to pixel to write by bit shifting 0 to the top bit.
        // Pass the mode in the call
        t6963_set_row (x >> 3, y, 0xff, 
                       pgm_read_byte(&bit_shift_rev_single_maskP [xbit]), 
                       mode | MODE_MERGE);
    }
}

static __inline__ uint8_t
merge_row (uint8_t new_row, uint8_t orig_row, uint8_t mode)
{
    // Normalise the mode to retrieve the operator.
    mode &= MODE_OP_MASK;

    if (mode >= MODE_XOR)
    {
        // This is XOR or NAND */
        if ((mode & MODE_XOR) != 0)
        {
            // MODE_XOR - XOR the existing buffer data with read data.
            new_row ^= orig_row;
        }
        else
        {
            // MODE_NAND - Clear bits according to buffer.
            new_row = ~new_row & orig_row;
        }
    }
    else
    {
        // MODE_OR - Set bits according to buffer.
        new_row |= orig_row;
    }

    // Return the merge to the caller.
    return new_row;
}

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
void
t6963_read_row (uint8_t x_column, uint8_t y, uint16_t length, uint8_t *buf, uint8_t mode)
{
    uint8_t data;

    // This sets our pointer to the location containing
    set_column_pointer (x_column, y);

    // Use a different command depending on how many bytes are being read.
    // For a length of 1 then simply perform a single read command. If the
    // length is >= 2 then it is quicker to perform an auto read function.
    if (length == 1)
    {
        cmd_write (CMD_DATA_READ);      // A non-incrementing read.
        data = data_read();             // Get the data from controller

        // Apply any reverse setting.
        if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
            data = ~data;

        // Handle any buffer merging on the read.
        if ((mode & MODE_MODIFIER) != 0)
            data = merge_row (*buf, data, mode);

        // Assign the data to the buffer.
        *buf = data;
    }
    else
    {
        int ii;

        // Perform an auto read to collect the data.
        cmd_write (CMD_DATA_AUTO_READ);

        // Iterate over all of the data
        for (ii = 0; ii < length; ii++)
        {
            data = data_read();         // Pick up the data

            // Reverse the data if required.
            if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
                data = ~data;
            
            // Handle any buffer merging on the read.
            if ((mode & MODE_MODIFIER) != 0)
                data = merge_row (buf[ii], data, mode);

            // Assign the data to the buffer.
            buf[ii] = data;
        }
        // End of auto mode
        cmd_write (CMD_DATA_AUTO_RESET);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Write a row to the display.
/// The row is written colum wise where x will start at (x & 0xf8)
///
/// @param [in] x_column The column to write (x % 8)
/// @param [in] y The row to read.
/// @param [in] length The number of columns to read.
/// @param [in] buf Location to write from.
///
void
t6963_write_row (uint8_t x_column, uint8_t y, uint16_t length, uint8_t *buf, uint8_t mode)
{
    // This sets our pointer to the location containing
    set_column_pointer (x_column, y);

    // Use a different command depending on how many bytes are being written.
    // For a length of 1 then simply perform a single write command. If the
    // length is >= 2 then it is quicker to perform an auto write function.
    if (length == 1)
    {
        // Write the data first and then send the command.
        data_write (*buf);              // Send data to controller
        cmd_write (CMD_DATA_WRITE);     // A non-incrementing write.
    }
    else
    {
        int ii;

        // Perform an auto read to collect the data.
        cmd_write (CMD_DATA_AUTO_WRITE);

        // Iterate over all of the data
        for (ii = 0; ii < length; ii++)
        {
            uint8_t data = *buf++;

            // Perform a reverse if required.
            if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
                data = ~data;

            // Write the data.
            data_write (data);
        }

        // End of auto mode
        cmd_write (CMD_DATA_AUTO_RESET);
    }
}

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
void
t6963_rewrite_row (uint8_t x_column, uint8_t y, uint16_t length, uint8_t *buf, uint8_t mode)
{
    // Use a different command depending on how many bytes are being read.
    // For a length of 1 then simply perform a single read command. If the
    // length is >= 2 then it is quicker to perform an auto read function.
    if (length == 1)
    {
        uint8_t data;

        // This sets our pointer to the location containing
        set_column_pointer (x_column, y);

        if ((mode & MODE_MODIFIER) != 0)
        {
            cmd_write (CMD_DATA_READ);      // A non-incrementing read.
            data = data_read();             // Get the data from controller

            // Apply any reverse setting
            if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
                data = ~data;

            // Handle any buffer merging on the read.
            data = merge_row (*buf, data, mode);
        }
        else
            data = *buf;

        if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
            data = ~data;

        // Write the data first and then send the command.
        data_write (data);              // Send data to controller
        cmd_write (CMD_DATA_WRITE);     // A non-incrementing write.
    }
    else
    {
        // Simply do a read_row() followed by write_row(). Only perform the
        // read if there is a data merge operation.
        if ((mode & MODE_MODIFIER) != 0)
        {
            // Read for the merge.
            t6963_read_row (x_column, y, length, buf, mode);
        }

        // Write the merged data.
        t6963_write_row (x_column, y, length, buf, mode);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Sets/Draws a single row to the screen.
/// The row is read colum wise where x is (x & 0xf8)
///
/// @param [in] x_column The column to read (x % 8)
/// @param [in] y The row to re-write.
/// @param [in] data The data to write.
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
///             0x02 - MODE_MERGE
///                    A copy with merge required.
///                    buffer[x] = read_data
///
///             0x04 - MODE_OR
///                    Merge - OR bits set in buffer
///                    buffer[x] = buffer[x] | read_data
///
///             0x08 - MODE_XOR
///                    Merge - XOR bits set in buffer
///                    buffer[x] = buffer[x] ^ read_data
///
///             0x0c - MODE_NAND
///                    Merge required - NAND bits cleared in buffer
///                    buffer[x] = ~buffer[x] & read_data
///
void
t6963_set_row (uint8_t x_column, uint8_t y, uint8_t data, uint8_t mask, uint8_t mode)
{
    // This sets our pointer to the location containing
    set_column_pointer (x_column, y);
    
    // Handle any buffer merging on the read.
    if ((mode & MODE_MODIFIER) != 0)
    {
        uint8_t screen_data;

        // We need to perform a merge
        cmd_write (CMD_DATA_READ);  // A non-incrementing read.
        screen_data = data_read();  // Get the data from controller

        // Apply any reverse setting; if the reverse bit is set then we
        // negate the data
        if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
            screen_data = ~screen_data;

        // Perform the merge
        if ((mode & MODE_OP_MASK) != 0)
            data = merge_row (data, screen_data, mode);
        
        // MODE_MERGE - Merge in the data in a copy mode
        data = (data & mask) | (screen_data & ~mask);
    }

    // Apply any reverse setting; if the reverse bit is set then we
    // negate the data
    if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
        data = ~data;

    // Write the data first and then send the command.
    data_write (data);                  // Send data to controller
    cmd_write (CMD_DATA_WRITE);
}

/////////////////////////////////////////////////////////////////////////////
/// Flip bitbit data from vertical to horizontal layout. This flits
/// vertically organised bitblt data to be horizontal. Thi requires a lot of
/// shifing. There are some clever 64-bit multiplacation functions aroudn
/// that perform this transformation but is not for the Arduino. This
/// implementation using brute force bit moving to achieve the rotation.
///
/// A reference to buffer that is passed in is re-organised and returned to
/// the caller in-place.
///
/// @param [in] buf An 8x8 buffer of data to be reorganised.
///
static __inline__ void
flip_8x8_v_to_h (uint8_t *buf)
{
    uint8_t dest [8];
    int8_t xx;
    int8_t yy;

    // Flip the bits.
    yy = 7;
    do
    {
        uint8_t datum = buf[yy];
        xx = 7;
        do
        {
            dest[xx] = (dest[xx] >> 1) | (datum & 0x80);
            datum <<= 1;
        }
        while (--xx >= 0);
    }
    while (--yy >= 0);

    // Copy the results back
    xx = 7;
    do
    {
        buf[xx] = dest[xx];
    }
    while (--xx >= 0);
}

//////////////////////////////////////////////////////////////////////////////
/// Bitblt a line. This is a worker function for bitblt that prepares a line
/// of data to be written to the screen managing the merge of screen data at
/// the start and end of the bitblt data.
///
/// @param [in] x The x-coordinate on the screen.
/// @param [in] y The y-coordinate on the screen.
/// @param [in] width The width of the data in pixels.
/// @param [in] data A pointer to the bitblit row.
/// @param [in] mode The merging mode of the data.
///
static void
bitblt_line (uint8_t x, uint8_t y, uint8_t width, uint8_t *data, uint8_t mode)
{
    uint8_t ii;                         // Local iterator

    // See if there is a left merge.
    ii = x & 7;                         // Get the shift on the left
    x >>= 3;                            // Convert to column address
    if (ii != 0)
    {
        uint8_t valid_mask;             // Mask for the valid bits to write.

        // Create mask for start block i.e. 0xff >> ii
        valid_mask = pgm_read_byte (&bit_shift_maskP[ii]);
        width += ii;                    // Add the sub-block start

        // Add any right mask if we have a short block.
        if (width <= 8)
        {
            // Add any right mask if we have a short block.
            if (width < 8)
            {
                // This is a short block, the data does not occupy the block.
                // Modify the mask to handle the right of the data.
                // i.e. ((1 << (8 - width)) - 1)
                valid_mask &= ~pgm_read_byte (&bit_shift_maskP[width]);
            }

            // Write the start of row.
            t6963_set_row (x, y, *data++, valid_mask, mode | MODE_MERGE);
            return;
        }

        // More than 1 block, compute the new length
        width -= 8;
        // This is a partial update so merge the data into the screen data.
        t6963_set_row (x++, y, *data++, valid_mask, mode | MODE_MERGE);
    }

    // Process the whole blocks; convert to column count
    if ((ii = (width >> 3)) > 0)
    {
        // Re-write the row, this will determine if we need to merge or not.
        t6963_rewrite_row (x, y, ii, data, mode);
    }

    // Process any remaining block
    width &= 7;
    if (width > 0)
    {
        uint8_t valid_mask;             // Mask for the valid bits to write.

        // This is a partial update so merge the data into the screen data.
        // Create the mask of invalid bits is  (1 << (8 - width)) - 1 i.e. bit_shift_mask;
        valid_mask = ~pgm_read_byte (&bit_shift_maskP[width]);
        t6963_set_row (x + ii, y, data[ii], valid_mask, mode | MODE_MERGE);
    }
}

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
void
t6963_vbitblt (uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t *data)
{
    uint8_t left_margin;                // Remainder bits on the left
    uint8_t right_remain;               // Number of remaining bits on line
    uint8_t length;                     // Block aligned length.
    uint8_t row;                        // The current row being processed.
    uint8_t col;                        // The current column being processed.
    uint8_t row_inc;                    // The row increment

    // Calculate the left margin and left aligned length
    left_margin = (x & 7);              // The left margin
    length = left_margin + width;       // Length of the left block aligned

    // Compute the row increment based on the length of the image
    row_inc = (length + 7) >> 3;

    // The margin to the right
    right_remain = length & 7;          // Remaining bits on line
    // right_margin = (8 - (right_remain)) & 0x7;

    // Iterate over all of the rows.
    for (row = 0; row < height; row += 8)
    {
        uint16_t buf_index;             // Index to buffer
        uint8_t max_rows;               // The maximum number of rows
        uint8_t ii;                     // General iterator

        /* Convert all of the data */
        for (col = 0; col < length; col += 8)
        {
            uint8_t right_index;
            uint8_t left_index;
            uint8_t fbuf [8];

            // Process the first row.
            left_index = 0;
            if (col == 0)
                left_index = left_margin;
            right_index = 8;
            if ((length - col) < 8)
                right_index = right_remain;

            // Read in the data.
            do
            {
                uint8_t datum;

                // Write the data data to the buffer
                if (data == NULL)
                {
                    // Collect the data from serial.
                    datum = serial_getc ();
                }
                else
                {
                    // Collect the data from the parameter.
                    datum = *data;

                    // Advance the datum if we are not in fill mode.
                    if ((mode & MODE_FILL) == 0)
                        data++;
                }

                // Save the data in the local buffer prior to conversion
                fbuf [left_index] = datum;
            }
            while (++left_index < right_index);

            // Flip the data from vertical to horizontal
            flip_8x8_v_to_h (fbuf);

            // Organise in the correct position in the horizontal line such
            // that the horizontal blocks are separated by a line width.
            buf_index = col >> 3;
            for (ii = 0; ii < 8; ii++)
            {
                draw_buffer [buf_index] = fbuf[ii];
                buf_index += row_inc;
            }
        }

        // Display the data row. Calculate the chung of data we have buffered
        // and the position in the buffer.
        max_rows = height - row;
        if (max_rows > 8)
            max_rows = 8;

        // Iterate over the rows
        for (buf_index = 0, ii = 0; ii < max_rows; ii++)
        {
            // Write the line.
            bitblt_line (x, y++, width, &draw_buffer [buf_index], mode);
            buf_index += row_inc;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Draw a horizontal line
///
/// @param [in] x The first x-coordinate.
/// @param [in] y The first y-coordinate.
/// @param [in] x1 The 2nd x-coordinate.
/// @param [in] mode The drawing mode.
///
void
t6963_hline (uint8_t x, uint8_t y, uint8_t x1, uint8_t mode)
{
    uint8_t width;                      // Width of line.
    // Use the draw_buffer. We offset at 128 because all functions that use
    // the draw buffer are restricted to 128 bytes. The biggest user is
    // polygon fill.
    uint8_t *buffer = (draw_buffer + 128);
    //uint8_t buffer [SCREEN_COLUMNS];  // Stack based if we have room

    // Check for a single pixeland handle imediately, no savings to be made.
    if (x == x1)
    {
        t6963_set_pixel (x, y, mode);
        return;
    }

    // Swap the bytes to ensure x is smaller than x1.
    if (x > x1)
        swap_bytes (x, x1);

#if 0
    // Simply using set_pixel; used for testing to confirm that this function is OK.
    while (x <= x1)
        t6963_set_pixel (x++, y, mode);
    return;
#endif

    width = (x1 - x) + 1;               // Calculate the width
    x1 = x & 7;                         // Get the offset to start of block
    x >>= 3;                            // Get the column index
    if (x1 != 0)                        // Not on boundary?
    {
        uint8_t valid_mask;             // Mask for the valid bits to write.

        // Compute the valid mask i.e. 0xff >> x1
        valid_mask = pgm_read_byte (&bit_shift_maskP[x1]);
        width += x1;                    // Add the sub-block start to width

        // Add any right mask if we have a short block
        if (width < 8)
        {
            // This is a short block, the data does not occupy the block.
            // Modify the mask to handle the right of the data.
            // i.e. valid_mask &= ~((1 << (8 - width)) - 1);
            valid_mask &= ~pgm_read_byte (&bit_shift_maskP[width]);
        }
        
        // Write the start of row.
        t6963_set_row (x, y, valid_mask, valid_mask, mode | MODE_MERGE);
        
        // Deal quickly with a single block.
        if (width <= 8)
            return;                     // Finished, quit now.
        width -= 8;
        x++;
    }
    
    // Process the whole blocks; convert to a column count
    if ((x1 = (width >> 3)) > 0)
    {
        uint8_t offset = 0;
        
        do
        {
            buffer[offset++] = 0xff;
        }
        while (--x1 > 0);
        
        // Write the data
        t6963_rewrite_row (x, y, offset, buffer, mode);
        
        // Advance x over the data written.
        x += offset;
    }

    // Process any remaining block
    width &= 0x07;
    if (width > 0)
    {
        uint8_t valid_mask;
        
        // Create the mask of valid bits.
        // buffer[offset++] = ~((uint8_t)(1 << (8 - width)) - 1);
        valid_mask = ~pgm_read_byte (&bit_shift_maskP[width]);
        t6963_set_row (x, y, valid_mask, valid_mask, mode|MODE_MERGE);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Draw a vertical line. The fastest way to draw this is to set pixels.
///
/// @param [in] x The first x-coordinate.
/// @param [in] y The first y-coordinate.
/// @param [in] y1 The 2nd y-coordinate.
/// @param [in] mode The drawing mode.
///
void
t6963_vline (uint8_t x, uint8_t y, uint8_t y1, uint8_t mode)
{
    // Swap the bytes so that we can iterate
    if (y > y1)
        swap_bytes (y, y1);

    // Iterate between the two y coordinates.
    while (y <= y1)
    {
        t6963_set_pixel (x, y++, mode);
    }
}
