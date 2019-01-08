/* -*- C++ -*- **************************************************************
 *
 *  System        : SerialGLCD
 *  Module        : KS0108B driver
 *  Object Name   : $RCSfile: ks0108b.c,v $
 *  Revision      : $Revision: 1.30 $
 *  Date          : $Date: 2015/07/05 21:09:53 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Thu Apr 16 21:24:20 2015
 *  Last Modified : <150705.1113>
 *
 *  Description   : Samsung KS0108B LCD screen driver.
 *
 *  Notes         : Ground up implementation of the KS0180b driver with some
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
 * 26 April 2015 - Jon Green (jon at jasspa) / jasspa.com
 *
 * This is a re-write of the KS0108b driver. Both the Sparkfun and Jennifer
 * Holt implementations used a timer implementation and did not follow the
 * Samsung data sheet for the KS0108b. This version implements the timings
 * from the Samsung data sheet and performs a status check to determine when
 * the chip is ready to accept the next command. This method considerably
 * speeds up the chip access and has significantly changed the structure of
 * the driver code. The abstractions used in the Sparkfun implementation i.e.
 * io_setup() have been discarded in favour of explicitly setting the I/O
 * state in the 3 major read/write methods: ks0108b_read(), ks0108b_write()
 * and status_check().
 *
 * This implementation discards the concept of pages which was used more in
 * the Jennifer Holt version. Rather than maintaining the page state then it
 * is easier to deal with each side of the LCD explicitly in the block/column
 * read and write methods.
 *
 * The Samsung data sheet uses an X and Y nomenclature for the axis of the
 * screen; these are used incorrectly in the conventional sense and the data
 * sheet uses X for a row address and Y for a column address.
 *
 * The EN twiddling for chip enable from both of the previous implementations
 * outwardly appeared to be a little strange when reading the code. The
 * Samsung data sheet made reference to this in a note on LCD data reading.
 * The implementation used here performs a full cycle dummy read (i.e. a
 * ks0108b_read() operation) and the data is discarded; a 2nd and any
 * subsequent reads are performed to clock the data out. The chip requires
 * the first read to transfer the data from the LCD screen to an internal
 * register; it is this first read that is discarded. The 2nd and subsequent
 * reads then extract the data from the internal register and place it on the
 * data bus to be picked up externally. Where performing multiple reads or
 * writes the column position is auto incremented and subsequent reads will
 * continue to clock the data out so the dummy read is only required on the
 * first read when reading multiple bytes. A dummy write cycle is not
 * required.
 *
 * One further note. I cannot fully explain why the Jennifer Holt version
 * needed to address the rows with 63 as the top left corner of the screen.
 * This version uses 0 as the top left coordinate and does not perform any
 * modification. The Samsung datasheet makes mention of the fact that the ADC
 * line voltage changes the coordinate system. There is no information in the
 * LCD datasheet that the ADC can be changed (unless this is related to the E
 * signal).
 *
 * Extreme care should be taken modifying any of the chip access timings. The
 * chip seems to be very sensitive to timings and control/data line settings
 * which are very difficult to track down and isolate; this might be easier
 * if I had a logic analyzer and could see what was going on. The current
 * implementation is very careful in setting any of the PORTC lines and
 * ensures that the there are no intermediate transitions when setting
 * states. In addition the data lines (PORTD and PORTB) are disabled and
 * restored to inputs as soon as any write sequence has finished. These two
 * methods together seem to have removed screen corruption. Changes to the
 * chip read / write sequence are are difficult to identify immediately and
 * may appear to work but they may manifest themselves under heavy load and
 * can be spotted by single pixel corruption on screen i.e. a pixel set
 * incorrectly; either set or clear when it should not be. If the timings are
 * modified in any way then it is advised that a lot of testing is required
 * using both sides of the screen to ensure that no regresions are
 * introduced.
 ***************************************************************************/

/****************************************************************************
 * 02 May 2013 - Mike Hord, SparkFun Electronics
 *
 * This code is released under the Creative Commons Attribution Share-Alike
 * 3.0 license. You are free to reuse, remix, or redistribute it as you see
 * fit, so long as you provide attribution to SparkFun Electronics.
 *
 * Copyright (c) 2010 Jennifer Holt
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
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include "glcd.h"

// 128x64 external control line definitions
#define EN               0              /* PC0 - Chip enable */
#define RS               1              /* PC1 - Data = H or Instruction = L */
#define RW               2              /* PC2 - Read/Write */
#define RES              3              /* PC3 - Reset */
#define CS1              4              /* PC4 - Chip select 1 (x < 64) or both */
#define CS2              5              /* PC5 - Chip select 2 (x > 64) or both */
#define BL_EN            2              /* PB2 - Backlight enable */

