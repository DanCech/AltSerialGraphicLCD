/* -*- c++ -*- ***************************************************************
 *
 *  System      : Serial GLCD
 *  Module      : Draw functions
 *  Object Name : $RCSfile: draw.c,v $
 *  Revision    : $Revision: 1.24 $
 *  Date        : $Date: 2015/05/31 21:05:23 $
 *  Author      : $Author: jon $
 *  Created By  : Jon Green
 *  Created     : Sun Apr 5 08:43:33 2015 Last Modified : <150531.2205>
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
#include <avr/pgmspace.h>

#include "glcd.h"

// Buffer used for bitblt and other draw operations.
uint8_t draw_buffer [SCREEN_MAX_WIDTH];

// Buffer used for line blitting; this is 8 pairs of coordinates.
uint8_t line_buffer [LINE_BUFFER_MAX];

// The current draw mode.
uint8_t drawing_mode;

/////////////////////////////////////////////////////////////////////////////
/// Change the current drawing mode.
///
/// @param [in] mode The new drawing mode.
///
void
draw_mode (uint8_t mode)
{
    drawing_mode = mode;
}

/////////////////////////////////////////////////////////////////////////////
/// line performs a Bresenhams line draw. This uses the buffer storage
/// so cannot be used with any operation that uses the buffer.
///
/// The function caches a block in the buffer to allow multiple joined
/// line draws to be perfomed in succession. This allows the line drawing
/// to be used with XOR type functions.
///
/// @param [in] x0 is the first x-coordinate to start drawing.
/// @param [in] y0 is the first y-coordinate to start drawing.
/// @param [in] x1 is the second x-coordinate to start drawing.
/// @param [in] y1 is the second y-coordinate to start drawing.
/// @param [in] s_r A bit mask of the drawer.
///                 0x01 draws or 0x00 erases a line
///                 0x40 Do not flush the data on completion.
///                 0x80 when bit is set then the last pixel is not
///                      drawn. used for polygons.
///
void
draw_line (uint8_t x, uint8_t y, uint8_t x1, uint8_t y1, uint8_t s_r)
{
    // Working variables.
    int16_t deltax;                     // Difference in x;
    int16_t deltay;                     // Difference in y;
    int8_t xinc;                        // X-coordinate increment.
    int8_t yinc;                        // X-coordinate increment.
    uint8_t ps_r;

    ps_r = ((~s_r ^ prefs_reverse) & MODE_NORMAL_MASK) | (s_r & MODE_MODIFIER);

    // Sort out tge x values.
    if (x1 >= x)                        // The x-values are increasing
    {
        xinc = 1;
        deltax = x1 - x;                // The difference between the x's
    }
    else                                // The x-values are decreasing
    {
        xinc = -1;
        deltax = x - x1;
    }

    // Sort out the y value
    if (y1 >= y)                       // The y-values are increasing
    {
        yinc = 1;
        deltay = y1 - y;
    }
    else                                // The y-values are decreasing
    {
        yinc = -1;
        deltay = y - y1;
    }

    // There is at least one x-value for every y-value. Handle the incrementing
    // and decremening separately.
    if (deltax >= deltay)
    {
        int16_t denominator;             // Denominator
        int16_t numerator;               // Numerator
        int16_t numadd;                  // Numerator to add.
        int16_t numpixels;               // Number of pixels to write.
        uint8_t xstart;                 // The x start position
        uint8_t written;                // Written the line

        // Calculate the Bresenham parameters
        denominator = deltax;           // The denominator
        numerator = deltax / 2;         // The numerator
        numadd = deltay;                // Increment of the numerator
        numpixels = deltax;             // There are more x-values than y-values

        // If we are skipping the last pixel the decrement the write length.
        if ((s_r & MODE_LINE_SKIP_LAST) == 0)
            numpixels++;

        xstart = x;
        written = numpixels - 1;

        // Write all of the pixels
        while (--numpixels >= 0)
        {
            numerator += numadd;        // Increase the numerator by the top of the fraction
            if (numerator >= denominator) // Check if numerator >= denominator
            {
                numerator -= denominator; // Calculate the new numerator value
                draw_hline (xstart, y, x, ps_r);

                y += yinc;              // Change the y as appropriate
                x += xinc;              // Increment x as required.

                xstart = x;             // New start position
                written = numpixels;    // Save the write position
            }
            else
                x += xinc;
        }

        // Make sure that any line fragment has been flushed.
        if (written != 0)
            draw_hline (xstart, y, x - xinc, ps_r);

    }
    // There is at least one y-value for every x-value
    else
    {
        int16_t denominator;             // Denominator
        int16_t numerator;               // Numerator
        int16_t numadd;                  // Numerator to add.
        int16_t numpixels;              // Number of pixels to write.
        uint8_t ystart;                 // The y start position
        uint8_t written;                // Written the line

        // Calculate the Bresenham parameters
        denominator = deltay;
        numerator = deltay / 2;
        numadd = deltax;
        numpixels = deltay;             // There are more y-values than x-values

        // If we are skipping the last pixel the decrement the write length.
        if ((s_r & MODE_LINE_SKIP_LAST) == 0)
            numpixels++;

        ystart = y;
        written = numpixels - 1;

        // Write all of the pixels
        while (--numpixels >= 0)
        {
            // Increase the numerator by the top of the fraction
            numerator += numadd;
            if (numerator >= denominator) // Check if numerator >= denominator
            {
                numerator -= denominator; // Calculate the new numerator value
                draw_vline (x, ystart, y, ps_r);

                x += xinc;              // Change the x as appropriate
                y += yinc;

                ystart = y;             // New start position
                written = numpixels;    // Save the write position
            }
            // Increment y.
            else
                y += yinc;
        }

        // Ensure that all pixels are written
        if (written != 0)
            draw_vline (x, ystart, y - yinc, ps_r);
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Draw multiple connected lines
///
/// @param [in] mode The drawing mode.
/// @param [in] data The list of x,y coordinates. The last y coordinate is
///                  marked with the top bit set to 0x80.
void
draw_lines (uint8_t s_r, uint8_t x, uint8_t y, uint8_t *data)
{
    // Cache line and do not draw endpoint
    s_r |= MODE_LINE_SKIP_LAST;

    // Iterate over the rest of the coordinate until we have finished.
    do
    {
        uint8_t x1, y1;

        if (data != NULL)
        {
            x1 = *data++;
            y1 = *data++;
        }
        else
        {
            x1 = serial_getc (); // Save x
            y1 = serial_getc (); // Save y
        }

        if ((y1 & 0x80) != 0)
        {
            y1 &= ~0x80;
            // Draw last point and flush cache.
            s_r &= ~MODE_LINE_SKIP_LAST;
        }

        // Do the best form of line draw that we can
        draw_line (x, y, x1, y1, s_r);
        x = x1;
        y = y1;
    }
    while ((s_r & MODE_LINE_SKIP_LAST) != 0);
}

//////////////////////////////////////////////////////////////////////////////
/// Draw a pixel. Confirm that the pixel is within bounds and then draw it.
///
/// @param [in] x The x-coordinate
/// @param [in] y The y-coordinate
/// @param [in] s_r The drawing mode.
void
draw_pixel (uint8_t x, uint8_t y, uint8_t s_r)
{
    if (x_valid(x) && y_valid(y))
    {
        // Compute the current mode based on the reverse preference.
        s_r = ((~s_r ^ prefs_reverse) & MODE_NORMAL_MASK) | (s_r & ~MODE_NORMAL_MASK);
        lcd_set_pixel (x, y, s_r);
    }
}

static void
clip_hline (int x0, int y0, int x1, uint8_t s_r)
{
    // Make sure y is on screen
    if (y_valid (y0))
    {
        // Swap the word if in the incorrect order.
        if (x0 > x1)
        {
            int temp;

            temp = x0;
            x0 = x1;
            x1 = temp;
        }

        // Ensure end point is on screen
        if (x1 >= 0)
        {
            // Ensure start point is on screen */
            if (x0 < x_dim)
            {
                // Ensure endpoint is not off screen
                if (x1 >= x_dim)
                    x1 = x_dim - 1;     // Clip end point to screen.
                if (x0 < 0)
                    x0 = 0;             // Clip start point to screen.
                draw_hline (x0, y0, x1, s_r);
            }
        }
    }
}

