// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : Alternative Serial Graphic LCD Firmware
//  Module        : Benchmark program
//  Object Name   : $RCSfile: AltSerialGraphicLCDBenchmark.ino,v $
//  Revision      : $Revision: 1.4 $
//  Date          : $Date: 2015/06/08 20:35:49 $
//  Author        : $Author: jon $
//  Created By    : Jon Green
//  Created       : Sat May 23 11:46:23 2015
//  Last Modified : <150608.2135>
//
//  Description   : This is a benchmark that I found on the web for serial
//                  grapic LCD benchmark. Out of curiosity will see what 
//                  we can do and try to keep the same functions that they
//                  are using. See the following reference:
//  
//                  https://www.pjrc.com/teensy/td_libs_GLCD.html
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

// Initialize an instance of the SoftwareSerial library
SoftwareSerial serial (SERIAL_RX_DPIN, SERIAL_TX_DPIN);

// Create an instance of the LCD class named LCD. We will use this instance
// to call all the subsequent LCD functions. The instance is called with a
// reference to the software serial object.
GLCD lcd(serial);

// Local variables.
unsigned long startMillis;
unsigned int loops = 0;
unsigned int iter = 0;

//////////////////////////////////////////////////////////////////////////////
// Initialisation method.
void
setup()
{
    // Start the Software serial library we run at 115200 by default.
    serial.begin(115200);

    // Reset the screen. As soon as it is reset then we can use it.
    lcd.reset();
}

void
drawSpinner (uint8_t pos, uint8_t x, uint8_t y)
{   
    // this draws an object that appears to spin
    switch (pos % 8)
    {
    case 0:
        lcd.drawLine (x, y-8, x, y+8, GLCD_MODE_NORMAL); 
        break;
    case 1:
        lcd.drawLine (x+3, y-7, x-3, y+7, GLCD_MODE_NORMAL);
        break;
    case 2:
        lcd.drawLine( x+6, y-6, x-6, y+6, GLCD_MODE_NORMAL);  
        break;
    case 3:
        lcd.drawLine( x+7, y-3, x-7, y+3, GLCD_MODE_NORMAL);  
        break;
    case 4: 
        lcd.drawLine( x+8, y, x-8, y, GLCD_MODE_NORMAL);  
        break;
    case 5:
        lcd.drawLine( x+7, y+3, x-7, y-3, GLCD_MODE_NORMAL);  
        break;
    case 6:
        lcd.drawLine( x+6, y+6, x-6, y-6, GLCD_MODE_NORMAL);  
        break; 
    case 7:
        lcd.drawLine( x+3, y+7, x-3, y-7, GLCD_MODE_NORMAL);  
        break;
    } 
}


//////////////////////////////////////////////////////////////////////////////
// Loop method - run over and over again
void
loop()
{
    iter = 0;
    startMillis = millis();
    
    // loop for one second
    while (millis() - startMillis < 1000)
    { 
        // Rectangle in left side of screen
        lcd.drawBox (0, 0, 64, 61, GLCD_MODE_NORMAL); 
        // Rounded rectangle around text area   
        lcd.drawRoundedBox (68, 0, 68+58-1, 61, 5, GLCD_MODE_NORMAL);
        for (int i = 0; i < 62; i += 4)
        {
            // draw lines from upper left down right side of rectangle  
            lcd.drawLine (1, 1, 63, i, GLCD_MODE_NORMAL);  
        }
        // draw circle centered in the left side of screen  
        lcd.drawCircle (32, 31, 30, GLCD_MODE_NORMAL);
        // clear previous spinner position  
        lcd.eraseBox (94-8, 40, 94-8+16, 40+16);
        // draw new spinner position
        drawSpinner(loops++, 94, 48);
        // locate curser for printing text
        lcd.setXY (5*6-3, 5*8+3);
        // print current iteration at the current cursor position 
        lcd.printNum(++iter);
    } 
    // display number of iterations in one second
    // clear the screen  
    lcd.clearScreen();
    // positon cursor  
    lcd.setXY(13*6,2*8);
    // print a text string
    lcd.putstr("FPS=");
    // print a number 
    lcd.printNum(iter);
}