#define CMD_RS           ((1 << RS) << 8)
#define CMD_RW           ((1 << RW) << 8)
#define CMD_RES          ((1 << RES) << 8)
#define CMD_CS1          ((1 << CS1) << 8)
#define CMD_CS2          ((1 << CS2) << 8)
#define CMD_CS12         (CMD_CS1|CMD_CS2)

// Define the display commands.

/* Turn the display off */
#define CMD_DISPLAY_ON   (0x003f | CMD_CS12 | CMD_RES)
/* Turn the display on */
#define CMD_DISPLAY_OFF  (0x003e | CMD_CS12 | CMD_RES)
/* Set the column address */
#define CMD_COLUMN       (0x0040 | CMD_RES)
/* Set the row address */
#define CMD_ROW          (0x00b8 | CMD_RES)
/* Data read */
#define CMD_READ         (CMD_RS | CMD_RW | CMD_RES)
/* Data write */
#define CMD_WRITE        (CMD_RS | CMD_RES)

// Define the dimensions of the screen
#define SCREEN_WIDTH 128                /* Screen width */
#define SCREEN_HEIGHT 64                /* Screen height */
#define SCREEN_ROWS  (SCREEN_HEIGHT>>3) /* Number of screen rows */
#define SCREEN_PAGE  (SCREEN_WIDTH>>1)  /* Size of screen page */

// Screen masks
#define SCREEN_WIDTH_MASK  (SCREEN_WIDTH - 1)
#define SCREEN_HEIGHT_MASK (SCREEN_HEIGHT-1)
#define SCREEN_ROWS_MASK   0x07

static uint8_t y_row;                   /* The current y row position */

static __inline__ uint8_t
merge_column (uint8_t new_column, uint8_t orig_column, uint8_t mode)
{
    // Normalise the mode to retrieve the operator.
    mode &= MODE_OP_MASK;

    if (mode >= MODE_XOR)
    {
        // This is XOR or NAND */
        if ((mode & MODE_XOR) != 0)
        {
            // MODE_XOR - XOR the existing buffer data with read data.
            new_column ^= orig_column;
        }
        else
        {
            // MODE_NAND - Clear bits according to buffer.
            new_column = ~new_column & orig_column;
        }
    }
    else
    {
        // MODE_OR - Set bits according to buffer.
        new_column |= orig_column;
    }

    // Return the merge to the caller.
    return new_column;
}