static void
clip_vline (int x0, int y0, int y1, uint8_t s_r)
{
    // Make sure y is on screen
    if (x_valid (x0))
    {
        // Swap the word if in the incorrect order.
        if (y0 > y1)
        {
            int temp;

            temp = y0;
            y0 = y1;
            y1 = temp;
        }

        // Ensure end point is on screen
        if (y1 >= 0)
        {
            // Ensure start point is on screen */
            if (y0 < y_dim)
            {
                // Ensure endpoint is not off screen
                if (y1 >= y_dim)
                    y1 = y_dim - 1;     // Clip end point to screen.
                if (y0 < 0)
                    y0 = 0;             // Clip start point to screen.
                draw_vline (x0, y0, y1, s_r);
            }
        }
    }
}

// Draws (s_r = 1) or erases (s_r = 0) a filled circle at x, y with radius r,
// using midpoint circle algorithm. For efficiency in drawing then the
// algorith draws staight line vertical and horizontal segments which degrade
// to a single pixel in the worst case.
//
// Noted that the Sparkfun circle algorithm is a little easier however the
// circle quality is dubious at a small radius so we keep the existing one
// used by Jennifer Holt.
static void
_draw_circle (uint8_t xin, uint8_t yin, uint8_t xgap, uint8_t ygap, uint8_t rin, uint8_t s_r)
{
    int r = rin;
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;
    int x0 = xin;
    int y0 = yin;
    int xstart;

    // Sort out the drawing mode
    s_r = ((~s_r ^ prefs_reverse) & MODE_NORMAL_MASK) | (s_r & ~MODE_NORMAL_MASK);
    xstart = x;
    while(x < y)
    {
        if(f >= 0)
        {
            if (xstart == 0)
            {
                // For the first co-ordinate x = 0 then coersce the lines as
                // they are abutting each other and we do not want them to
                // overlap in an xor operation which would cancel out 2 draw operations.
                clip_hline (x0 - x, y0 - y,        x0 + x + xgap, s_r);  // 1/8 + 8/8
                clip_hline (x0 - x, y0 + y + ygap, x0 + x + xgap, s_r);  // 4/8 + 5/8

                if (s_r & MODE_FILL)
                {
                    int ii;

                    // Fill the lines
                    for (ii = y0 - x; ii <= y0 + x + ygap; ii++)
                        clip_hline (x0 - y, ii, x0 + y + xgap, s_r);
                }
                else
                {
                    // Draw the lines
                    clip_vline (x0 + y + xgap, y0 + x + ygap, y0 - x, s_r);  // 2/8 + 3/8
                    clip_vline (x0 - y,        y0 + x + ygap, y0 - x, s_r);  // 6/8 + 7/8
                }
            }
            else
            {
                if (s_r & MODE_FILL)
                {
                    // Fill the lines
                    clip_hline (x0 - x, y0 - y,        x0 + x + xgap, s_r);  // 1/8 + 8/8
                    clip_hline (x0 - x, y0 + y + ygap, x0 + x + xgap, s_r);  // 4/8 + 5/8

                    int ii;
                    for (ii = y0 - x; ii <= y0 - xstart; ii++)
                        clip_hline (x0 - y, ii, x0 + y + xgap, s_r);    // 2/8 + 7/8
                    for (ii = y0 + xstart + ygap; ii <= y0 + x + ygap; ii++)
                        clip_hline (x0 - y, ii, x0 + y + xgap, s_r);    // 3/8 + 6/8
                }
                else
                {
                    // For the rest of the lines then draw each octant.
                    clip_hline (x0 + xstart + xgap, y0 - y,        x0 + x + xgap, s_r);  // 1/8
                    clip_hline (x0 + xstart + xgap, y0 + y + ygap, x0 + x + xgap, s_r);  // 4/8
                    clip_hline (x0 - xstart,        y0 + y + ygap, x0 - x,        s_r);  // 5/8
                    clip_hline (x0 - xstart,        y0 - y,        x0 - x,        s_r);  // 8/8

                    clip_vline (x0 + y + xgap, y0 + xstart + ygap, y0 + x + ygap, s_r);  // 3/8
                    clip_vline (x0 + y + xgap, y0 - xstart,        y0 - x,        s_r);  // 2/8
                    clip_vline (x0 - y,        y0 - xstart,        y0 - x,        s_r);  // 7/8
                    clip_vline (x0 - y,        y0 + xstart + ygap, y0 + x + ygap, s_r);  // 6/8
                }
            }
            y--;
            ddF_y += 2;
            f += ddF_y;
            x++;
            xstart = x;
        }
        else
            x++;
        ddF_x += 2;
        f += ddF_x;
    }

    // Handle the last round only if the 2 coordinates are the same. When the
    // coordinates are different then over-drawing occurs which is why we skip.
    if (x == y)
    {
        if (s_r & MODE_FILL)
        {
            // Fill the lines.
            clip_hline (x0 - x, y0 + y + ygap, x0 + x + xgap, s_r); // 3/8 + 5/8
            clip_hline (x0 - x, y0 - y,        x0 + x + xgap, s_r); // 1/8 + 7/8
        }
        else
        {
            // Draw the lines
            clip_hline (x0 + xstart + xgap, y0 + y + ygap, x0 + x + xgap, s_r);    // 3/8
            clip_hline (x0 + xstart + xgap, y0 - y,        x0 + x + xgap, s_r);    // 1/8
            clip_hline (x0 - xstart,        y0 + y + ygap, x0 - x,        s_r);    // 5/8
            clip_hline (x0 - xstart,        y0 - y,        x0 - x,        s_r);    // 7/8
        }
        if (xstart < x)
        {
            // In this case then the line is longer than 1 pixel, ensure that
            // the x is not re-drawn.
            if (s_r & MODE_FILL)
            {
                // Fill the lines
                clip_hline (x0 - y, y0 + xstart + ygap, x0 + y + xgap, s_r);
                clip_hline (x0 - y, y0 - xstart,        x0 + y + xgap, s_r);
            }
            else
            {
                // Draw the lines
                clip_vline (x0 + y + xgap, y0 + xstart + ygap, y0 + xstart + ygap, s_r);
                clip_vline (x0 - y,        y0 + xstart + ygap, y0 + xstart + ygap, s_r);
                clip_vline (x0 + y + xgap, y0 - xstart,        y0 - xstart,        s_r);  // 2/8
                clip_vline (x0 - y,        y0 - xstart,        y0 - xstart,        s_r);
            }
        }
    }
}

