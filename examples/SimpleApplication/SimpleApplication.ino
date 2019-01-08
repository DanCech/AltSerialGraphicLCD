// -!- c++ -!- ///////////////////////////////////////////////////////////////
//
// Alternative Serial Graphic LCD Library Demo by Jon Green May 24, 2015
//
// This is a simple application to draw some text on the screen. The Sparkfun
// demo application provided a good starting page for "Hello World".
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

#include <AltSerialGraphicLCD.h>
#include <SoftwareSerial.h>

// Define the TX and RX pins used to connect the screen. Change these two pin
// values to whichever pins you wish to use (RX, TX).
//#define SERIAL_TX_DPIN    3
//#define SERIAL_RX_DPIN    2
#define SERIAL_TX_DPIN   12
#define SERIAL_RX_DPIN   10

// Initialize an instance of the SoftwareSerial library
SoftwareSerial serial (SERIAL_RX_DPIN,SERIAL_TX_DPIN);

// Create an instance of the GLCD class named glcd. This instance is used to
// call all the subsequent GLCD functions. The instance is called with a
// reference to the software serial object.
GLCD glcd(serial);

static uint32_t counter = 0;            // Counter for number of iterations.
static uint32_t start_time;             // The time we started running.

//////////////////////////////////////////////////////////////////////////////
// Perform significant initialisation.
void setup()
{
    // Start the Software serial library we run at 115200 by default.
    serial.begin(115200);

    // The first call is reset to the sceeen. This puts it into a sane state
    // irrespective of the state that we last left it in. Following a reset
    // then the screen is clear and the cursor is at location 0,0. Reset can
    // be called at any time, not just at the start.
    glcd.reset();

    // Initialise our simple clock so we can keep a time count.
    start_time = millis();
}

//////////////////////////////////////////////////////////////////////////////
// Execution loop
void loop()
{
    uint32_t diff_time;                 // Variable for the time difference
    char buffer [20];                   // Character buffer for strings
    uint8_t x_pos_1_4;                  // 1/4 of horizontal screen
    uint8_t x_pos_3_4;                  // 3/4 of horizontal screen
    uint8_t radius;                     // Radius of circle.
    
    // Work out the size of the screen and calculate the 1/4 and 3/4
    // horizontal pixel positions. 
    x_pos_1_4 = glcd.xdim / 4;
    x_pos_3_4 = x_pos_1_4 * 3;

    // Prints "Hello World" to the screen and draws a tiny world (circle) in
    // the right 1/4 of the screen.

    // "Hello" is 6 * 5 = 30 pixels long, place at 3/4 of screen at the top.
    glcd.setXY(x_pos_3_4 - 15, 0);
    glcd.printStr(F("Hello"));          // Print "Hello"

    // "World" is 6 * 5 = 30 pixels long and 8 pixels high place at 3/4 of
    // screen at the bottom.
    glcd.setXY(x_pos_3_4 - 15, glcd.ydim - 8);
    glcd.printStr(F("World"));          // Print "World"
    
    // Compute the radius of the circle, draw as large as possible
    // considering the size of the screen. 
    radius = (glcd.ydim - 24) / 2;
    if (x_pos_1_4 - 1 < radius) 
        radius = x_pos_1_4 - 1;
          
    // Draw a circle in the middle of the screen leaving 12 pixels at the top
    // and bottom of the screen.
    glcd.drawCircle (x_pos_3_4,              // Horizontal 3/4 left
                     (glcd.ydim / 2),        // Vertical middle
                     radius,                 // Radius is 1/2 remaining height.
                     GLCD_MODE_NORMAL);      // Write normally

    // Draw the Sparkfun logo sprite (sprite_id=0x80) as we know it is
    // loaded. Position 1/4 fscreen width from the left and in the middle.
    glcd.drawSprite (x_pos_1_4, glcd.ydim / 2, 0x80,
                     GLCD_MODE_CENTER|GLCD_MODE_NORMAL);

    /// Add some animation to spice it up a bit.

    // Print counter of number of iterations at top left of screen, use a
    // long number so it can run for a long time without wrapping.
    glcd.setXY(0, 0);                   // Top of screen
    sprintf (buffer, "%ld", counter++);
    glcd.printStr(buffer);

    // Print our running time at the bottom left of screen. Display hours,
    // minutes, seconds and milliseconds.
    glcd.setXY(0, glcd.ydim - 8);       // Bottom of screen - char height
    diff_time = millis() - start_time;
    sprintf (buffer, "%02d:%02d:%02d.%03d",
             (int)(diff_time / (1000L * 60L * 60L)),
             (int)((diff_time / (1000L * 60L)) % 60L),
             (int)((diff_time / 1000L) % 60L),
             (int)(diff_time % 1000L));
    glcd.printStr(buffer);
}
