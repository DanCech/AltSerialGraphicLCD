// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : Alternative Serial Graphic LCD Firmware
//  Module        : Test the different functions
//  Object Name   : $RCSfile: AltSerialGraphicLCDTest.ino,v $
//  Revision      : $Revision: 1.13 $
//  Date          : $Date: 2015/07/05 21:10:36 $
//  Author        : $Author: jon $
//  Created By    : Jon Green
//  Created       : Sat May 23 11:46:23 2015
//  Last Modified : <150705.2050>
//
//  Description   : This program exercises the serial graphics library API
//
//  Notes         : Assume a Arduino UNO (AVR 328)
//
//  History
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Jon Green.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////

#include <avr/pgmspace.h>
#include <AltSerialGraphicLCD.h>
#include <SoftwareSerial.h>

// This test code was created for both the 128x64 and the 160x128 pixel LCD.
// The function will determine the display size from the screen.

// Define the TX and RX pins used to connect the screen. Change these two pin
// values to whichever pins you wish to use (RX, TX).
// #define SERIAL_TX_DPIN    3
// #define SERIAL_RX_DPIN    2
#define SERIAL_TX_DPIN   12
#define SERIAL_RX_DPIN   10

// Define the common strings we are using.
static const char STR_NAND[]    PROGMEM = "NAND";
static const char STR_NORMAL[]  PROGMEM = "NORMAL";
static const char STR_OR[]      PROGMEM = "OR";
static const char STR_REVERSE[] PROGMEM = "REVERSE";
static const char STR_XOR[]     PROGMEM = "XOR";

#define pstr_t  const char PROGMEM *
#define fchar_t const __FlashStringHelper *

// Initialize an instance of the SoftwareSerial library
SoftwareSerial serial (SERIAL_RX_DPIN, SERIAL_TX_DPIN);

// Create an instance of the LCD class named LCD. We will use this instance
// to call all the subsequent LCD functions. The instance is called with a
// reference to the software serial object.
GLCD lcd(serial);

// The test state machine number. This keeps track of the test that we are
// running,
static int test_no;

//////////////////////////////////////////////////////////////////////////////
// Initialisation method.
void
setup()
{
    // Start the Software serial library we run at 115200 by default.
    serial.begin(115200);

    // Reset the screen. As soon as it is reset then we can use it.
    lcd.reset();

    // Reset the test number for the purposes of running the tests.
    test_no = 0;

    // Leave the splash screen showing for 4 seconds.
    delay (4000);
}

//////////////////////////////////////////////////////////////////////////////
/// Iterate through the tests.
void
loop()
{
    switch (test_no)
    {
    case 0:
        glcdDrawFrontPage();
        //test_no = 12 - 1; /* shortcut to test minus 1 */
        break;

    case 1:
        glcdDrawFrontPageProportional();
        //test_no = 1000;
        break;

    case 2:
        pixelTest();
        break;

    case 3:
        glcdDrawLine();
        break;

    case 4:
        glcdTestBox();
        break;

    case 5:
        glcdTestCircle();
        break;

    case 6:
        glcdTestRoundBox();
        break;

    case 7:
        glcdTestPolygon();
        break;

    case 8:
        glcdTestLine();
        break;

    case 9:
        glcdTestDrawLines();
        break;

    case 10:
        resetTest();
        break;

    case 11:
        glcdTestDrawSprite();
        break;

    case 12:
        glcdTestDrawSprite2();
        break;

    case 13:
        glcdComplexDrawTest();
        break;

    case 14:
        glcdTestBitblt();
        break;

    case 15:
        glcdCharTest();
        break;

    case 16:
        glcdTestFillPoly();
        break;

    case 17:
        glcdDemo();
        break;

    default:
        test_no = -1;
        break;
    }
    test_no++;
}

#if 0
//////////////////////////////////////////////////////////////////////////////
// Print the string in the centred on x
static void
centreString (uint8_t y, char *s)
{
    uint8_t x;

    x = strlen (s);
    x = (lcd.xdim - (x * 6)) / 2;
    lcd.setXY (x, y);
    lcd.putstr (s);
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Centre a string on the line given the y-coordinate.
static void
centreString_P (uint8_t y, const char PROGMEM *s)
{
    uint8_t x;

    x = strlen_P (s);
    x = (lcd.xdim - (x * 6)) / 2;
    lcd.setXY (x, y);
    lcd.putstr_P (s);
}

/////////////////////////////////////////////////////////////////////////////
/// Draw a countdown marker on the screen in the bottom left. Countdown for 5
/// seconds.
static void
countdown (void)
{
    uint8_t ii;
    static const uint8_t counter = 5;
    static const uint8_t width = 1;

    lcd.drawBox (0, lcd.ydim - counter, width, lcd.ydim - 1, GLCD_MODE_FILL|GLCD_MODE_XOR);
    for (ii = counter; ii > 0; ii--)
    {
        uint8_t y;

        delay (1000);
        y = lcd.ydim - ii;
        lcd.drawLine (0, y, width, y, GLCD_MODE_XOR);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Test the reset function recovery
void
resetTest ()
{
    static const char title_name[]    PROGMEM = "Reset Test";
    static const char bitblt_name []  PROGMEM = "Bitblt Recovery";
    static const char polygon_name [] PROGMEM = "Polygon Recovery";
    static const char pass_msg1 []    PROGMEM = "Seems OK, got here OK";
    static const char pass_msg2 []    PROGMEM = "Lets call it a pass!";
    uint8_t ii;

    for (ii = 0; ii < 2; ii++)
    {
        const char PROGMEM *test_name;

        lcd.clearScreen ();
        centreString_P (0, title_name);
        if (ii == 0)
        {
            test_name = bitblt_name;
            //lcd.putstr (test_name);
            centreString_P (8, test_name);

            // Kick off a dummy bitblt operation and see if we can recover.
            lcd.put (GLCD_CHAR_CMD);
            lcd.put (GLCD_CMD_BITBLT);
            lcd.put (0);
            lcd.put (16);
            lcd.put (1);
            lcd.put (128);                      // Width
            lcd.put (48);                       // Height
            // Do not put the data - the screen will now hang up waiting for
            // the data
        }
        else
        {
            test_name = polygon_name;
            centreString_P (8, test_name);

            // Kick off a dummy polygon draw operation and see if we can recover.
            lcd.put (GLCD_CHAR_CMD);
            lcd.put (GLCD_CMD_DRAW_POLYGON);
            lcd.put (0);
            lcd.put (16);
            lcd.put (lcd.xdim - 1);
            lcd.put (lcd.ydim - 1);
            // Do not put the data - the screen will now hang up waiting for
            // the data
        }

        // Invoke a reset - this is what we are testing.
        lcd.reset();

        // Print on the new page it should be OK.
        centreString_P (0, title_name);
        centreString_P (8, test_name);
        centreString_P (24, pass_msg1);
        centreString_P (32, pass_msg2);

        countdown();
    }
}

//////////////////////////////////////////////////////////////////////////////
// Test writing pixels across the whole screen.
void
pixelTest (void)
{
    static const char title_name[] PROGMEM = "Pixel Test";
    int xmax = lcd.xdim;
    int ymax = lcd.ydim;
    int xx;
    int yy;

    lcd.clearScreen ();
    centreString_P (0, title_name);
    delay (2000);

    for (xx = 0; xx < xmax; xx++)
        for (yy = 0; yy < ymax; yy++)
            lcd.setPixel (xx, yy, GLCD_MODE_NORMAL);
    countdown();

    for (xx = 0; xx < xmax; xx++)
        for (yy = 0; yy < ymax; yy++)
            lcd.setPixel (xx, yy, GLCD_MODE_REVERSE);
    countdown();
}

//////////////////////////////////////////////////////////////////////////////
// Line Drawing : Exercise the line drawing.
void
glcdDrawLine (void)
{
    static const char title_name[] PROGMEM = "Line Drawing Test";
    uint8_t xmax = lcd.xdim;
    uint8_t ymax = lcd.ydim;
    uint8_t ii;

    lcd.clearScreen ();
    centreString_P (0, title_name);
    delay (2000);

    /* Draw black lines */
    for (ii = 0; ii < xmax; ii++)
    {
        lcd.drawLine (ii, 0, ii, (ii * ymax) / xmax, GLCD_MODE_NORMAL);
    }
    countdown();

    /* Insert white lines */
    for (ii = 0; ii < xmax; ii += 2)
    {
        lcd.drawLine (ii, 0, ii, (ii * ymax) / xmax, GLCD_MODE_REVERSE);
    }
    countdown();

    lcd.clearScreen ();
    // Draw lines from upper left down right side of rectangle
    lcd.drawLine (1, 1, (xmax/2) - 1, 1, GLCD_MODE_NORMAL);
    for (ii = 4; ii < ymax-1; ii += 4)
    {
        lcd.drawLine (1, 1, (xmax / 2) - 1, ii, GLCD_MODE_NORMAL);
    }
    // Draw lines from upper left down right side of rectangle
    lcd.drawLine (xmax-1, 1, (xmax/2) - 1, 1, GLCD_MODE_NORMAL);
    for (ii = 4; ii < ymax-1; ii += 4)
    {
        lcd.drawLine (xmax-1, 1, (xmax/2) - 1, ii, GLCD_MODE_NORMAL);
    }
    // Draw lines from lower left down left side of rectangle
    lcd.drawLine (1, ymax-2, (xmax/2) - 1, ymax-2, GLCD_MODE_NORMAL);
    for (ii = 4; ii < ymax - 2; ii += 4)
    {
        lcd.drawLine (1, ymax-2, (xmax/2) - 1, ii, GLCD_MODE_NORMAL);
    }
    // Draw lines from lower left down right side of rectangle
    lcd.drawLine (xmax-1, ymax-2, xmax/2, ymax-2, GLCD_MODE_NORMAL);
    for (ii = 4; ii < ymax-2; ii += 4)
    {
        lcd.drawLine (xmax-1, ymax-2, xmax/2, ii, GLCD_MODE_NORMAL);
    }
    countdown();
}

//////////////////////////////////////////////////////////////////////////////
// Helper for the box text
static void
glcdTestBoxHelper (uint8_t x_offset, uint8_t s_r, const char PROGMEM *str)
{
    uint8_t y_offset = 9;
    uint8_t width = lcd.xdim / 3;
    uint8_t height = lcd.ydim - 1 - (2 * y_offset);
    uint8_t ii;

    // Draw the box
    lcd.drawBox (x_offset + 1, y_offset,
                 x_offset + width - 1, y_offset + height - 1,
                 s_r);

    ii = strlen_P (str);
    lcd.setXY (x_offset + (width - (ii * 6)) / 2, lcd.ydim - y_offset);
    lcd.putstr_P (str);
}

//////////////////////////////////////////////////////////////////////////////
// Test the draw box function for drawing and filling in different modes.
void
glcdTestBox (void)
{
    static const char title_nrm_draw[] PROGMEM = "Draw Box Test";
    static const char title_rev_draw[] PROGMEM = "Rev Draw Box Test";
    static const char title_nrm_fill[] PROGMEM = "Fill Box Test";
    static const char title_rev_fill[] PROGMEM = "Rev Fill Box Test";
    const char PROGMEM *test_name;

    uint8_t ii;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 8; ii++)
    {
        uint8_t mode = 0;

        lcd.clearScreen ();

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            if ((ii & 4) == 0)
                test_name = title_rev_draw;
            else
                test_name = title_rev_fill;
        }
        else
        {
            if ((ii & 4) == 0)
                test_name = title_nrm_draw;
            else
                test_name = title_nrm_fill;
        }

        if (ii & 4)
            mode = GLCD_MODE_FILL;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_FILL|GLCD_MODE_NORMAL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestBoxHelper ((xdim / 3) * 0,     mode|GLCD_MODE_NORMAL,  STR_NORMAL);
            glcdTestBoxHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR,     STR_XOR);
            glcdTestBoxHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_REVERSE, STR_REVERSE);
        }
        else
        {
            glcdTestBoxHelper ((xdim / 3) * 0,     mode|GLCD_MODE_OR|GLCD_MODE_NORMAL,   STR_OR);
            glcdTestBoxHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR|GLCD_MODE_NORMAL,  STR_XOR);
            glcdTestBoxHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_NAND|GLCD_MODE_NORMAL, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Helper for the circle test