// Draws (s_r = 1) or erases (s_r = 0) a filled circle at x, y with radius r,
// using midpoint circle algorithm. For efficiency in drawing then the
// algorith draws staight line vertical and horizontal segments which degrade
// to a single pixel in the worst case.
//
// Noted that the Sparkfun circle algorithm is a little easier however the
// circle quality is dubious at a small radius so we keep the existing one
// used by Jennifer Holt.
void
draw_circle (uint8_t xin, uint8_t yin, uint8_t rin, uint8_t s_r)
{
    // Invoke the _draw_circle function
    _draw_circle (xin, yin, 0, 0, rin, s_r);
}

/**
 * Draws a rounded corner box. The box is described by a diagonal line from x, y1 to x2, y2
 *
 * @param x1 The upper left x-coordinate.
 * @param y1 The upper left y-coordingate.
 * @param x2 The lower right x-coordinate.
 * @param y2 The lower right y-coordinate
 * @param radius The radius of the corner.
 * @param s_r The mode to draw the line
 */
void
draw_rbox (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t radius, uint8_t s_r)
{
    uint8_t radius2 = radius << 1;
    uint8_t xdiff;
    uint8_t ydiff;
    uint8_t diff;

    // Sort the x and y coordinates into ascending order.
    if (x1 > x2)
        swap_bytes (x1, x2);
    if (y1 > y2)
        swap_bytes (y1, y2);

    // Determine the maximum radius we can deal with.
    xdiff = x2 - x1;
    ydiff = y2 - y1;
    diff = xdiff;
    if (ydiff < xdiff)
        diff = ydiff;

    // Radius is larger than the box, reduce the radius to fit the box.
    if (radius2 > diff)
    {
        radius = diff >> 1;
        radius2 = diff;
    }

    // Invoke the _draw_circle function
    _draw_circle (x1 + radius, y1 + radius,
                  xdiff - radius2, ydiff - radius2,
                  radius, s_r);
}