//////////////////////////////////////////////////////////////////////////////
/// Perform status check to make sure that the controller is ready.
/// controller. The status_check is performed for the last command that
/// was issued and active on PORTC. The next command to execute is passed
/// in, if the chip_select is different then a status check is performed
/// on the new chip before returning to the caller.
///
/// The 2nd status check is required, when it is not present then 1/100
/// commands on a chip select change fail which case a screen anomaly.
///
/// @param [in] portc_next The next portc setting.
static void
status_check (uint8_t portc_next)
{
    uint8_t portc;                      // Port c setting

    // Set up for status read. Chip select is already defined.
    portc = PORTC;

    // Run a single iteration of the loop in the normal case where the chip
    // select does not change. Run 2 iterations of the loop in the case where
    // the chip select does change. The first loop checks the status of the
    // previous command we executed and takes values from PORTC itself. The
    // 2nd loop uses the new commands chip select value and ensures that the
    // newly addresed chip is ready for a command.
    for (;;)
    {
        uint8_t portc_en;               // Port c setting with EN

        // Set up PORTC for a status read. Note if we go round the loop again
        // then we need a 500ns strobe so do not fold the commands into the
        // nop's
        portc &= ~(1 << RS);
        portc |= (1 << RW);
        portc_en = portc | (1 << EN);

        // Change the control lines.
        PORTC = portc;

        // Wait at least 140ns before we strobe EN (Tasu) after changing
        // RS+RW. Instead of doing NOPs then prepare the port ready for the
        // write.
        asm volatile ("nop");           //  62.5ns
        asm volatile ("nop");           // 125.0ns
        asm volatile ("nop");           // 187.5ns
        PORTC = portc_en;               //   0.0ns
        // Wait a minimum of 320ns (Td) after EN before attempting a read
        asm volatile ("nop");           //  62.5ns
        asm volatile ("nop");           // 125.0ns
        asm volatile ("nop");           // 187.5ns
        asm volatile ("nop");           // 250.0ns
        asm volatile ("nop");           // 312.5ns
        asm volatile ("nop");           // 375.0ns

        // With the chip enabled continually poll the status until it changes
        // to a ready state. We only pull in the bits we need and can ignore
        // the lower bits as we do not use them
        while ((PIND & 0x80) != 0)      // 437.5ns
            /* Nothing */;
        PORTC = portc;                  // 500.0ns

        // Check if the next portc is going to change the chip select, if it
        // is then we re-execute the loop.
        if (((portc ^ portc_next) & ((1 << CS1) | (1<< CS2))) == 0)
            break;                      // Finished

        // Assign the new portc value to be processed, we will drop out of
        // the loop on the next iteration.
        portc = portc_next;
    }
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
ks0108b_write (uint16_t data)
{
    uint8_t portc;                      // New portc setting
    uint8_t portc_en;                   // New portc setting with EN
    uint8_t ddrb;                       // Temporary of port B control

    // Prepare the portc ready for testing when the status check completes.
    portc = data >> 8;                  // Get the port data

    // Ensure the chip is ready for the operation. Send the new portc setting
    // so that the new chip select may be checked for readyness.
    status_check (portc);

    // Prepare the control lines.
    PORTC = portc;

    // Wait at least 140ns (tASU) before toggling the chip enable. Instead of
    // doing NOPs then prepare the port ready for the write.
    portc_en = portc | (1 << EN);       // 62.5ns

    // Prepare the Port-B data
    PORTD = data & 0xfc;                // 125.0ns, 187.5ns,
    PORTB = (PORTB & ~0x03) | (data & 0x03);
    ddrb = DDRB | 0x03;

    // Enable the line the 500ns strobe starts from now.
    PORTC = portc_en;                   //   0.0ns

    // Wait 500ns before we toggle EN. The data must go on the bus by the
    // 300ns (Tdsu) mark.
    DDRB = ddrb;                        //  62.5ns,
    DDRD = 0xfc;                        // 125.0ns, 187.5ns
    asm volatile ("nop");               // 250.0ns
    asm volatile ("nop");               // 312.5ns
    asm volatile ("nop");               // 375.0ns
    asm volatile ("nop");               // 437.5ns
    PORTC = portc;                      // 500.0ns
    // Wait 10ns - we do not need to do anything here.
    // Leave the data lines set to input
    DDRD = 0;
    DDRB &= ~0x03;
}

/////////////////////////////////////////////////////////////////////////////
/// Read a byte of data from the screen
///
/// @return The screen data read.
/// /
static uint8_t
ks0108b_read (uint16_t command)
{
    uint8_t data;                       // The data to read
    uint8_t portc;                      // New port c setting
    uint8_t portc_en;                   // New port c setting with EN

    // Set up the data lines. For data read we need RS=1 and RW=1
    portc = command >> 8;

    // Ensure the chip is ready for the operation. Send the new portc setting
    // so that the new chip select may be checked for readyness.
    status_check (portc);

    // Wait 140ns (tASU) before toggling the chip enable. Instead of doing
    // NOPs then prepare the port ready for the read.
    PORTC = portc;                      //   0.0ns
    portc_en = portc | (1 << EN);       //  62.5ns, 125.0ns
    asm volatile ("nop");               // 187.5ns

    // Enable the command
    PORTC = portc_en;                   //   0.0ns

    // Wait 500ns before we toggle EN.
    // Wait a minimum of 320ns (Td) after EN before attempting a read
    asm volatile ("nop");               //  62.5ns
    asm volatile ("nop");               // 125.0ns
    asm volatile ("nop");               // 187.5ns
    asm volatile ("nop");               // 250.0ns
    asm volatile ("nop");               // 312.5ns
    asm volatile ("nop");               // 375.0ns
    // Pull the data in
    data = PINB & 0x03;                 // 437.5ns
    data |= PIND & 0xfc;                // 500.0ns
    PORTC = portc;                      // +500.0ns

    return data;
}

//////////////////////////////////////////////////////////////////////////////
/// Set the y column. This function caches the current y row position and
/// instructs the display to modify the row position when the cached version
/// does not match the new row. This prevents display commands being
/// dispatched for continual position changes on the same line that affect x
/// bit not y.
///
/// @param [in] y The new y position required.
///
static void
set_y_position (uint8_t y)
{
    // Ensure y is in the correct range.
    y &= 0x7;

    // If the row is already selected then skip the position command.
    if (y_row != y)
    {
        // The row position has changed, move to the new row and update the
        // internal row position.
        y_row = y;                      // Update the cached row position.
        ks0108b_write (CMD_ROW|CMD_CS12|y); // Set the row on the display.
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Read a row of bytes from the screen.
/// Reads [length] display bytes from page [page] starting at horizontal
/// value [x] and puts the values in buf automatically accounts for crossing
/// chips. The command may perform an in-place operation and merge existing
/// data that is in the buffer as part of the read process depending on the
/// value of flags.
///
/// @param [in] x The column to start at.
/// @param [in] page The page (y axis) to process.
/// @param [in] length The length (number columns) to read.
/// @param [out] buf The buffer to read the data into.
/// @param [in] mask The data mask
/// @param [in] mode The merge operation to perform.
///             0x00 - No merge required.
///             0x80 - Merge required - NAND bits cleared in buffer
///                    buffer[x] = read_data & ~buffer[x]
///             0x81 - Merge - OR bits set in buffer
///                    buffer[x] |= read_data
static void
read_block (uint8_t x, uint8_t y_row, uint8_t length, uint8_t *buf, uint8_t mask, uint8_t mode)
{
    uint8_t s;                          // Distance to edge of chip
    uint8_t num_bytes1;                 // Side #1 number of bytes
    uint8_t num_bytes2;                 // Side #2 number of bytes
    uint16_t cs_select;                 // The cs_select bits

    // Set the y-position enable the display, write to both chips/pages
    // concurrently.
    set_y_position (y_row);             // Set row

    // See if we are writing to the 2nd side only accounting for x being
    // larger than 63
    if (x >= 64)
    {
        x -= 64;                        // Normalise for chip

        // We are on the 2nd chip first so swap the lcd_read value
        cs_select = CMD_CS1;
    }
    else
    {
        // We are on the first chip.
        cs_select = CMD_CS2;
    }

    s = 64 - x;                         // Distance to edge
    if (length > s)
    {
        num_bytes1 = s;                 // How much to read in the first loop
        num_bytes2 = length - s;        // How much to read in the second loop
    }
    else
    {
        num_bytes1 = length;
        num_bytes2 = 0;
    }

    // Iterate over both sides, we compute the break condition in the loop.
    for (;;)
    {
        // Set the column position.
        ks0108b_write (CMD_COLUMN | cs_select | (x & 0x3f));

        // Perform a dummy read to transfer to register
        ks0108b_read (CMD_READ | cs_select);

        // Loop for the number of bytes to be read.
        do
        {
            uint8_t data;
            uint8_t screen_data;

            // Read the data from the screen.
            screen_data = ks0108b_read (CMD_READ | cs_select);
            data = *buf;

            // Apply any reverse setting; if the reverse bit is set then
            // we negate the data
            if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
                screen_data = ~screen_data;

            // Perform the merge
            if ((mode & MODE_OP_MASK) != 0)
                data = merge_column (data, screen_data, mode);

            // MODE_MERGE - Merge in the data in a copy mode
            data = (data & mask) | (screen_data & ~mask);

            // Assign the data to the buffer.
            *buf++ = data;
        }
        while (--num_bytes1 > 0);

        // See if we have finished.
        if ((num_bytes1 = num_bytes2) == 0)
            break;

        /* Set up for the next loop */
        num_bytes1 = num_bytes2;        // Set up for num_bytes1
        num_bytes2 = 0;                 // Terminate on next loop

        // Select proper chip we move to the other one, this catches any
        // overrun from previous side.
        cs_select = CMD_CS1;

        // We have swapped chips so we will start from column 0.
        x = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Write a row to the display.
/// The row is written colum wise where x will start at (x & 0xf8)
///
/// @param [in] x The column to write
/// @param [in] y_row The row to read (y % 8).
/// @param [in] length The number of columns to read.
/// @param [in] buf Location to write from.
/// @param [in] mode The merge mode required.
///
void
write_block (uint8_t x, uint8_t y_row, uint8_t length, uint8_t *buf, uint8_t mode)
{
    uint8_t s;                          // Distance to edge of chip
    uint8_t num_bytes1;                 // Side #1 number of bytes
    uint8_t num_bytes2;                 // Side #2 number of bytes
    uint16_t cs_select;                 // The cs_select bits

    // Set the y-position enable the display, write to both chips/pages
    // concurrently.
    set_y_position (y_row);             // Set row

    // See if we are writing to the 2nd side only accounting for x being
    // larger than 63
    if (x >= 64)
    {
        x -= 64;                        // Normalise for chip

        // We are on the 2nd chip first so swap the lcd_read value
        cs_select = CMD_CS1;
    }
    else
    {
        // We are on the first chip.
        cs_select = CMD_CS2;
    }

    s = 64 - x;                         // Distance to edge
    if (length > s)
    {
        num_bytes1 = s;                 // How much to read in the first loop
        num_bytes2 = length - s;        // How much to read in the second loop
    }
    else
    {
        num_bytes1 = length;
        num_bytes2 = 0;
    }

    // Iterate over both sides, we compute the break condition in the loop.
    for (;;)
    {
        // Set the column position.
        ks0108b_write (CMD_COLUMN | cs_select | (x & 0x3f));

        // Loop for the number of bytes to be written.
        do
        {
            uint8_t data;

            // Get the data from the buffer
            data = *buf++;

            // Perform a reverse if required.
            if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
                data = ~data;

            // Write the data to the screen.
            ks0108b_write (CMD_WRITE | cs_select | data);
        }
        while (--num_bytes1 > 0);

        // See if we have finished.
        if ((num_bytes1 = num_bytes2) == 0)
            break;

        /* Set up for the next loop */
        num_bytes1 = num_bytes2;        // Set up for num_bytes1
        num_bytes2 = 0;                 // Terminate on next loop

        // Select proper chip we move to the other one, this catches any
        // overrun from previous side.
        cs_select = CMD_CS1;

        // We have swapped chips so we will start from column 0.
        x = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////
/// First unitialisation of the device. Set up the display hardware.
///
void
ks0108b_init (void)
{
    // Set up the screen size.
    x_dim = SCREEN_WIDTH;
    y_dim = SCREEN_HEIGHT;

    // Perform a chip reset; with ^RESet low then the chip should reset.
    PORTC = (1 << RW);
    // Enable the outputs
    DDRC = ((1 << EN) | (1 << RS) | (1 << RW) | (1 << RES) | (1 << CS1) | (1 << CS2));
    _delay_ms (60);
    PORTC |= (1 << RES);

    // Enable the display
    ks0108b_write (CMD_DISPLAY_ON);         // Set up data on lines

    // Reset the column cache position.
    y_row = 0xff;
}

/////////////////////////////////////////////////////////////////////////////
/// Clearing the display. All we're *really* doing is writing a one or zero
/// to all the memory locations for the display.
///
/// @param [in] mode The mode to clear the screen
///                  0x00 MODE_REVERSE - clears with 1's
///                  0x01 MODE_NORMAL  - clears with 0's
void
ks0108b_screen_clear (uint8_t mode)
{
    uint8_t data;                       // The data to write to screen.
    uint8_t yy;                         // The y coordinate.

    // Determine if the screen is reversed or not. In normal mode we write
    // 0x00 when reversed we write 0xff. Note: 0x00-0x01 = 0xff !!
    data = (mode & MODE_NORMAL_MASK) - 1;

    // Iterate over all of the rows.
    for (yy = 0; yy < SCREEN_ROWS; yy++)
    {
        uint8_t xx;                     // The x coordinate.

        // Set the row and column on both chips. We write to both chips at
        // the same time to speed up the screen clear
        set_y_position (yy);            // Set row
        ks0108b_write(CMD_COLUMN|CMD_CS12); // Set column

        // Write the data
        for (xx = 0; xx < SCREEN_PAGE; xx++)
            ks0108b_write (CMD_WRITE | CMD_CS12 | data);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Scroll the display up by 1 line.
///
/// @param [in] buf The buffer to use for reading and writing
/// @param [in] pixels The number of pixels to scroll where -ve is up
/// @param [in] mode The current mode.
///
void
ks0108b_vscroll (uint8_t *buf, int8_t pixels, uint8_t mode)
{
    uint8_t yy;                         // The y coordinate.
    
    // TODO: Currently we ignore pixels and simple do -8.
    
    // Only use the normal and reverse mode
    mode &= MODE_NORMAL_MASK;
    
    // Iterate over all of the rows.
    for (yy = 0; yy < SCREEN_ROWS; yy++)
    {
        // Read in display data or clear the line
        if (yy == (SCREEN_ROWS - 1))
            memset (buf, 0, SCREEN_WIDTH);
        else
            read_block (0, yy + 1, SCREEN_WIDTH, buf, 0x00, mode);
        
        // Write data back to screen
        write_block (0, yy, SCREEN_WIDTH, buf, mode);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Reverse the display. We read all of the screen values, invert them and
/// then write them back.
///
/// @param [in] buffer A buffer to use for 8 lines of screen data.
///
void
ks0108b_screen_reverse (uint8_t *buf)
{
    uint8_t yy;

    // This is a reverse mode switch, do a logical inversion of the screen.
    for (yy = 0; yy < 8; yy++)	//loop for each page
    {
        // Read in display data and invert it
        read_block (0, yy, SCREEN_WIDTH, buf, 0x00, MODE_NORMAL);
        // Write inverted data back to screen
        write_block (0, yy, SCREEN_WIDTH, buf, MODE_REVERSE);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Sets/Draws a single column to the screen.
/// The column is read row wise where y is (y & 0xf8)
///
/// @param [in] x     The column to read.
/// @param [in] y_row The row to re-write (y % 8).
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
ks0108b_set_column (uint8_t x, uint8_t y_row, uint8_t data, uint8_t mask, uint8_t mode)
{
    uint16_t cs_select;

    // Set the y-position enable the display, write to both chips/pages
    // concurrently.
    set_y_position (y_row);              // Set row

    // Set the x-position chip select.
    if (x < 64)
        cs_select = CMD_CS2;
    else
        cs_select = CMD_CS1;

    // Set the column we are writing.
    ks0108b_write (CMD_COLUMN | cs_select | (x & 0x3f)); // Set column

    // Handle any buffer merging on the read.
    if ((mode & MODE_MODIFIER) != 0)
    {
        uint8_t screen_data;

        // Perform a dummy read to transfer to register
        ks0108b_read (CMD_READ | cs_select);

        // We need to perform a merge. Get the data from controller
        screen_data = ks0108b_read (CMD_READ | cs_select);

        // Apply any reverse setting; if the reverse bit is set then we
        // negate the data
        if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
            screen_data = ~screen_data;

        // Perform the merge
        if ((mode & MODE_OP_MASK) != 0)
            data = merge_column (data, screen_data, mode);

        // MODE_MERGE - Merge in the data in a copy mode
        data = (data & mask) | (screen_data & ~mask);

        // Reset the position ready to write the data.
        ks0108b_write (CMD_COLUMN | cs_select | (x & 0x3f));
    }

    // Apply any reverse setting; if the reverse bit is set then we
    // negate the data
    if ((mode & MODE_NORMAL_MASK) == MODE_REVERSE)
        data = ~data;

    // Write the data.
    ks0108b_write (CMD_WRITE | cs_select | data);
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
ks0108b_set_pixel (uint8_t x, uint8_t y, uint8_t mode)
{
    uint8_t mask;
    // The caller has requested a modification of the bit based on the screen
    // contents. Perform a read-modify-write operation to set the pixel to
    // the right value.
    //
    // Convert y to a row value.
    // Set data to pixel to write by bit shifting 0 to the top bit.
    // Pass the mode in the call
    // 1 << y & 0x7
    mask = pgm_read_byte (&bit_shift_single_maskP[y & 0x7]);
    ks0108b_set_column (x, y >> 3, mask, mask, MODE_MERGE | mode);
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
ks0108b_vbitblt (uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t *data)
{
    // Define the sources.
    const uint8_t SOURCE_DATA_BYTE  = 0x00;  // Single data byte is the source
    const uint8_t SOURCE_DATA_PTR   = 0x01;  // Data pointer is source
    const uint8_t SOURCE_SERIAL     = 0x02;  // Serial is the source
    
    // Define the mixing operations.
    const uint8_t OPERATION_ALIGNED = 0x00;  // The merge is aligned
    const uint8_t OPERATION_TOP     = 0x01;  // The top line 
    const uint8_t OPERATION_MIDDLE  = 0x02;  // The bottom line
    const uint8_t OPERATION_BOTTOM  = 0x03;  // The middle line
    
    uint8_t source = SOURCE_SERIAL;     // Source of the data
    uint8_t operation = 0;              // The operation required.
    uint8_t page_rows;                  // The number of screen page rows
    uint8_t image_rows;                 // The number of image page rows
    uint8_t col;                        // The current column being processed.
    uint8_t row;                        // The current row being processed.
    uint8_t shift_top;                  // Shift to move to upper part of page.
    uint8_t shift_bot;                  // Shift to move to lower part of page.
    uint8_t mask_top;                   // Top line mask.
    uint8_t mask_bot;                   // Bottom line mask
    int offset;                         // Offset into the data

    // Calculate how much to shift the data bytes to line them up with the
    // pages. Set up the row flag if the pages are aligned.
    shift_top = y & 0x7;
    shift_bot = 8 - shift_top;          // Bottom shift of opposite of top

    // Number of pages(rows) the image occupies. We need to loop through all
    // of these, each gets pixels changed.
    page_rows = (uint8_t)(height + shift_top + 7) >> 3;      // Divide by 8
    image_rows = (uint8_t)(height + 7) >> 3;                 // Divide by 8

    if (height < 8)
    {
        // Mask for the row bits.
        mask_top = (1 << height) - 1;
        // mask_bot needs to have 0's for each pixel in the bottom row NOT
        // occupied by new image data
        mask_bot = mask_top >> shift_bot;
        // mask_top needs to have 0's for each pixel in the top row NOT occupied
        // by new image data
        mask_top = mask_top << shift_top;
    }
    else
    {
        mask_bot = 0xff;
        // mask_top needs to have 0's for each pixel in the top row NOT occupied
        // by new image data
        mask_top = mask_bot << shift_top;
        if (((height+y) & 7) != 0)
        {
            // mask_bot needs to have 0's for each pixel in the bottom row NOT
            // occupied by new image data
            mask_bot >>= (8 - ((height + y) & 7));
        }
    }

    // Determine the source of the data
    if (mode & MODE_FILL)
    {
        // Knock off the fill mode. 
        mode &= ~MODE_FILL;
        
        // Re-form the data that was passed. This would be better taken out
        // of the loop as it is static.
        data[0] = (data[0] << shift_top) | (data[0] >> shift_bot);
        source = SOURCE_DATA_BYTE;
    }
    // Source is the data pointer
    else if (data != NULL)
        source = SOURCE_DATA_PTR;
    
    // Loop through all page rows
    offset = 0;                         // Start at the beginning of the data.
    y >>= 3;
    for (row = 0; row < page_rows; row++)
    {
        uint8_t mask;                   // The mask to be applied to the data.

        // The basic operation is zero so use the shift_top to initialise.
        operation = shift_top;
            
        // Set up the operation.
        if (row == 0)
        {
            // TOP: If this is the first row, we need to mask off the blank
            // pixels at the top of the row (these pix have random data).
            // mask_top has shift blank pixels starting from LSB (LSB is the
            // top of the stripe) 
            mask = mask_top;
            if (operation != OPERATION_ALIGNED)
            {
                // Special case of height < 8 then we render as a bottom line
                // rather than top. 
                if (page_rows == 1)
                    operation = OPERATION_BOTTOM;
                else
                    operation = OPERATION_TOP;
                // This is a partial row so merge.
                mode |= MODE_MERGE;
            }
            // Special case of height < 8 then render as bottom line. 
            else if ((page_rows == 1) && ((mask & 0x80) == 0))
            {
                operation = OPERATION_BOTTOM;
                mode |= MODE_MERGE;
            }
        }
        else if (row == (uint8_t)(page_rows - 1))
        {
            // BOTTOM: If this is the last row, we need to maks off the blank
            // pixels at the bottom of the image. mask_bot has blank pixels
            // starting at MSB (MSB is the bottom of the stripe) 
            mask = mask_bot;
            
            // If the bottom mask does not extend to the bottom line then a
            // merge is required. 
            if ((mask & 0x80) == 0)
                mode |= MODE_MERGE;
            
            // If this is not aligned then determine the type of operation
            // required. 
            if (operation != OPERATION_ALIGNED)
            {
                // If we have an exact match of image rows to pages rows then
                // treat as a middle line. The other condition we have more
                // page rows than image rows so we perform a bottom line
                // operation. 
                if ((row != 0) && (image_rows == page_rows))
                    operation = OPERATION_MIDDLE;
                else
                    operation = OPERATION_BOTTOM;
            }
        }
        else  
        {
            // MIDDLE: Accept the whole image stripe
            mask = 0xff;
            if (operation != OPERATION_ALIGNED)
                operation = OPERATION_MIDDLE;
            // Do not need to force a merge in the middle of the line.
            mode &= ~MODE_MERGE;
        }
        
        // Loop for columns
        for (col = 0; col < width; col++)
        {
            uint8_t temp;               // The currently process column data
            
            if (source < SOURCE_DATA_PTR)
            {
                // Use the static data that was passed.
                temp = *data;
            }
            else if (source == SOURCE_DATA_PTR)
            {
                // Handle the data passed in 
                
                // Operation 1: Not aligned, process the top row only.
                if (operation == OPERATION_TOP)
                {
                    // Get the data for the first row for the bottom of the
                    // page. 
                    temp = data[col] << shift_top;
                }
                else
                {
                    // Get the data from the current position for the top of
                    // the page. 
                    temp = data[offset];
                    
                    if (operation > OPERATION_ALIGNED)
                    {
                        // Operation 2: Not aligned, look ahead. If we are
                        // mid row (not first or last) and non-aligned then
                        // look ahead and get the bottom of the column from
                        // the next byte that is in the RX buffer. This will
                        // be exactly 'width' bytes ahead. 
                        if (operation == OPERATION_MIDDLE)
                        {
                            // Get the data for the bottom from the next row.
                            temp >>= shift_bot;
                            temp |= data[offset + width] << shift_top;
                        }
                        // Operation 1: Not aligned. 
                        else
                            temp = (temp << shift_top) | (temp >> shift_bot);
                    }
                    
                    // Operation 3: Aligned data pass through without relocating.
                    // Move to the next.
                    offset++;
                }
            }
            else
            {
                // Handle the passed from the serial.
                
                // Operation 1: Not aligned, process the top row only.
                if (operation == OPERATION_TOP)
                {
                    temp = serial_peek (col) << shift_top;
                }
                else
                {
                    // Read a byte from the serial and align with the page
                    // boundary.
                    temp = serial_getc ();

                    if (operation > OPERATION_ALIGNED)
                    {
                        // Operation 2: Not aligned, look ahead. If we are
                        // mid row (not first or last) and non-aligned then
                        // look ahead and get the bottom of the column from
                        // the next byte that is in the RX buffer. This will
                        // be exactly 'width' bytes ahead. 
                        if (operation == OPERATION_MIDDLE)
                        {
                            // Read the value of the data ahead of us and
                            // shift into the correct position. 
                            temp >>= shift_bot;
                            temp |= serial_peek (width - 1) << shift_top;
                        }
                        else
                        {
                            // Bottom line.
                            temp = (temp << shift_top) | (temp >> shift_bot);
                        }
                    }
                }
            }
            // Write the data to the buffer
            draw_buffer[col] = temp;
        }
            
        // If NULL was passed for data, take it from the serial port. It is
        // necessary to have 2 rows of data, current and previous to do
        // bitblt since 0<width<128 (display is only 128 wide), we can use
        // the second 128 bytes in buffer to hold the previous row
        //
        // Read the row in (background image data). Skip the read if we are
        // writing a complete row and we are not mxing in any pixels. We
        // perform the read when (merge mode) || (first row shifted) || (last
        // row shifted).
        if ((mode & (MODE_OP_MASK|MODE_MERGE)) != 0)
        {
            // We need to perfom some mixing so read the data.
            read_block (x, y, width, draw_buffer, mask, mode);
        }
        // Write the data back.
        write_block (x, y, width, draw_buffer, mode & (MODE_OP_MASK | MODE_NORMAL));
        y++;
    }//row loop
    
    // Clean the serial if there is any pending data not consumed.
    if ((source == SOURCE_SERIAL) && (operation == OPERATION_MIDDLE))
        serial_flushc (width);
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
ks0108b_hline (uint8_t x, uint8_t y, uint8_t x1, uint8_t mode)
{
    uint8_t ii;
    uint8_t pixel_mask;
    uint8_t width;
    uint8_t width2;

    // Swap the bytes so that we can iterate
    if (x > x1)
        swap_bytes (x, x1);

#if 0
    // Iterate between the two y coordinates.
    while (x <= x1)
    {
        ks0108b_set_pixel (x++, y, mode);
    }
    return;
#endif

    // Calculate the pixel mask to use.
    //pixel_mask = 1 << (y & 7);          // Get pixel mask
    pixel_mask = pgm_read_byte (&bit_shift_single_maskP[y & 7]);
    y >>= 3;                            // Convert to column address

    // Write in 64 byte chunks as we do not have enough memory to do a
    // complete line. The limit is set by polygon fill which uses 64 bytes of
    // the draw buffer.
    width = (x1 - x) + 1;               // Calculate the width
    if (width > 64)
    {
        x1 = 64 - x;
        width2 = width - x1;
        width = x1;
    }
    else
        width2 = 0;

    // Iterate over both widths if defined.
    for (;;)
    {
        // Set up the buffer for a merge.
        for (ii = 0; ii < width; ii++)
            draw_buffer [SCREEN_HEIGHT + ii] = pixel_mask;

        // Read the block in with a modification and then write it back.
        read_block (x, y, width, &draw_buffer [SCREEN_HEIGHT], pixel_mask, mode | MODE_MERGE);
        write_block (x, y, width, &draw_buffer [SCREEN_HEIGHT], mode | MODE_MERGE);

        // See if there is another block to do.
        if (width2 == 0)
            break;

        // Another iteration to be performed. Adjust the width ready for the
        // next loop.
        x += width;                     // Change the x position.
        width = width2;                 // Assume new width.
        width2 = 0;                     // Reset width2 so we do not loop again.
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Draw a vertical line.
///
/// @param [in] x The first x-coordinate.
/// @param [in] y The first y-coordinate.
/// @param [in] y1 The 2nd y-coordinate.
/// @param [in] mode The drawing mode.
///
void
ks0108b_vline (uint8_t x, uint8_t y, uint8_t y1, uint8_t mode)
{
    uint8_t height;                     // Height of the line

    // Swap the bytes so that we can iterate
    if (y > y1)
        swap_bytes (y, y1);

#if 0
    // Iterate between the two y coordinates.
    while (y <= y1)
    {
        ks0108b_set_pixel (x, y++, mode);
    }
    return;
#endif

    height = (y1 - y) + 1;              // Calculate the height
    y1 = y & 7;                         // Get offset to start of block
    y >>= 3;                            // Get the column index

    if (y1 != 0)                        // Not on boundary??
    {
        uint8_t valid_mask;             // Mask for the valid bits to write.

        // Compute the valid mask i.e. 0xff << x1
        valid_mask = ~pgm_read_byte (&bit_shift_maskP[(uint8_t)(8 - y1)]);
        height += y1;                    // Add the sub-block start to width

        // Add any bottom mask if we have a short block
        if (height < 8)
        {
            // This is a short block, the data does not occupy the block.
            // Modify the mask to handle the right of the data.
            // i.e. valid_mask &= ~((1 << (8 - height)) - 1);
            valid_mask &= pgm_read_byte (&bit_shift_maskP[(uint8_t)(8 - height)]);
        }

        ks0108b_set_column (x, y++, valid_mask, valid_mask, mode | MODE_MERGE);

        // Adust the height now a column has been written.
        if (height <= 8)
            return;                     // Quit - nothing left to do
        height -= 8;
    }

    // Proecess the complete vertical blocks
    if ((y1 = (height >> 3)) > 0)
    {
        do
        {
            ks0108b_set_column (x, y++, 0xff, 0xff, mode);
        }
        while (--y1 > 0);
    }

    // Process any remaining block
    height &= 0x07;
    if (height > 0)
    {
        uint8_t valid_mask;             // Mask for the valid bits to write.

        // Create the mask of valid bits.
        valid_mask = pgm_read_byte (&bit_shift_maskP[(uint8_t)(8 - height)]);
        ks0108b_set_column (x, y, valid_mask, valid_mask, mode | MODE_MERGE);
    }
}