static void
glcdTestCircleHelper (uint8_t x_offset, uint8_t s_r, const char *str)
{
    uint8_t y_offset = 9;
    uint8_t width = lcd.xdim / 3;
    uint8_t height = lcd.ydim - 1 - (2 * y_offset);
    uint8_t ii;

    // Draw the box
    lcd.drawCircle (x_offset + width / 2,
                    y_offset + height / 2,
                    (width - 1) / 2,
                    s_r);

    ii = strlen_P (str);
    lcd.setXY (x_offset + (width - (ii * 6)) / 2, lcd.ydim - y_offset);
    lcd.putstr_P (str);
}

/////////////////////////////////////////////////////////////////////////////
// Test the draw circle function for drawing and filling in different modes.
void
glcdTestCircle (void)
{
    static const char title_nrm_draw[] PROGMEM = "Draw Circle Test";
    static const char title_rev_draw[] PROGMEM = "Rev Draw Circle Test";
    static const char title_nrm_fill[] PROGMEM = "Fill Circle Test";
    static const char title_rev_fill[] PROGMEM = "Rev Fill Circle Test";
    const char PROGMEM *test_name;

    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t ii;

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 8; ii++)
    {
        uint8_t mode = 0;

        lcd.clearScreen ();

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            if ((ii & 4) == 0)
                test_name = title_rev_draw;
            else
                test_name = title_rev_fill;
        }
        else
        {
            if ((ii & 4) == 0)
                test_name = title_nrm_draw;
            else
                test_name = title_nrm_fill;
        }

        if (ii & 4)
            mode = GLCD_MODE_FILL;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_FILL|GLCD_MODE_NORMAL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestCircleHelper ((xdim / 3) * 0,     mode|GLCD_MODE_NORMAL,  STR_NORMAL);
            glcdTestCircleHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR,     STR_XOR);
            glcdTestCircleHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_REVERSE, STR_REVERSE);
        }
        else
        {
            glcdTestCircleHelper ((xdim / 3) * 0,     mode|GLCD_MODE_OR|GLCD_MODE_NORMAL,   STR_OR);
            glcdTestCircleHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR|GLCD_MODE_NORMAL,  STR_XOR);
            glcdTestCircleHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_NAND|GLCD_MODE_NORMAL, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Helper for the rounded box text
static void
glcdTestRoundBoxHelper (uint8_t x_offset, uint8_t s_r, const char PROGMEM *str)
{
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t y_offset = 9;
    uint8_t width = xdim / 3;
    uint8_t height = ydim - 1 - (2 * y_offset);
    uint8_t ii;

    // Draw the box
    lcd.drawRoundedBox (x_offset + 1, y_offset,
                        x_offset + width - 1, y_offset + height - 1,
                        8, s_r);

    ii = strlen_P (str);
    lcd.setXY (x_offset + (width - (ii * 6)) / 2, ydim - y_offset);
    lcd.putstr_P (str);
}

/////////////////////////////////////////////////////////////////////////////
// Test the draw rounded box function for drawing and filling in different
// modes.
void
glcdTestRoundBox (void)
{
    static const char title_nrm_draw[] PROGMEM = "Draw R-Box Test";
    static const char title_rev_draw[] PROGMEM = "Rev Draw R-Box Test";
    static const char title_nrm_fill[] PROGMEM = "Fill R-Box Test";
    static const char title_rev_fill[] PROGMEM = "Rev Fill R-Box Test";
    const char PROGMEM *test_name;

    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t ii;

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 8; ii++)
    {
        uint8_t slen;
        uint8_t mode = 0;

        lcd.clearScreen ();

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            if ((ii & 4) == 0)
                test_name = title_rev_draw;
            else
                test_name = title_rev_fill;
        }
        else
        {
            if ((ii & 4) == 0)
                test_name = title_nrm_draw;
            else
                test_name = title_nrm_fill;
        }

        if (ii & 4)
            mode = GLCD_MODE_FILL;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_NORMAL|GLCD_MODE_FILL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestRoundBoxHelper ((xdim / 3) * 0,     mode|GLCD_MODE_NORMAL,  STR_NORMAL);
            glcdTestRoundBoxHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR,     STR_XOR);
            glcdTestRoundBoxHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_REVERSE, STR_REVERSE);
        }
        else
        {
            glcdTestRoundBoxHelper ((xdim / 3) * 0,     mode|GLCD_MODE_OR|GLCD_MODE_NORMAL,   STR_OR);
            glcdTestRoundBoxHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR|GLCD_MODE_NORMAL,  STR_XOR);
            glcdTestRoundBoxHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_NAND|GLCD_MODE_NORMAL, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Helper for the polygon test
static void
glcdTestPolygonHelper (uint8_t x_offset, uint8_t s_r, const char PROGMEM *str)
{
    uint8_t y_offset = 9;
    uint8_t width = lcd.xdim / 3;
    uint8_t height = lcd.ydim - 1 - (2 * y_offset);
    uint8_t ii;
    uint8_t polygon [40];
    uint8_t width_unit = (width - 1) / 3;
    uint8_t height_unit = (height - 1) / 3;
    uint8_t indent = width / 6;

    // Draw the polygon
    ii = 0;
    x_offset++;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset + height_unit * 2;
    polygon [ii++] = x_offset + indent;
    polygon [ii++] = y_offset + height_unit * 2;
    polygon [ii++] = x_offset + indent;
    polygon [ii++] = y_offset + height_unit * 1;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset + height_unit * 1;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset + indent;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset + indent;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset + height_unit;
    polygon [ii++] = x_offset + width_unit * 3 - indent;
    polygon [ii++] = y_offset + height_unit;
    polygon [ii++] = x_offset + width_unit * 3 - indent;
    polygon [ii++] = y_offset + height_unit * 2;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset + height_unit * 2;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset + height_unit * 3;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset + height_unit * 3;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset + height_unit * 3 - indent;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset + height_unit * 3 - indent;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset + height_unit * 3;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset + height_unit * 3 | 0x80;

    lcd.drawPolygon (s_r, polygon);

    ii = strlen_P (str);
    lcd.setXY (x_offset + (width - (ii * 6)) / 2, lcd.ydim - y_offset);
    lcd.putstr_P (str);
}

/////////////////////////////////////////////////////////////////////////////
// Test the draw polygon function for drawing and filling in different modes.
void
glcdTestPolygon (void)
{
    static const char title_nrm_draw[] PROGMEM = "Draw Poly Test";
    static const char title_rev_draw[] PROGMEM = "Rev Draw Poly Test";
    static const char title_nrm_fill[] PROGMEM = "Fill Poly Test";
    static const char title_rev_fill[] PROGMEM = "Rev Fill Poly Test";
    const char PROGMEM *test_name;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t ii;

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 8; ii++)
    {
        uint8_t mode = 0;

        lcd.clearScreen ();

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            if ((ii & 4) == 0)
                test_name = title_rev_draw;
            else
                test_name = title_rev_fill;
        }
        else
        {
            if ((ii & 4) == 0)
                test_name = title_nrm_draw;
            else
                test_name = title_nrm_fill;
        }

        if (ii & 4)
            mode = GLCD_MODE_FILL;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_NORMAL|GLCD_MODE_FILL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestPolygonHelper ((xdim / 3) * 0,     mode|GLCD_MODE_NORMAL,  STR_NORMAL);
            glcdTestPolygonHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR,     STR_XOR);
            glcdTestPolygonHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_REVERSE, STR_REVERSE);
        }
        else
        {
            glcdTestPolygonHelper ((xdim / 3) * 0,     mode|GLCD_MODE_OR|GLCD_MODE_NORMAL,  STR_OR);
            glcdTestPolygonHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR|GLCD_MODE_NORMAL,  STR_XOR);
            glcdTestPolygonHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_NAND|GLCD_MODE_NORMAL, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Fill polygon test. An array of different polygon fills.