///
/// Fill a convex polygons defined in a clockwise direction. The algorithm
/// will cope with concave shapes in the y-direction but does not cope with
/// all concave shapes in the x direction.
///
/// The algorithm uses besenham's line drawing algorithm and creates vertical
/// scal lines between the top and bottom of the polygone. Provided that the
/// vertical scan line is contained within the vertical scan line then the
/// shape will fill correctly. What this means is:
///
/// @    +--+  +--+
/// @    |  |  |  |
/// @ +--+  +--+  +--+
/// @ |              |    OK all vertical scan lines connected.
/// @ +--+  +--+  +--+
/// @    |  |  |  |
/// @    +--+  +--+
/// @
/// @ +----------+
/// @ |  +-------+
/// @ |  |
/// @ |  +-----+         OK because of clockwise scan.
/// @ |  +-----+
/// @ |  |
/// @ |  +-------+
/// @ +----------+
/// @
/// @ $---------+
/// @ +-------@ |        FAILS starting at '$' because cannot resolve convex.
/// @         | |        OK starting at '@' because can resolve convex
/// @ +-------+ |
/// @ +---------+
///
/// The algorithm works by determining the direction of the line and operates
/// in clockwise direction. A line is drawn between each successive point and
/// the y-value is saved in the buffer[x] if it is already empty (0xff). If
/// the buffer[x] has a value then a vertical line is rendered between the
/// x,y position of the current line and the x,buffer[x] stored position.
///
/// The libne drawing renders the whole line EXCEPT the last point of the
/// line to ensiure that points are not duplicated. Special handling is
/// required at the points where the line changes direction from forward
/// moving to backward moving. At forward/backward transition (because only 1
/// point is processed) then an extra plot is introduced this means that if
/// the buffer[x] is empty the y-position is saved or when set to a valid
/// y-value then a line is drawn.
///
/// A line moves in the forward direction if (x>x1) or ((x==x1) & (y<y1)). A
/// line moves in a backward direction if (x<x1) or ((x==x1) & (y>y1)).
///
/// At the end of the polygon fill we check to ensure that there is not a
/// residual draw in the buffer which has not been consumed, if there is then
/// we need a pixel at this location.
///
/// @param [in] s_r The fill colour
/// @param [in] x The first and last x-coordinate
/// @param [in] y The first and last y-coordinate
///
static void
_fill_polygon (uint8_t s_r, uint8_t x, uint8_t y)
{
    // Cached y coordinates.
    uint8_t write_ystart;               // The writing start y-coordinate
    uint8_t write_yend;                 // The writing end y-coordinate

    // Working variables.
    uint8_t last_coord = 0;             // The last coordinate.
    uint8_t xfirst, yfirst;             // Working x,y coordinates
    int8_t direction = -1;              // Current Direction
    int8_t direction_changed = 0;       // Direction has changed.

    // Initialise our line buffer
    write_ystart = y;
    write_yend = y;
    if (y_valid(y))
        draw_buffer[y] = 0xff;
    xfirst = x;
    yfirst = y;

    do
    {
        uint8_t x1, y1;                 // Working x,y coordinates
        int16_t deltax;                 // Difference in x;
        int16_t deltay;                 // Difference in y;
        int8_t xinc;                    // X-coordinate increment.
        int8_t yinc;                    // X-coordinate increment.

        // Get the end marker out of the last coordinate and normalise
        if (last_coord == 0)
        {
            // Read data from serial.
            x1 = serial_getc();
            y1 = serial_getc();

            // Handle the end of the polygon and strip out the signalling.
            last_coord = y1 & 0x80;
            y1 &= ~0x80;
        }
        else
        {
            // We have processed the last coordinate. Draw a line from the
            // last coordinate to the first coordinate.
            x1 = xfirst;
            y1 = yfirst;
            yfirst |= 0x80;             // mark as end the loop
        }

        // Horizontal lines cause a problem for concave shapes when filling
        // with XOR as we have to be very careful that we do not over-draw
        // any line as this causes an anomaly in the fill. Process the line
        // segment immediately and effectivelly reduce the line to a point.
        // We skip the direction processing and will effectively carry the
        // direction change to any next line segment.
        //
        // Ignore all horizontal lines as the x coordinates will be repeated
        // on another coordinate. Sort out the y values.
        if (y == y1)
        {
            uint8_t xstart;
            uint8_t xend;
            uint8_t buffer = draw_buffer[y];

            // Throw out a coincidental point.
            if (x == x1)
                continue;               // This is the same point, discard.

            // Determine the direction of the line from x
            if (x > x1)
            {
                // Going left, this is backwards i.e. (x1,y) <--- (x,y)

                // If the direction is reverse then do not do anything as the
                // next line segment will process the start of line.
                if (direction <= 0)
                    goto skip_line;

                // Otherwise draw the line. If the draw_buffer is defined
                // then advance the draw buffer position to the end point.
                if ((buffer != 0xff) && (buffer >= x))
                {
                    xstart = buffer;
                    draw_buffer[y] = 0xff /* x1*/;
                }
                else
                    xstart = x;
                xend = x1 + 1;
            }
            else
            {
                // Going right, this is forwards. i.e. (x,y) ---> (x1,y)

                // If the direction is currently forward then do not do
                // anything as the next line segment will process the line.
                if (direction != 0)
                    goto skip_line;

                // Otherwise draw the line. If the draw_buffer is defined
                // then advance the draw buffer position to the end point.
                if ((buffer != 0xff) && (buffer <= x))
                {
                    xstart = buffer;
                    draw_buffer[y] = 0xff /* x1*/;
                }
                else
                    xstart = x;
                xend = x1 - 1;
            }
            draw_hline (xstart, y, xend, s_r);

            // Advance the point.
skip_line:  x = x1;                     // Move the x to the next point.
            continue;                   // Skip any drawing, not processed.
        }
        else if (y1 > y)                // The y-values are increasing
        {
            yinc = 1;
            deltay = y1 - y;            // The difference between the y's
            // If y is increasing downwards then grow the buffer.
            while ((y1 > write_yend) && (write_yend < y_dim - 1))
                draw_buffer [++write_yend] = 0xff;

            // Going down, this is forwards
            if (direction >= 0)
                direction_changed = direction ^ 1;
            direction = 1;
        }
        else                            // The y-values are decreasing
        {
            yinc = -1;
            deltay = y - y1;
            // If the y is decreasing upwards then grow the buffer
            while ((y1 < write_ystart) && (write_ystart > 0))
                draw_buffer [--write_ystart] = 0xff;

            // Going up, this is backwards
            if (direction >= 0)
                direction_changed = direction ^ 0;
            direction = 0;
        }

        // Sort out the x values
        if (x1 >= x)                       // The x-values are increasing
        {
            if (x1 == x)
                xinc = 0;
            else
                xinc = 1;
            deltax = x1 - x;
        }
        else                                // The y-values are decreasing
        {
            xinc = -1;
            deltax = x - x1;
        }

        /* If the direction has changed then process the last pixel we did
         * not plot. We need some special handling because we only visit this
         * pixel once. If draw_the buffer[y] is not initialised then write
         * our current y value, this is the end of the last line. if the
         * buffer[y] is valid then we need to draw a line to the x position
         * in the buffer. */
        if (direction_changed)
        {
            uint8_t buffer = draw_buffer[y];

            // Handle the special case of internal and corners with care, we
            // do not want to promote drawing for an internal corner.
            // On internal corners then do not process.

            // Going forwards
            if (direction == 1)
            {
                if (buffer == 0xff)
                    draw_buffer[y] = x;
                else if (buffer > x)
                {
                    draw_hline (x+1, y, buffer, s_r);
                    draw_buffer[y] = 0xff;
                }
            }
            // Going backwards
            else
            {
                if (buffer == 0xff)
                    draw_buffer[y] = x;
                else if (buffer < x)
                {
                    draw_hline (x-1, y, buffer, s_r);
                    draw_buffer[y] = 0xff;
                }
            }
        }

        // There is at least one x-value for every y-value. Handle the incrementing
        // and decremening separately.
        if (deltax >= deltay)
        {
            int16_t denominator;             // Denominator
            int16_t numerator;               // Numerator
            int16_t numadd;                  // Numerator to add.
            int16_t numpixels;               // Number of pixels to write.
            uint8_t xstart;             // X start point

            // Calculate the Bresenham parameters
            denominator = deltax;           // The denominator
            numerator = deltax / 2;         // The numerator
            numadd = deltay;                // Increment of the numerator
            numpixels = deltax;             // There are more x-values than y-values -1
            xstart = x;                 // Save the start point

            // Write all of the pixels EXCLUDING the last pixel of the line.
            while (--numpixels >= 0)
            {
                numerator += numadd;          // Increase the numerator by the top of the fraction
                if (numerator >= denominator) // Check if numerator >= denominator
                {
                    uint8_t buffer;

                    // If we are going backwards with a positive increment or
                    // going forwards with a negative increment then save the
                    // start x position for plotting.
                    if (((direction == 1) && (xinc > 0)) ||
                        ((direction == 0) && (xinc < 0)))
                        xstart = x;

                    // This is the only position where y changes. Flush out
                    // the last y change before we increment it.
                    buffer = draw_buffer[y];
                    if (buffer != 0xff)
                    {
                        // Draw the horizontal line to fill the column and
                        // clear the y buffer
                        draw_hline (xstart, y, buffer, s_r);
                        xstart = 0xff;
                    }
                    draw_buffer[y] = xstart;

                    numerator -= denominator; // Calculate the new numerator value
                    y += yinc;                // Change the y as appropriate
                    x += xinc;                // Change the x for the nxt loop
                    xstart = x;               // Save the start position.
                }
                else
                {
                    // Advance the x-coordinate
                    x += xinc;
                }
            }

            // Catch any residue horizontal transition that has not been
            // recorded. This occurs for convex shapes where x is advanced
            // without y.
            if (xstart != x)
            {
                uint8_t buffer = draw_buffer[y];

                // This is the only position where y changes. Flush out
                // the last y change before we increment it.
                if (buffer != 0xff)
                {
                    // Draw the vertical line to fill the column and
                    // clear the y buffer
                    draw_hline (xstart, y, buffer, s_r);
                    xstart = 0xff;
                }
                draw_buffer[y] = xstart;
            }
        }
        // There is at least one y-value for every x-value
        else
        {
            int16_t denominator;             // Denominator
            int16_t numerator;               // Numerator
            int16_t numadd;                  // Numerator to add.
            int16_t numpixels;               // Number of pixels to write.

            // Calculate the Bresenham parameters
            denominator = deltay;
            numerator = deltay / 2;
            numadd = deltax;
            numpixels = deltay;             // There are more y-values than x-values

            // Write all of the pixels EXCLUDING the last pixel.
            while (--numpixels >= 0)
            {
                uint8_t buffer = draw_buffer[y];

                // We are about to increment y and move to the next column.
                // Either save the point or draw the point.
                if (buffer != 0xff)
                {
                    draw_hline (x, y, buffer, s_r);
                    buffer = 0xff;
                }
                else
                {
                    buffer = x;
                }
                draw_buffer[y] = buffer;

                // Increment x if necessary.
                numerator += numadd;          // Increase the numerator by the top of the fraction
                if (numerator >= denominator) // Check if numerator >= denominator
                {
                    numerator -= denominator; // Calculate the new numerator value
                    x += xinc;                // Change the x as appropriate
                }

                // Increment y.
                y += yinc;
            }
        }

        x = x1;
        y = y1;
    }
    while ((yfirst & 0x80) == 0);
    yfirst &= ~0x80;

    // Flush out any first pixel
    if (draw_buffer[yfirst] != 0xff)
    {
        draw_hline (xfirst, yfirst, draw_buffer[yfirst], s_r);
    }
}