void
glcdTestFillPoly (void)
{
    /* Angled corner box */
    static const uint8_t polygon1 [] PROGMEM =
    {
        3,  20,
        32, 20,
        35, 23,
        35, 60,
        32, 63,
        3,  63,
        0,  60,
        0,  23|0x80
    };

    /* Trapezium */
    static const uint8_t polygon2 [] PROGMEM =
    {
        10, 0,
        30, 0,
        20, 15,
        0, 15|0x80
    };

    /* Backwards Trapezium */
    static const uint8_t polygon3 [] PROGMEM =
    {
        40+10, 0,
        40+30, 0,
        40+40, 15,
        40+20, 15|0x80
    };

    /* Castle top and bottom */
    static const uint8_t polygon4 [] PROGMEM =
    {
        95, 5,
        95, 0,
        100, 0,
        100, 5,
        105, 5,
        105, 0,
        110, 0,
        110, 5,
        115, 5,
        115, 10,
        110, 10,
        110, 15,
        105, 15,
        105, 10,
        100, 10,
        100, 15,
        95,15,
        95,10,
        90,10,
        90, 5|0x80
    };

    /* An 'E' shape */
    static const uint8_t polygon5 [] PROGMEM =
    {
        40, 20,
        60, 20,
        60, 24,
        43, 24,
        43, 28,
        57, 28,
        57, 32,
        43, 32,
        43, 36,
        60, 36,
        60, 40,
        40, 40 | 0x80
    };

    /* A 'C' shape - clockwise */
    static const uint8_t polygon6 [] PROGMEM =
    {
        70, 20,
        90, 20,
        90, 40,
        70, 40,
        70, 36,
        87, 36,
        87, 24,
        70, 24 | 0x80
    };

    /* A 'I' shape - clockwise */
    static const uint8_t polygon7 [] PROGMEM =
    {
        30+78, 24,
        30+70, 24,
        30+70, 20,
        30+90, 20,
        30+90, 24,
        30+81, 24,
        30+81, 36,
        30+90, 36,
        30+90, 40,
        30+70, 40,
        30+70, 36,
        30+78, 36 | 0x80
    };

    /* Figure of 8 */
    static const uint8_t polygon8 [] PROGMEM =
    {
        40 +  3, 45 +  5,
        40 +  0, 45 +  5,
        40 +  0, 45 +  0,
        40 +  5, 45 +  0,
        40 +  5, 45 +  3,
        40 + 10, 45 +  3,
        40 + 10, 45 +  0,
        40 + 15, 45 +  0,
        40 + 15, 45 +  5,
        40 + 12, 45 +  5,
        40 + 12, 45 + 10,
        40 + 15, 45 + 10,
        40 + 15, 45 + 15,
        40 + 10, 45 + 15,
        40 + 10, 45 + 12,
        40 +  5, 45 + 12,
        40 +  5, 45 + 15,
        40 +  0, 45 + 15,
        40 +  0, 45 + 10,
        40 +  3, 45 + 10 | 0x80
    };

    /* A 'H' shape - clockwise */
    static const uint8_t polygon9 [] PROGMEM =
    {
        30+78, 24,
        30+70, 24,
        30+70, 20,
        30+90, 20,
        30+90, 24,
        30+81, 24,
        30+81, 36,
        30+90, 36,
        30+90, 40,
        30+70, 40,
        30+70, 36,
        30+78, 36 | 0x80
    };

    lcd.clearScreen();
    lcd.setXY (60, 45);
    lcd.putstr (F("Polygon"));
    lcd.setXY (60, 45+8);
    lcd.putstr (F("Fill Test"));
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon1);
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon2);
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon3);
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon4);
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon5);
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon6);
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon7);
    lcd.drawPolygon_P (GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL, polygon8);
    countdown ();
}

//////////////////////////////////////////////////////////////////////////////
// Helper for the draw lines test
static void
glcdTestDrawLinesHelper (uint8_t x_offset, uint8_t s_r, const char PROGMEM *str)
{
    uint8_t y_offset = 9;
    uint8_t width = lcd.xdim / 3;
    uint8_t height = lcd.ydim - 1 - (2 * y_offset);
    uint8_t ii;
    uint8_t polygon [40];
    uint8_t width_unit = (width - 1) / 3;
    uint8_t height_unit = (height - 1) / 3;
    uint8_t indent = width / 6;

    // Draw the lines
    ii = 0;
    x_offset++;
    polygon [ii++] = x_offset + indent;
    polygon [ii++] = y_offset + height_unit * 2;
    polygon [ii++] = x_offset + indent;
    polygon [ii++] = y_offset + height_unit * 1;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset + height_unit * 1;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset + indent;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset + indent;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset + height_unit;
    polygon [ii++] = x_offset + width_unit * 3 - indent;
    polygon [ii++] = y_offset + height_unit;
    polygon [ii++] = x_offset + width_unit * 3 - indent;
    polygon [ii++] = y_offset + height_unit * 2;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset + height_unit * 2;
    polygon [ii++] = x_offset + width_unit * 3;
    polygon [ii++] = y_offset + height_unit * 3;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset + height_unit * 3;
    polygon [ii++] = x_offset + width_unit * 2;
    polygon [ii++] = y_offset + height_unit * 3 - indent;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset + height_unit * 3 - indent;
    polygon [ii++] = x_offset + width_unit * 1;
    polygon [ii++] = y_offset + height_unit * 3;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset + height_unit * 3;
    polygon [ii++] = x_offset;
    polygon [ii++] = y_offset + height_unit * 2 | 0x80;

    lcd.drawLines (s_r, polygon);

    ii = strlen_P (str);
    lcd.setXY (x_offset + (width - (ii * 6)) / 2, lcd.ydim - y_offset);
    lcd.putstr_P (str);
}

/////////////////////////////////////////////////////////////////////////////
// Test the draw lines function for drawing in different modes.
void
glcdTestDrawLines (void)
{
    static const char title_nrm_draw[] PROGMEM = "Draw Lines Test";
    static const char title_rev_draw[] PROGMEM = "Rev Draw Lines Test";
    const char PROGMEM *test_name;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t ii;

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 4; ii++)
    {
        uint8_t mode = 0;

        lcd.clearScreen ();

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            test_name = title_rev_draw;
        }
        else
        {
            test_name = title_nrm_draw;
        }

        if (ii & 4)
            mode = GLCD_MODE_FILL;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_NORMAL|GLCD_MODE_FILL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestDrawLinesHelper ((xdim / 3) * 0,     mode|GLCD_MODE_NORMAL, STR_NORMAL);
            glcdTestDrawLinesHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR, STR_XOR);
            glcdTestDrawLinesHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_REVERSE, STR_REVERSE);
        }
        else
        {
            glcdTestDrawLinesHelper ((xdim / 3) * 0,     mode|GLCD_MODE_OR|GLCD_MODE_NORMAL, STR_OR);
            glcdTestDrawLinesHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR|GLCD_MODE_NORMAL, STR_XOR);
            glcdTestDrawLinesHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_NAND|GLCD_MODE_NORMAL, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Helper for the line test
static void
glcdTestLineHelper (uint8_t x_offset, uint8_t s_r, const char PROGMEM *str)
{
    uint8_t y_offset = 9;
    uint8_t width = lcd.xdim / 3;
    uint8_t height = lcd.ydim - 1 - (2 * y_offset);
    uint8_t ii;

    // Vertical line top
    lcd.drawLine (x_offset + width/2, y_offset,
                  x_offset + width/2, y_offset + height / 2,
                  s_r);
    // Vertical line bottom
    lcd.drawLine (x_offset + width/2 + 1, y_offset + 1 + height / 2 ,
                  x_offset + width/2 + 1, y_offset + height - 1,
                  s_r);

    // Horizontal line left
    lcd.drawLine (x_offset,           y_offset + height / 2 + 1,
                  x_offset + width/2, y_offset + height / 2 + 1,
                  s_r);
    // Horizontal line right
    lcd.drawLine (x_offset + width/2,   y_offset + height / 2,
                  x_offset + width - 1, y_offset + height / 2,
                  s_r);

    // Diagonal line 1/8
    lcd.drawLine (x_offset + width/2,        y_offset + height / 2,
                  x_offset + (width /4 * 3), y_offset,
                  s_r);
    // Diagonal line /
    lcd.drawLine (x_offset + width/2 + 1, y_offset + height / 2,
                  x_offset + width - 1,   y_offset,
                  s_r);
    // Diagonal line 2/8
    lcd.drawLine (x_offset + width/2,   y_offset + height / 2 + 1,
                  x_offset + width - 1, y_offset + height / 4,
                  s_r);

    // Diagonal line 7/8
    lcd.drawLine (x_offset + width/2,        y_offset + height / 2,
                  x_offset + (width /4 * 1), y_offset,
                  s_r);
    // Diagonal line /
    lcd.drawLine (x_offset + width/2 - 1, y_offset + height / 2,
                  x_offset,               y_offset,
                  s_r);
    // Diagonal line 2/8
    lcd.drawLine (x_offset + width/2 - 2, y_offset + height / 2,
                  x_offset,               y_offset + height / 4,
                  s_r);

    // Diagonal line 3/8
    lcd.drawLine (x_offset + width/2 + 2, y_offset + height / 2 + 1,
                  x_offset + width - 1,   y_offset + (height / 4) * 3,
                  s_r);
    // Diagonal line
    lcd.drawLine (x_offset + width/2 + 3, y_offset + height / 2 + 1,
                  x_offset + width - 1  , y_offset + height - 1,
                  s_r);
    // Diagonal line 2/8
    lcd.drawLine (x_offset + width/2 + 2,   y_offset + height / 2 + 2,
                  x_offset + (width * 3)/4, y_offset + height - 1,
                  s_r);

    // Diagonal line 5/8
    lcd.drawLine (x_offset + width/2 + 1,  y_offset + height / 2 + 1,
                  x_offset + width/4,  y_offset + height - 1,
                  s_r);
    // Diagonal line
    lcd.drawLine (x_offset + width/2, y_offset + height / 2 + 1,
                  x_offset,           y_offset + height - 1,
                  s_r);
    // Diagonal line 5/8
    lcd.drawLine (x_offset + width/2 + 1, y_offset + height / 2 + 2,
                  x_offset,               y_offset + (height * 3) /4,
                  s_r);

    ii = strlen_P (str);
    lcd.setXY (x_offset + (width - (ii * 6)) / 2, lcd.ydim - y_offset);
    lcd.putstr_P (str);
}

/////////////////////////////////////////////////////////////////////////////
// Test the draw line function for drawing in different modes.
void
glcdTestLine (void)
{
    static const char title_nrm_draw[] PROGMEM = "Draw Line Test";
    static const char title_rev_draw[] PROGMEM = "Rev Draw Line Test";
    const char PROGMEM *test_name;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t ii;

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 4; ii++)
    {
        lcd.clearScreen ();

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            test_name = title_rev_draw;
        }
        else
            test_name = title_nrm_draw;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_NORMAL|GLCD_MODE_FILL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestLineHelper ((xdim / 3) * 0,     GLCD_MODE_NORMAL,  STR_NORMAL);
            glcdTestLineHelper ((xdim / 3) * 1,     GLCD_MODE_XOR,     STR_XOR);
            glcdTestLineHelper ((xdim / 3) * 2 + 1, GLCD_MODE_REVERSE, STR_REVERSE);
        }
        else
        {
            glcdTestLineHelper ((xdim / 3) * 0,     GLCD_MODE_OR|GLCD_MODE_NORMAL,   STR_OR);
            glcdTestLineHelper ((xdim / 3) * 1,     GLCD_MODE_XOR|GLCD_MODE_NORMAL,  STR_XOR);
            glcdTestLineHelper ((xdim / 3) * 2 + 1, GLCD_MODE_NAND|GLCD_MODE_NORMAL, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Helper for the sprite test
static void
glcdTestSpriteHelper (uint8_t x_offset, uint8_t s_r, uint8_t id, const char PROGMEM *str)
{
    uint8_t y_offset = 9;
    uint8_t width = lcd.xdim / 3;
    uint8_t height = lcd.ydim - 1 - (2 * y_offset);
    uint8_t ii;

    // Draw the sprite
    lcd.drawSprite (x_offset + width / 2, lcd.ydim / 2, id, s_r);

    // Draw the text
    ii = strlen_P (str);
    lcd.setXY (x_offset + (width - (ii * 6)) / 2, lcd.ydim - y_offset);
    lcd.putstr_P (str);
}

/////////////////////////////////////////////////////////////////////////////
// Test the draw sprite function for draw in different modes.
void
glcdTestDrawSprite (void)
{
    static const char title_nrm_draw[] PROGMEM = "Sprite Test";
    static const char title_rev_draw[] PROGMEM = "Rev Sprite Test";
    static const char title_nrm_fill[] PROGMEM = "Center Sprite Test";
    static const char title_rev_fill[] PROGMEM = "Rev Cent Sprite Test";
    const char PROGMEM *test_name;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t ii;

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 8; ii++)
    {
        uint8_t mode = 0;

        lcd.clearScreen ();

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            if ((ii & 4) == 0)
                test_name = title_rev_draw;
            else
                test_name = title_rev_fill;
        }
        else
        {
            if ((ii & 4) == 0)
                test_name = title_nrm_draw;
            else
                test_name = title_nrm_fill;
        }

        if (ii & 4)
            mode = GLCD_MODE_FILL;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_NORMAL|GLCD_MODE_FILL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestSpriteHelper ((xdim / 3) * 0,     mode|GLCD_MODE_NORMAL,  0x80, STR_NORMAL);
            glcdTestSpriteHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR,     0x80, STR_XOR);
            glcdTestSpriteHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_REVERSE, 0x80, STR_REVERSE);
        }
        else
        {
            glcdTestSpriteHelper ((xdim / 3) * 0,     mode|GLCD_MODE_OR|GLCD_MODE_NORMAL,   0x80, STR_OR);
            glcdTestSpriteHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR|GLCD_MODE_NORMAL,  0x80, STR_XOR);
            glcdTestSpriteHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_NAND|GLCD_MODE_NORMAL, 0x80, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Test the draw sprite function for load and draw in different modes.