//////////////////////////////////////////////////////////////////////////////
/// Draw Polygon.
///
/// This draws a polygon which commences from x, y and draws a line to the next
/// point defined by the next (x, y) pair received from the data. The last
/// point of the line is defined by setting the last top bit of the <x>
/// coordinate to 0x80. The last coordinate is connected to the first
/// coordinate. This is essentially the same as draw multi-line which does not
/// close the polygon
///
/// The line colour is defined by the s_r argument.
///
/// @param [in] mode The drawing mode.
/// @param [in] data The list of x,y coordinates. The last y coordinate is
///                  marked with the top bit set to 0x80.
void
draw_polygon (uint8_t s_r, uint8_t x, uint8_t y, uint8_t *data)
{
    if (s_r & MODE_FILL)
    {
        // Correct the reverse flag
        s_r = ((~s_r ^ prefs_reverse) & MODE_NORMAL_MASK) | (s_r & ~MODE_NORMAL_MASK);
        // Invoke the fill
        _fill_polygon (s_r, x, y);
    }
    else
    {
        uint8_t x1, y1;

        // Cache line and do not draw endpoint
        s_r |= MODE_LINE_SKIP_LAST;

        // Prepare x and y for the loop
        x1 = x;
        y1 = y;

        // Iterate over the rest of the coordinate until we have finished.
        do
        {
            uint8_t x2, y2;

            if (data != NULL)
            {
                x2 = *data++;
                y2 = *data++;
            }
            else
            {
                x2 = serial_getc (); // Save x
                y2 = serial_getc (); // Save y
            }

            // Draw the line
            draw_line (x1, y1, x2, y2 & 0x7f, s_r);
            x1 = x2;
            y1 = y2;
        }
        while ((y1 & 0x80) == 0);

        // Join up to the start position.
        draw_line (x1, y1 & 0x7f, x, y, s_r);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Draws a filled box on the screen. The box is described by a diagonal line
// from x, y1 to x2, y2. The block is filled with byte data (describes a
// vertical row of 8 pixels, use 0x00 to clear the block, 0xFF to fill it,
// etc.)
void
fill_vbox (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t data)
{
    uint8_t width;
    uint8_t height;

    // Get the top left corner of the block in x1, y1, Get the width and
    // height of the block in the bitblt array.
    if (x1 > x2)
        swap_bytes (x1, x2);
    width = 1 + x2 - x1;         // Blit Width

    if (y1 > y2)
        swap_bytes (y1, y2);
    height = 1 + y2 - y1;         // Blit Height

    // Use erase mode of bitblt to draw the block.
    lcd_vbitblt (x1, y1, width, height, MODE_FILL|prefs_reverse, &data);
}

/////////////////////////////////////////////////////////////////////////////
// Draws a filled box on the screen. The box is described by a diagonal line
// from x, y1 to x2, y2. The block is filled according to the mode.
void
fill_box (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t s_r)
{
    uint8_t width;
    uint8_t height;
    uint8_t data = 0xff;

    // Correct the reverse flag
    s_r = ((~s_r ^ prefs_reverse) & MODE_NORMAL_MASK) | (s_r & ~MODE_NORMAL_MASK);

    // Get the top left corner of the block in x1, y1, Get the width and
    // height of the block in the bitblt array.
    if (x1 > x2)
        swap_bytes (x1, x2);
    width = 1 + x2 - x1;                // Blit Width

    if (y1 > y2)
        swap_bytes (y1, y2);
    height = 1 + y2 - y1;               // Blit Height

    // Use erase mode of bitblt to draw the block.
    lcd_vbitblt (x1, y1, width, height, s_r | MODE_FILL, &data);
}

/**
 * Draws a box. The box is described by a diagonal line from x, y1 to x2, y2
 *
 * @param x1 The upper left x-coordinate.
 * @param y1 The upper left y-coordingate.
 * @param x2 The lower right x-coordinate.
 * @param y2 The lower right y-coordinate
 * @param s_r The mode to draw the line
 */
void
draw_box (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t s_r)
{
    if (s_r & MODE_FILL)
    {
        // Invoke the fill
        fill_box (x1, y1, x2, y2, s_r);
    }
    else
    {
        // Correct the reverse flag and clear any line modes that have been set.
        s_r = ((~s_r ^ prefs_reverse) & MODE_NORMAL_MASK) | (s_r & ~(MODE_LINE_MASK|MODE_NORMAL_MASK));

        // Do not over-write the corner pixels multiple times. Detect a box
        // of width of 2 which requires 2 lines not 4.

        // Swap the x coordinate if necessary
        if (x1 > x2)
            swap_bytes (x1, x2);
        // Swap the y coordinate if necessary
        if (y1 > y2)
            swap_bytes (y1, y2);

        // Draw a box in a clockwise direction and chain the lines.
        draw_hline (x1, y1, x2-1, s_r);     // Top horizontal.
        draw_vline (x2, y1, y2-1, s_r);     // Right vertical.
        draw_hline (x2, y2, x1+1, s_r);     // Bottom horizontal.
        draw_vline (x1, y2, y1+1, s_r);     // Left vertical.
    }
}

/////////////////////////////////////////////////////////////////////////////
// Erases a block from the screen. The box is described by a diagonal line
// from x, y1 to x2, y2. The block is filled according to the mode.
void
erase_box (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    fill_box (x1, y1, x2, y2, ~prefs_reverse & MODE_NORMAL_MASK);
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
draw_vbitblt (uint8_t x, uint8_t y, uint8_t s_r, uint8_t *data)
{
    uint8_t width;                      // Width of the bitmap
    uint8_t height;                     // Height of the bitmap

    s_r = ((~s_r ^ prefs_reverse) & MODE_NORMAL_MASK) | (s_r & ~(MODE_LINE_MASK|MODE_NORMAL_MASK));

    // Get the width and the height from the data stream.
    if (data == NULL)
    {
        width = serial_getc();
        height = serial_getc();
    }
    else
    {
        width = *data++;
        height = *data++;
    }

    // Make sure we have legal dimensions otherwise discard the data.
    if ((height < 1) || (height > y_dim) ||
        (width < 1) || (width > x_dim))
    {
        // If we are reading from serial then consume all of the content from
        // the serial input.
        if (data == NULL)
        {
            uint8_t row, col;

            // Iterate over all of the data that's coming in.
            for (row = 0; row < height; row++)
                for (col = 0; col < width; col++)
                    serial_getc ();
        }

        // Quit the command there is an error.
        return;
    }

    // Invoke the screen driver to perform the bitblt operation.
    ((vfunc_iiiiip_t)(pgm_read_word(&functabP [F_DRV_VBITBLT])))(x, y, width, height, s_r, data);
}