void
glcdTestDrawSprite2 (void)
{
    static const char title_nrm_draw[] PROGMEM = "Sprite2 Test";
    static const char title_rev_draw[] PROGMEM = "Rev Sprite2 Test";
    static const char title_nrm_fill[] PROGMEM = "Center Sprite2 Test";
    static const char title_rev_fill[] PROGMEM = "Rev Cent Sprite2 Test";
    const char PROGMEM *test_name;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t ii;
    static const uint8_t char_A [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0x00, 0xe0, 0xfe, 0x9f, 0x9f, 0xfe, 0xe0, 0x00, 0x00, /*A*/
        0x00, 0x0c, 0x0f, 0x07, 0x01, 0x01, 0x07, 0x0f, 0x0c, 0x00
    };
    static const uint8_t char_B [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0xff, 0xff, 0x63, 0x63, 0x63, 0x63, 0xfe, 0x9e, 0x00, /*B*/
        0x00, 0x0f, 0x0f, 0x0c, 0x0c, 0x0c, 0x0c, 0x0f, 0x07, 0x00
    };
    static const uint8_t char_C [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0xf8, 0xfe, 0x06, 0x03, 0x03, 0x03, 0x03, 0x06, 0x00, /*C*/
        0x00, 0x01, 0x07, 0x06, 0x0c, 0x0c, 0x0c, 0x0c, 0x06, 0x00
    };
    static const uint8_t char_D [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0xff, 0xff, 0x03, 0x03, 0x03, 0x06, 0xfe, 0xf8, 0x00, /*D*/
        0x00, 0x0f, 0x0f, 0x0c, 0x0c, 0x0c, 0x06, 0x07, 0x01, 0x00
    };
    static const uint8_t char_E [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0xff, 0xff, 0x63, 0x63, 0x63, 0x63, 0x63, 0x03, 0x00, /*E*/
        0x00, 0x0f, 0x0f, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00
    };
    static const uint8_t char_F [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0xff, 0xff, 0x63, 0x63, 0x63, 0x63, 0x63, 0x03, 0x00, /*F*/
        0x00, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    static const uint8_t char_G [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0xf8, 0xfe, 0x06, 0x03, 0xc3, 0xc3, 0xc3, 0xc6, 0x00, /*G*/
        0x00, 0x01, 0x07, 0x06, 0x0c, 0x0c, 0x0c, 0x0f, 0x07, 0x00
    };
    static const uint8_t char_H [] PROGMEM =
    {
        0x0a, 0x10,
        0x00, 0xff, 0xff, 0x60, 0x60, 0x60, 0x60, 0xff, 0xff, 0x00, /*H*/
        0x00, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x00
    };

    lcd.fontMode (GLCD_MODE_XOR);
    for (ii = 0; ii < 8; ii++)
    {
        uint8_t mode = 0;
        const uint8_t PROGMEM *sprite;
        uint8_t sprite_id;

        lcd.clearScreen ();

        switch (ii)
        {
        case 0:
            sprite = char_A;
            break;
        case 1:
            sprite = char_B;
            break;
        case 2:
            sprite = char_C;
            break;
        case 3:
            sprite = char_D;
            break;
        case 4:
            sprite = char_D;
            break;
        case 5:
            sprite = char_E;
            break;
        case 6:
            sprite = char_F;
            break;
        case 7:
            sprite = char_G;
            break;
        }

        // Upload the sprite into RAM and then EEPROM
        sprite_id = ii;
        if (sprite_id >= 6)
            sprite_id |= 0x80;          // EEPROM

        lcd.loadSprite_P (sprite_id, sprite);

        if (ii & 1)
        {
            lcd.reverseMode (GLCD_MODE_REVERSE);
            if ((ii & 4) == 0)
                test_name = title_rev_draw;
            else
                test_name = title_rev_fill;
        }
        else
        {
            if ((ii & 4) == 0)
                test_name = title_nrm_draw;
            else
                test_name = title_nrm_fill;
        }

        if (ii & 4)
            mode = GLCD_MODE_FILL;

        lcd.drawBox (xdim / 2, 0, xdim - 1, ydim - 1, GLCD_MODE_NORMAL|GLCD_MODE_FILL);
        centreString_P (0, test_name);

        if ((ii & 2) == 0)
        {
            glcdTestSpriteHelper ((xdim / 3) * 0,     mode|GLCD_MODE_NORMAL, sprite_id, STR_NORMAL);
            glcdTestSpriteHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR, sprite_id, STR_XOR);
            glcdTestSpriteHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_REVERSE, sprite_id, STR_REVERSE);
        }
        else
        {
            glcdTestSpriteHelper ((xdim / 3) * 0,     mode|GLCD_MODE_OR|GLCD_MODE_NORMAL, sprite_id, STR_OR);
            glcdTestSpriteHelper ((xdim / 3) * 1,     mode|GLCD_MODE_XOR|GLCD_MODE_NORMAL, sprite_id, STR_XOR);
            glcdTestSpriteHelper ((xdim / 3) * 2 + 1, mode|GLCD_MODE_NAND|GLCD_MODE_NORMAL, sprite_id, STR_NAND);
        }
        countdown ();

        if (ii & 1)
            lcd.reverseMode (GLCD_MODE_NORMAL);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Test a complex draw function from PROGMEM.
void
glcdComplexDrawTest (void)
{
    static const uint8_t complex [] PROGMEM =
    {
        0x7c, 0x40, /* Graphics mode on */
        0x16, 0x53, 0x09, 0x01, 0x28, 0x04,  /* Bitblt: [83,9] 40x4 */
        0x00, 0x00, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x00, /* Bitblt: Row 0 */
        0x16, 0x53, 0x17, 0x01, 0x28, 0x04,  /* Bitblt: [83,23] 40x4 */
        0x01, 0x01, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x02, 0x02, /* Bitblt: Row 0 */
        0x16, 0x53, 0x1d, 0x01, 0x28, 0x04,  /* Bitblt: [83,29] 40x4 */
        0x00, 0x00, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x02, 0x01, 0x02, 0x0c, 0x00, /* Bitblt: Row 0 */
        0x16, 0x53, 0x2b, 0x01, 0x28, 0x04,  /* Bitblt: [83,43] 40x4 */
        0x02, 0x02, 0x02, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x03, 0x04, 0x08, 0x04, 0x02, 0x02, /* Bitblt: Row 0 */
        0x06, 0x00, 0x18, 0x01, 0x3b, 0xff, /* Filled Box (0,24) -> (1, 59) = ff [2x36 @ 72] */
        0x06, 0x04, 0x14, 0x1f, 0x15, 0xff, /* Filled Box (4,20) -> (31, 21) = ff [28x2 @ 56] */
        0x06, 0x04, 0x3e, 0x1f, 0x3f, 0xff, /* Filled Box (4,62) -> (31, 63) = ff [28x2 @ 56] */
        0x06, 0x22, 0x2f, 0x23, 0x3b, 0xff, /* Filled Box (34,47) -> (35, 59) = ff [2x13 @ 26] */
        0x06, 0x22, 0x23, 0x23, 0x2c, 0xff, /* Filled Box (34,35) -> (35, 44) = ff [2x10 @ 20] */
        0x06, 0x22, 0x18, 0x23, 0x20, 0xff, /* Filled Box (34,24) -> (35, 32) = ff [2x9 @ 18] */
        0x06, 0x0f, 0x1d, 0x10, 0x1e, 0xff, /* Filled Box (15,29) -> (16, 30) = ff [2x2 @ 4] */
        0x06, 0x0f, 0x2b, 0x10, 0x2c, 0xff, /* Filled Box (15,43) -> (16, 44) = ff [2x2 @ 4] */
        0x06, 0x0f, 0x39, 0x10, 0x3a, 0xff, /* Filled Box (15,57) -> (16, 58) = ff [2x2 @ 4] */
        0x06, 0x12, 0x05, 0x13, 0x06, 0xff, /* Filled Box (18,5) -> (19, 6) = ff [2x2 @ 4] */
        0x06, 0x12, 0x0e, 0x13, 0x0f, 0xff, /* Filled Box (18,14) -> (19, 15) = ff [2x2 @ 4] */
        0x06, 0x2f, 0x0c, 0x30, 0x0d, 0xff, /* Filled Box (47,12) -> (48, 13) = ff [2x2 @ 4] */
        0x06, 0x33, 0x0c, 0x34, 0x0d, 0xff, /* Filled Box (51,12) -> (52, 13) = ff [2x2 @ 4] */
        0x06, 0x34, 0x39, 0x35, 0x3a, 0xff, /* Filled Box (52,57) -> (53, 58) = ff [2x2 @ 4] */
        0x06, 0x34, 0x3d, 0x35, 0x3e, 0xff, /* Filled Box (52,61) -> (53, 62) = ff [2x2 @ 4] */
        0x06, 0x37, 0x0c, 0x38, 0x0d, 0xff, /* Filled Box (55,12) -> (56, 13) = ff [2x2 @ 4] */
        0x06, 0x3b, 0x0c, 0x3c, 0x0d, 0xff, /* Filled Box (59,12) -> (60, 13) = ff [2x2 @ 4] */
        0x06, 0x3f, 0x0c, 0x40, 0x0d, 0xff, /* Filled Box (63,12) -> (64, 13) = ff [2x2 @ 4] */
        0x06, 0x64, 0x13, 0x65, 0x14, 0xff, /* Filled Box (100,19) -> (101, 20) = ff [2x2 @ 4] */
        0x06, 0x64, 0x27, 0x65, 0x28, 0xff, /* Filled Box (100,39) -> (101, 40) = ff [2x2 @ 4] */
        0x06, 0x6e, 0x05, 0x6f, 0x06, 0xff, /* Filled Box (110,5) -> (111, 6) = ff [2x2 @ 4] */
        0x4f, 0x1b, 0x00, 0x1d, 0x02, /* Auto Rectangle: (27,0)->(29,2) 3x3 @ 1 */
        0x4f, 0x77, 0x00, 0x79, 0x02, /* Auto Rectangle: (119,0)->(121,2) 3x3 @ 1 */
        0x4f, 0x1b, 0x09, 0x1d, 0x0b, /* Auto Rectangle: (27,9)->(29,11) 3x3 @ 1 */
        0x4f, 0x2c, 0x0a, 0x43, 0x18, /* Auto Rectangle: (44,10)->(67,24) 24x15 @ 1 */
        0x4f, 0x6d, 0x0e, 0x6f, 0x10, /* Auto Rectangle: (109,14)->(111,16) 3x3 @ 1 */
        0x4f, 0x18, 0x18, 0x1a, 0x1a, /* Auto Rectangle: (24,24)->(26,26) 3x3 @ 1 */
        0x4f, 0x4d, 0x1a, 0x52, 0x1d, /* Auto Rectangle: (77,26)->(82,29) 6x4 @ 1 */
        0x4f, 0x6d, 0x22, 0x6f, 0x24, /* Auto Rectangle: (109,34)->(111,36) 3x3 @ 1 */
        0x4f, 0x18, 0x26, 0x1a, 0x28, /* Auto Rectangle: (24,38)->(26,40) 3x3 @ 1 */
        0x51, 0x32, 0x32, 0x32, 0x2f, 0x2f, 0x2f, 0x2f, 0x35, 0x32, 0x35|0x80,  /* Auto Rectangle: (47,47)->(50,53) 4x7 @ 1 */
        0x4f, 0x18, 0x34, 0x1a, 0x36, /* Auto Rectangle: (24,52)->(26,54) 3x3 @ 1 */
        0x4c, 0x03, 0x14, 0x00, 0x17, /* Diagonal Line 3,20 -> 0,23 = 1 [4] */
        0x4c, 0x20, 0x14, 0x23, 0x17, /* Diagonal Line 32,20 -> 35,23 = 1 [4] */
        0x4c, 0x02, 0x15, 0x00, 0x17, /* Diagonal Line 2,21 -> 0,23 = 1 [3] */
        0x4c, 0x03, 0x15, 0x01, 0x17, /* Diagonal Line 3,21 -> 1,23 = 1 [3] */
        0x4c, 0x20, 0x15, 0x22, 0x17, /* Diagonal Line 32,21 -> 34,23 = 1 [3] */
        0x4c, 0x21, 0x15, 0x23, 0x17, /* Diagonal Line 33,21 -> 35,23 = 1 [3] */
        0x4c, 0x01, 0x16, 0x00, 0x17, /* Diagonal Line 1,22 -> 0,23 = 1 [2] */
        0x4c, 0x02, 0x16, 0x01, 0x17, /* Diagonal Line 2,22 -> 1,23 = 1 [2] */
        0x4c, 0x21, 0x16, 0x22, 0x17, /* Diagonal Line 33,22 -> 34,23 = 1 [2] */
        0x4c, 0x22, 0x16, 0x23, 0x17, /* Diagonal Line 34,22 -> 35,23 = 1 [2] */
        0x4c, 0x3b, 0x1f, 0x3a, 0x20, /* Diagonal Line 59,31 -> 58,32 = 1 [2] */
        0x4c, 0x42, 0x1f, 0x43, 0x20, /* Diagonal Line 66,31 -> 67,32 = 1 [2] */
        0x4c, 0x38, 0x23, 0x39, 0x24, /* Diagonal Line 56,35 -> 57,36 = 1 [2] */
        0x4c, 0x45, 0x23, 0x44, 0x24, /* Diagonal Line 69,35 -> 68,36 = 1 [2] */
        0x4c, 0x39, 0x25, 0x3b, 0x27, /* Diagonal Line 57,37 -> 59,39 = 1 [3] */
        0x4c, 0x44, 0x25, 0x42, 0x27, /* Diagonal Line 68,37 -> 66,39 = 1 [3] */
        0x4c, 0x3a, 0x26, 0x3b, 0x27, /* Diagonal Line 58,38 -> 59,39 = 1 [2] */
        0x4c, 0x43, 0x26, 0x42, 0x27, /* Diagonal Line 67,38 -> 66,39 = 1 [2] */
        0x4c, 0x00, 0x3c, 0x03, 0x3f, /* Diagonal Line 0,60 -> 3,63 = 1 [4] */
        0x4c, 0x01, 0x3c, 0x03, 0x3e, /* Diagonal Line 1,60 -> 3,62 = 1 [3] */
        0x4c, 0x22, 0x3c, 0x20, 0x3e, /* Diagonal Line 34,60 -> 32,62 = 1 [3] */
        0x4c, 0x23, 0x3c, 0x20, 0x3f, /* Diagonal Line 35,60 -> 32,63 = 1 [4] */
        0x4c, 0x01, 0x3d, 0x03, 0x3f, /* Diagonal Line 1,61 -> 3,63 = 1 [3] */
        0x4c, 0x02, 0x3d, 0x03, 0x3e, /* Diagonal Line 2,61 -> 3,62 = 1 [2] */
        0x4c, 0x21, 0x3d, 0x20, 0x3e, /* Diagonal Line 33,61 -> 32,62 = 1 [2] */
        0x4c, 0x22, 0x3d, 0x20, 0x3f, /* Diagonal Line 34,61 -> 32,63 = 1 [3] */
        0x4c, 0x02, 0x3e, 0x03, 0x3f, /* Diagonal Line 2,62 -> 3,63 = 1 [2] */
        0x4c, 0x21, 0x3e, 0x20, 0x3f, /* Diagonal Line 33,62 -> 32,63 = 1 [2] */
        0x4c, 0x33, 0x35, 0x7f, 0x35, /* Line 51,53 -> 127,53 = 77 [1] */
        0x4c, 0x33, 0x32, 0x7c, 0x32, /* Line 51,50 -> 124,50 = 74 [1] */
        0x4c, 0x7f, 0x15, 0x7f, 0x35, /* Line 127,21 -> 127,53 = 33 [1] */
        0x4c, 0x24, 0x2c, 0x3b, 0x2c, /* Line 36,44 -> 59,44 = 24 [1] */
        0x4c, 0x32, 0x19, 0x32, 0x2c, /* Line 50,25 -> 50,44 = 20 [1] */
        0x4c, 0x7c, 0x18, 0x7c, 0x29, /* Line 124,24 -> 124,41 = 18 [1] */
        0x4c, 0x42, 0x2f, 0x51, 0x2f, /* Line 66,47 -> 81,47 = 16 [1] */
        0x4c, 0x4e, 0x1e, 0x4e, 0x2c, /* Line 78,30 -> 78,44 = 15 [1] */
        0x4c, 0x42, 0x2c, 0x4e, 0x2c, /* Line 66,44 -> 78,44 = 13 [1] */
        0x4c, 0x51, 0x1e, 0x51, 0x29, /* Line 81,30 -> 81,41 = 12 [1] */
        0x4c, 0x24, 0x20, 0x2f, 0x20, /* Line 36,32 -> 47,32 = 12 [1] */
        0x4c, 0x24, 0x23, 0x2f, 0x23, /* Line 36,35 -> 47,35 = 12 [1] */
        0x4c, 0x24, 0x2f, 0x2e, 0x2f, /* Line 36,47 -> 46,47 = 11 [1] */
        0x4c, 0x2f, 0x24, 0x2f, 0x2c, /* Line 47,36 -> 47,44 = 9 [1] */
        0x4c, 0x55, 0x21, 0x55, 0x29, /* Line 85,33 -> 85,41 = 9 [1] */
        0x4c, 0x79, 0x0d, 0x79, 0x15, /* Line 121,13 -> 121,21 = 9 [1] */
        0x4c, 0x79, 0x21, 0x79, 0x29, /* Line 121,33 -> 121,41 = 9 [1] */
        0x4c, 0x33, 0x2f, 0x3b, 0x2f, /* Line 51,47 -> 59,47 = 9 [1] */
        0x4c, 0x2f, 0x19, 0x2f, 0x20, /* Line 47,25 -> 47,32 = 8 [1] */
        0x4c, 0x55, 0x0d, 0x55, 0x14, /* Line 85,13 -> 85,20 = 8 [1] */
        0x4c, 0x4e, 0x14, 0x55, 0x14, /* Line 78,20 -> 85,20 = 8 [1] */
        0x4c, 0x3b, 0x30, 0x42, 0x30, /* Line 59,48 -> 66,48 = 8 [1] */
        0x4c, 0x7c, 0x2c, 0x7c, 0x32, /* Line 124,44 -> 124,50 = 7 [1] */
        0x4c, 0x3b, 0x2b, 0x3b, 0x30, /* Line 59,43 -> 59,48 = 6 [1] */
        0x4c, 0x3d, 0x19, 0x3d, 0x1e, /* Line 61,25 -> 61,30 = 6 [1] */
        0x4c, 0x40, 0x19, 0x40, 0x1e, /* Line 64,25 -> 64,30 = 6 [1] */
        0x4c, 0x42, 0x2b, 0x42, 0x30, /* Line 66,43 -> 66,48 = 6 [1] */
        0x4c, 0x7a, 0x15, 0x7f, 0x15, /* Line 122,21 -> 127,21 = 6 [1] */
        0x4c, 0x4e, 0x15, 0x4e, 0x19, /* Line 78,21 -> 78,25 = 5 [1] */
        0x4c, 0x00, 0x03, 0x04, 0x03, /* Line 0,3 -> 4,3 = 5 [1] */
        0x4c, 0x00, 0x0c, 0x04, 0x0c, /* Line 0,12 -> 4,12 = 5 [1] */
        0x4c, 0x3d, 0x28, 0x3d, 0x2b, /* Line 61,40 -> 61,43 = 4 [1] */
        0x4c, 0x40, 0x28, 0x40, 0x2b, /* Line 64,40 -> 64,43 = 4 [1] */
        0x4c, 0x51, 0x2c, 0x51, 0x2f, /* Line 81,44 -> 81,47 = 4 [1] */
        0x4c, 0x52, 0x29, 0x55, 0x29, /* Line 82,41 -> 85,41 = 4 [1] */
        0x4c, 0x01, 0x0b, 0x01, 0x0d, /* Line 1,11 -> 1,13 = 3 [1] */
        0x4c, 0x03, 0x02, 0x03, 0x04, /* Line 3,2 -> 3,4 = 3 [1] */
        0x4c, 0x51, 0x17, 0x51, 0x19, /* Line 81,23 -> 81,25 = 3 [1] */
        0x4c, 0x7a, 0x29, 0x7c, 0x29, /* Line 122,41 -> 124,41 = 3 [1] */
        0x4c, 0x39, 0x21, 0x39, 0x22, /* Line 57,33 -> 57,34 = 2 [1] */
        0x4c, 0x44, 0x21, 0x44, 0x22, /* Line 68,33 -> 68,34 = 2 [1] */
        0x4c, 0x7b, 0x18, 0x7c, 0x18, /* Line 123,24 -> 124,24 = 2 [1] */
        0x4c, 0x3c, 0x1e, 0x3d, 0x1e, /* Line 60,30 -> 61,30 = 2 [1] */
        0x4c, 0x3c, 0x28, 0x3d, 0x28, /* Line 60,40 -> 61,40 = 2 [1] */
        0x4c, 0x3c, 0x2b, 0x3d, 0x2b, /* Line 60,43 -> 61,43 = 2 [1] */
        0x4c, 0x41, 0x2b, 0x42, 0x2b, /* Line 65,43 -> 66,43 = 2 [1] */
        0x4c, 0x7b, 0x2c, 0x7c, 0x2c, /* Line 123,44 -> 124,44 = 2 [1] */
        0x50, 0x02, 0x01,             /* Pixel [2,1] =  1 */
        0x50, 0x02, 0x05,             /* Pixel [2,5] =  1 */
        0x50, 0x02, 0x0a,             /* Pixel [2,10] =  1 */
        0x50, 0x02, 0x0e,             /* Pixel [2,14] =  1 */
        0x50, 0x41, 0x1e,             /* Pixel [65,30] =  1 */
        0x50, 0x41, 0x28,             /* Pixel [65,40] =  1 */
        0x50, 0x52, 0x17,             /* Pixel [82,23] =  1 */
        0x50, 0x52, 0x2c              /* Pixel [82,44] =  1 */
        /* Graphics mode off - we do not need this because a 0x7c command
         * follows. */

        /* 0x41 */
    };

    static const uint8_t annotation [] PROGMEM =
    {
        0x7c, 0x58, 0x06, 0x00, '5', '6', /* Char [6, 0] = C1 */
        0x7c, 0x58, 0x15, 0x00, '8', /* Char [21, 0] = C2 */
        0x7c, 0x58, 0x1f, 0x00, 'C', /* Char [31, 0] = C3 */
        0x7c, 0x58, 0x2c, 0x00, ' ', /* Char [44, 0] = C24 */
        0x7c, 0x58, 0x32, 0x00, ' ', /* Char [50, 0] = C25 */
        0x7c, 0x58, 0x38, 0x00, ' ', /* Char [56, 0] = C26 */
        0x7c, 0x58, 0x3e, 0x00, ' ', /* Char [62, 0] = C27 */
        0x7c, 0x58, 0x44, 0x00, ' ', /* Char [68, 0] = C28 */
        0x7c, 0x58, 0x4a, 0x00, ' ', /* Char [74, 0] = C29 */
        0x7c, 0x58, 0x5c, 0x00, ' ', ' ', '9', /* Char [92, 0] = C8 */
        0x7c, 0x58, 0x71, 0x00, '8', /* Char [113, 0] = C9 */
        0x7c, 0x58, 0x7b, 0x00, 'C', /* Char [123, 0] = C3 */
        0x7c, 0x58, 0x06, 0x09, '5', '1', /* Char [6, 9] = C4 */
        0x7c, 0x58, 0x15, 0x09, '2', /* Char [21, 9] = C5 */
        0x7c, 0x58, 0x1f, 0x09, 'C', /* Char [31, 9] = C3 */
        0x7c, 0x58, 0x58, 0x0e, '2', '1', /* Char [88, 14] = C6 */
        0x7c, 0x58, 0x67, 0x0e, '0', /* Char [103, 14] = C7 */
        0x7c, 0x58, 0x71, 0x0e, 'C', /* Char [113, 14] = C3 */
        0x7c, 0x58, 0x2f, 0x0f, 'O', 'N', ' ', /* Char [47, 15] = C23 */
        0x7c, 0x58, 0x03, 0x18, '5', '3', /* Char [3, 24] = C12 */
        0x7c, 0x58, 0x12, 0x18, '7', /* Char [18, 24] = C13 */
        0x7c, 0x58, 0x1c, 0x18, 'C', /* Char [28, 24] = C3 */
        0x7c, 0x58, 0x58, 0x22, '2', '2', /* Char [88, 34] = C10 */
        0x7c, 0x58, 0x67, 0x22, '1', /* Char [103, 34] = C11 */
        0x7c, 0x58, 0x71, 0x22, 'C', /* Char [113, 34] = C3 */
        0x7c, 0x58, 0x03, 0x26, '4', '3', /* Char [3, 38] = C14 */
        0x7c, 0x58, 0x12, 0x26, '6', /* Char [18, 38] = C15 */
        0x7c, 0x58, 0x1c, 0x26, 'C', /* Char [28, 38] = C3 */
        0x7c, 0x58, 0x03, 0x34, '3', '7', /* Char [3, 52] = C16 */
        0x7c, 0x58, 0x12, 0x34, '7', /* Char [18, 52] = C17 */
        0x7c, 0x58, 0x1c, 0x34, 'C', /* Char [28, 52] = C3 */
        0x7c, 0x58, 0x27, 0x38, '2', '1', /* Char [39, 56] = C18 */
        0x7c, 0x58, 0x38, 0x38, '3', '9', /* Char [56, 56] = C19 */
        0x7c, 0x58, 0x48, 0x38, 'S', 'U', 'N', /* Char [72, 56] = C20 */
        0x7c, 0x58, 0x5d, 0x38, '2', '6', /* Char [93, 56] = C21 */
        0x7c, 0x58, 0x6c, 0x38, 'A', 'P', 'R' /* Char [108, 56] = C22 */
    };

    // Label the test
    lcd.clearScreen ();
    centreString_P (0, (const char PROGMEM *) F("Complex Write Test"));
    centreString_P (8, (const char PROGMEM *) F("Noted a 128x64 draw"));
    delay (2000);

    // Do the test.
    lcd.clearScreen ();
    lcd.drawMode (GLCD_MODE_NORMAL);
    lcd.write_P (complex, sizeof (complex));
    lcd.write_P (annotation, sizeof (annotation));
    delay (5000);
}

/////////////////////////////////////////////////////////////////////////////
// Test the different bitblt operations.
void
glcdTestBitblt ()
{
    static const uint8_t bitmap_128x64 [] PROGMEM =
    {
        0x80, 0x40, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x7f, 0x3f, 0x1f, 0x1f, 0x0f, 0x0f, 0x0f,
        0x0f, 0x0f, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x0f, 0x0f, 0x0f, 0x0f,
        0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x1f,
        0x1f, 0x1f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x7f,
        0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x0f, 0x07, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x07, 0x0f, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0,
        0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x07, 0x0f,
        0x1f, 0x3f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x78,
        0xf0, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x06,
        0x06, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07, 0x07,
        0x0f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x3f, 0x7f,
        0xff, 0xff, 0xf8, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0,
        0xe0, 0xe0, 0xe0, 0xf0, 0xf8, 0xfe, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfc,
        0xf8, 0xf8, 0xf8, 0xf8, 0xf0, 0xf0, 0xf0, 0x70,
        0x70, 0x60, 0x60, 0x60, 0x20, 0x20, 0x20, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40,
        0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0xe0, 0xe0,
        0xe0, 0xe0, 0xe0, 0xe0, 0xf0, 0xf0, 0xe0, 0xc0,
        0xc0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07,
        0x0f, 0x1f, 0x3f, 0x7f, 0x7f, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xff,
        0xff, 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x0f, 0x07,
        0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
        0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x0f, 0x0f,
        0x1f, 0x1f, 0x1f, 0x3f, 0x3f, 0x7e, 0x7e, 0xfc,
        0xfc, 0xf8, 0xf8, 0xf0, 0xf0, 0xe0, 0xe0, 0xc0,
        0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x03,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xe0, 0xf0,
        0xf8, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc,
        0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc,
        0xfc, 0xfc, 0xfc, 0xf8, 0xf8, 0xf8, 0xf8, 0xf0,
        0xf0, 0xf0, 0xf0, 0xe0, 0xe0, 0xc0, 0xc0, 0xc0,
        0xc0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x03, 0x03, 0x07, 0x0f, 0x3f, 0x7f,
        0x3f, 0x3f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x0f,
        0x07, 0x07, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xf0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xc0,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x0f, 0x07, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x07, 0x0f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe,
        0xfc, 0xf8, 0xf8, 0xf0, 0x80, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xe0,
        0xe0, 0xe0, 0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0xfc,
        0xfc, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0x0f, 0x07,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07,
        0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
        0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xf8, 0xf0, 0xe0, 0xe0, 0xe0,
        0xe0, 0xe0, 0xe0, 0xe0, 0xf0, 0xf8, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xf8,
        0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8,
        0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfe, 0xfe, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xf0,
        0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xf0,
        0xf8, 0xff
    };

    static const uint8_t bitmap_160x128 [] PROGMEM = {
        0xa0, 0x80   /* Width, Height */
        , 0x00, 0xc0, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc
        , 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfe, 0xfe
        , 0xfe, 0xfe, 0xfe, 0xfe, 0xf0, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0
        , 0xe0, 0xe0, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0x00/* Row 0 */
        , 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x1f
        , 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x1f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f
        , 0x1f, 0x1f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x1f, 0x1f, 0x1f
        , 0x1f, 0x1f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
        , 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
        , 0x1f, 0x1f, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00/* Row 1 */
        , 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x1f, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xf8, 0xf8, 0xf8
        , 0xfc, 0xfc, 0xf8, 0xf8, 0xf0, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf8, 0xf0
        , 0xf0, 0xf8, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x90, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
        , 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf8, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 2 */
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x03, 0x03, 0x03, 0x03, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03
        , 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
        , 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 3 */
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0
        , 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x03, 0x00
        , 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xf3, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 4 */
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x80, 0xf8, 0xfe, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xf1, 0xf0, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 5 */
        , 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0xc0, 0xf0, 0xfc, 0xff, 0xff, 0xff, 0x7f
        , 0x1f, 0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x01, 0x01, 0x03, 0x03, 0x07, 0x0f, 0x0f
        , 0x1f, 0x3f, 0x3f, 0x7f, 0xff, 0xfe, 0xfe, 0xfc
        , 0xf8, 0xf0, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf0, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff
        , 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 6 */
        , 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x07, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03
        , 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x81, 0x81, 0x81
        , 0x81, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0xc0, 0xe0
        , 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0
        , 0xe0, 0xe0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 7 */
        , 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0xc0, 0xf0, 0xf8, 0xfc, 0xfc, 0xfe, 0xfe
        , 0x7e, 0x7e, 0x7f, 0x7f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x7f, 0x7f, 0x7f, 0x7e, 0xfe, 0xfe
        , 0xfe, 0xfc, 0xfc, 0xf8, 0x00, 0x00, 0xf8, 0xfe
        , 0xfe, 0xfe, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0x00
        , 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0f, 0x0f
        , 0x0f, 0x1f, 0x1f, 0x3f, 0x7f, 0xff, 0xff, 0xff
        , 0xfe, 0xfe, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 8 */
        , 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00
        , 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 9 */
        , 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf8
        , 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8
        , 0xf8, 0xf8, 0x80, 0x00, 0x00, 0xf0, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 10 */
        , 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xc0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x81, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x80, 0xe0, 0xf8, 0xff, 0xff, 0xff, 0xff, 0x7f
        , 0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 11 */
        , 0x00, 0x00, 0x80, 0x01, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x0f, 0x0f
        , 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x1f, 0x0f, 0x07, 0x00, 0x00, 0x01, 0x0f, 0x1f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x7f, 0x7e, 0x7e, 0x7e
        , 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e
        , 0x7e, 0x3c, 0x00, 0x00, 0x07, 0x1f, 0x1f, 0x3f
        , 0x3f, 0x7f, 0x7f, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e
        , 0x7f, 0x3f, 0x3f, 0x3f, 0x3f, 0x1f, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x3f, 0x7f, 0x7f, 0x7f
        , 0x7f, 0x7f, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7f
        , 0x7f, 0x7f, 0x7f, 0x3f, 0x1f, 0x0f, 0x03, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/* Row 12 */
        , 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfe
        , 0xfc, 0xfc, 0xf8, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0
        , 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0
        , 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0
        , 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0
        , 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0
        , 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xc0
        , 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0
        , 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0
        , 0xc0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
        , 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
        , 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
        , 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
        , 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
        , 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
        , 0x80, 0x80, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xfc, 0xfc, 0xfc, 0xf8, 0x00, 0x00/* Row 13 */
        , 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00/* Row 14 */
        , 0x00, 0x00, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
        , 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
        , 0x1f, 0x1f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
        , 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01
        , 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        , 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        , 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x00/* Row 15 */
    };

    // Label the test
    lcd.clearScreen ();
    centreString_P (0, (const char PROGMEM *) F("Bitblt Test"));
    delay (2000);
    lcd.clearScreen ();

    // Test parameters (x, y, mode, sprite)
    if (lcd.xdim == 128)
        lcd.bitblt_P (0, 0, GLCD_MODE_NORMAL, bitmap_128x64);
    else
        lcd.bitblt_P (0, 0, GLCD_MODE_NORMAL, bitmap_160x128);

    countdown ();
    lcd.clearScreen ();
    centreString_P (0, (const char PROGMEM *) F("Rev Bitblt Test"));
    delay (2000);
    lcd.clearScreen ();

    // Test parameters (x, y, mode, sprite) - reversed
    if (lcd.xdim == 128)
        lcd.bitblt_P (0, 0, GLCD_MODE_REVERSE, bitmap_128x64);
    else
        lcd.bitblt_P (0, 0, GLCD_MODE_REVERSE, bitmap_160x128);

    countdown ();
    lcd.clearScreen ();
    centreString_P (0, (const char PROGMEM *) F("Bitblt WH Test"));
    delay (2000);
    lcd.clearScreen ();

    // Test parameters (x, y, mode, width, height, sprite)
    if (lcd.xdim == 128)
        lcd.bitblt_P (0, 0, GLCD_MODE_NORMAL,
                      pgm_read_byte (&bitmap_128x64[0]),
                      pgm_read_byte (&bitmap_128x64[1]),
                      &bitmap_128x64[2]);
    else
        lcd.bitblt_P (0, 0, GLCD_MODE_NORMAL,
                      pgm_read_byte (&bitmap_160x128[0]),
                      pgm_read_byte (&bitmap_160x128[1]),
                      &bitmap_160x128[2]);

    countdown ();
    lcd.clearScreen ();
    centreString_P (0, (const char PROGMEM *) F("Bitblt Size Test"));
    delay (2000);
    lcd.clearScreen ();

    // Test parameters (x, y, mode, length, sprite)
    if (lcd.xdim == 128)
        lcd.bitblt_P (0, 0, GLCD_MODE_NORMAL, sizeof (bitmap_128x64), bitmap_128x64);
    else
        lcd.bitblt_P (0, 0, GLCD_MODE_NORMAL, sizeof (bitmap_160x128), bitmap_160x128);
    countdown ();
}

/////////////////////////////////////////////////////////////////////////////
// Test character rendering
void
glcdCharTest (void)
{
    int ii;
    int qq;

    // Test Scroll
    lcd.clearScreen ();
    lcd.setXY (0,0);
    lcd.fontMode (GLCD_MODE_NORMAL);
    lcd.printStr (F("Query Scroll\r\n"));
    qq = lcd.query (GLCD_ID_SCROLL);
    if (qq < 0)
    {
        lcd.printStr (F("FAILED"));
        countdown ();
    }
    else
    {
        uint8_t old_setting = qq;

        // Print the number.
        lcd.printStr (F("passed("));
        lcd.printNum (qq);
        lcd.printStr (F(")"));
        lcd.nextLine ();
        countdown ();

        // Change the setting.
        if (old_setting == 0)
        {
            lcd.set (GLCD_ID_SCROLL, 0);
            lcd.printStr (F("Query scroll "));
            lcd.nextLine ();
            qq = lcd.query (GLCD_ID_SCROLL);
        }

        // Scroll is enabled.
        if (qq == 0)
        {
            for (ii = 0; ii < 20; ii++)
            {
                lcd.printStr (F("Scroll line "));
                lcd.printNum (ii);
                lcd.nextLine ();
                delay(500);
            }

            // Change the scroll mode to 1
            lcd.set (GLCD_ID_SCROLL, 1);
            lcd.printStr (F("Query scroll "));
            qq = lcd.query (GLCD_ID_SCROLL);
            if (qq != 1)
            {
                lcd.printStr (F("FAILED("));
                lcd.printNum (qq);
                lcd.printStr (F(")"));
                lcd.nextLine ();
                countdown();
            }
            lcd.demo();
            delay(5000);

            for (ii = 0; ii < 20; ii++)
            {
                lcd.printStr (F("No scroll line "));
                lcd.printNum (ii);
                lcd.nextLine ();
                delay(500);
            }

            // Change the scroll mode to 0
            lcd.set (GLCD_ID_SCROLL, 0);
            lcd.printStr (F("Query scroll "));
            qq = lcd.query (GLCD_ID_SCROLL);
            if (qq != 0)
            {
                lcd.printStr (F("FAILED("));
                lcd.printNum (qq);
                lcd.printStr (F(")"));
                lcd.nextLine ();
                countdown();
            }
            lcd.demo();
            delay(5000);
        }
        lcd.set (GLCD_ID_SCROLL, old_setting);

    }

    // Test CRLF
    lcd.clearScreen ();
    lcd.setXY (0,0);
    lcd.printStr (F("Query CRLF\r\n"));
    qq = lcd.query (GLCD_ID_CRLF);
    if (qq < 0)
    {
        lcd.printStr (F("FAILED"));
        countdown();
    }
    else
    {
        uint8_t old_value = qq;

        // Print the number.
        lcd.printStr (F("passed("));
        lcd.printNum (qq);
        lcd.printStr (F(")"));
        lcd.nextLine ();
        countdown();
        lcd.demo();
        delay(5000);

        // Change the CRLF mode to 0
        lcd.set (GLCD_ID_CRLF, 0);
        lcd.printStr (F("Query CRLF "));
        qq = lcd.query (GLCD_ID_CRLF);
        if (qq != 0)
        {
            lcd.printStr (F("FAILED("));
            lcd.printNum (qq);
            lcd.printStr (F(")"));
            lcd.nextLine ();
            countdown();
        }
        lcd.nextLine ();

        // CRLF is enabled.
        if (qq == 0)
        {
            lcd.printStr (F("CRLF=0\r\n"));
            lcd.printStr (F("Vertical Align\r\n"));
            lcd.printStr (F("Expected\r\n"));
            for (ii = 0; ii < 5; ii++)
            {
                lcd.printNum (ii);
                lcd.printStr (F("\n"));
                delay (500);
            }

            // Change the scroll mode to 0
            lcd.set (GLCD_ID_CRLF, 1);
            lcd.printStr (F("Query CRLF\r\n"));
            qq = lcd.query (GLCD_ID_CRLF);
            if (qq != 1)
            {
                lcd.printStr (F("FAILED("));
                lcd.printNum (qq);
                lcd.printStr (F(")"));
                lcd.nextLine ();
                countdown();
            }
            lcd.demo();
            delay(5000);

            lcd.printStr (F("CRLF=1\r\n"));
            lcd.printStr (F("Vertical Step\r\n"));
            lcd.printStr (F("Expected\r\n"));
            for (ii = 0; ii < 5; ii++)
            {
                lcd.printNum (ii);
                lcd.printStr (F("\n"));
                delay (500);
            }

            // Change the CRLF mode to 0
            lcd.set (GLCD_ID_CRLF, 0);
            lcd.printStr (F("Query CRLF "));
            qq = lcd.query (GLCD_ID_CRLF);
            if (qq != 0)
            {
                lcd.printStr (F("FAILED("));
                lcd.printNum (qq);
                lcd.printStr (F(")"));
                lcd.nextLine ();
                countdown();
            }
            lcd.demo();
            delay(5000);
        }

        lcd.set (GLCD_ID_CRLF, old_value);
    }

    lcd.clearScreen ();
    lcd.setXY (0,0);
    lcd.printStr(F("Query Test"));
    lcd.nextLine();
    {
        int ii;

        for (ii = 0; ii <= GLCD_ID_FONT; ii++)
        {
            int qq = lcd.query (ii);

            lcd.printStr (F("Query["));
            lcd.printNum (ii);
            lcd.printStr (F("] = "));
            lcd.printNum (qq & 0xff);
            lcd.nextLine ();
            delay (500);
        }
        for (ii = GLCD_ID_VERSION_MAJOR; ii <= GLCD_ID_RAM_SPRITE_NUM; ii++)
        {
            int qq = lcd.query (ii);

            lcd.printStr (F("Query["));
            lcd.printNum (ii);
            lcd.printStr (F("] = "));
            lcd.printNum (qq & 0xff);
            lcd.nextLine ();
            delay (500);
        }
    }

    // Test CR
    lcd.clearScreen ();
    lcd.setXY (0,0);
    lcd.printStr(F("Test CR (\\r)"));
    lcd.nextLine();
    lcd.printStr(F("Expect over-write"));
    lcd.nextLine();
    {
        int ii;
        for (ii = 0; ii < 5000; ii++)
        {
            lcd.printStr (F("Iteration = "));
            lcd.printNum (ii);
            lcd.printStr (F("\r"));
        }
    }
    countdown();

    // Test lots of writes.
    lcd.printStr(F("Test writes"));
    lcd.nextLine();
    {
        int ii;
        for (ii = 0; ii < 500; ii++)
        {
            lcd.printStr (F("Iteration = "));
            lcd.printNum (ii);
            lcd.nextLine();
        }
    }
    countdown();
}

/////////////////////////////////////////////////////////////////////////////
// Draw the demo page
void
glcdDemo ()
{
    lcd.demo ();
    delay (5000);
}

/////////////////////////////////////////////////////////////////////////////
// Draw the font page
void
glcdDrawFrontPage ()
{
    char *s;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t x;
    uint8_t y = (ydim - (3 * 8)) / 2;

    lcd.clearScreen ();
    if (ydim == 64)
    {
        y += 8;
        lcd.setXY (5, y + 0);
        lcd.putstr (F("Alternative"));

        s = "Sparkfun";
        lcd.setXY (xdim - (strlen(s) * 6) - 4, y + 0);
        lcd.printStr (s);
    }
    else
    {
        s = "Alternative Sparkfun";
        lcd.setXY ((xdim - (strlen(s) * 6)) / 2, y + 0);
        lcd.printStr (s);
    }

    s = "Serial Graphic LCD";
    lcd.setXY ((xdim - (strlen(s) * 6)) / 2, y + 8);
    lcd.printStr (s);

    s = "Backpack Firmware";
    lcd.setXY ((xdim - (strlen(s) * 6)) / 2, y + 16);
    lcd.printStr (s);

    lcd.drawBox (0, 0,
                 6, 6,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (3, 3,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawBox (xdim - 6 - 1, 0,
                 xdim - 1, 6,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (xdim - 3 - 1, 3,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawBox (0, ydim - 6 - 1,
                 6, ydim - 1,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (3, ydim - 3 - 1,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawBox (xdim - 6 - 1, ydim - 6 - 1,
                 xdim - 1, ydim - 1,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (xdim - 3 - 1, ydim - 3 - 1,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawLine (7, 1, xdim - 7 - 1, 1, GLCD_MODE_NORMAL);
    lcd.drawLine (7, 3, xdim - 7 - 1, 3, GLCD_MODE_NORMAL);

    y = ydim - 1 - 1;
    lcd.drawLine (7, y, xdim - 7 - 1, y, GLCD_MODE_NORMAL);
    y -= 2;
    lcd.drawLine (7, y, xdim - 7 - 1, y, GLCD_MODE_NORMAL);

    x = 1;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);
    x += 2;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);

    x = xdim - 1 - 1;;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);
    x -= 2;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);

    lcd.drawSprite (xdim/2, 16, 0x80, GLCD_MODE_FILL|GLCD_MODE_NORMAL);

    delay(5000);
}

/////////////////////////////////////////////////////////////////////////////
// Draw the font page with proportional font
void
glcdDrawFrontPageProportional ()
{
    char *s;
    uint8_t xdim = lcd.xdim;
    uint8_t ydim = lcd.ydim;
    uint8_t x;
    uint8_t y = (ydim - (3 * 8)) / 2;

    lcd.clearScreen ();

    // Make some more space for the sprite
    if (ydim == 64)
        y += 8;                         // More space for sprite.

    lcd.fontFace (GLCD_FONT_NORMAL);
    lcd.fontMode (GLCD_MODE_NORMAL|GLCD_MODE_FONT_PROPORTIONAL);

    lcd.setString (xdim/2, y+0, GLCD_FONT_CENTER, F("Alternative Sparkfun"));
    lcd.setString (xdim/2, y+8, GLCD_FONT_CENTER, F("Serial Graphic LCD"));
    lcd.setString (xdim/2, y+16, GLCD_FONT_CENTER, F("Backpack Firmware"));

    lcd.drawBox (0, 0,
                 6, 6,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (3, 3,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawBox (xdim - 6 - 1, 0,
                 xdim - 1, 6,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (xdim - 3 - 1, 3,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawBox (0, ydim - 6 - 1,
                 6, ydim - 1,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (3, ydim - 3 - 1,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawBox (xdim - 6 - 1, ydim - 6 - 1,
                 xdim - 1, ydim - 1,
                 GLCD_MODE_FILL|GLCD_MODE_NORMAL);
    lcd.drawCircle (xdim - 3 - 1, ydim - 3 - 1,
                    2, GLCD_MODE_FILL|GLCD_MODE_XOR|GLCD_MODE_NORMAL);

    lcd.drawLine (7, 1, xdim - 7 - 1, 1, GLCD_MODE_NORMAL);
    lcd.drawLine (7, 3, xdim - 7 - 1, 3, GLCD_MODE_NORMAL);

    y = ydim - 1 - 1;
    lcd.drawLine (7, y, xdim - 7 - 1, y, GLCD_MODE_NORMAL);
    y -= 2;
    lcd.drawLine (7, y, xdim - 7 - 1, y, GLCD_MODE_NORMAL);

    x = 1;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);
    x += 2;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);

    x = xdim - 1 - 1;;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);
    x -= 2;
    lcd.drawLine (x, 7, x, ydim - 7 - 1, GLCD_MODE_NORMAL);

    lcd.drawSprite (xdim/2, 16, 0x80, GLCD_MODE_FILL|GLCD_MODE_NORMAL);

    delay(5000);
    lcd.fontMode (GLCD_MODE_NORMAL);
}
