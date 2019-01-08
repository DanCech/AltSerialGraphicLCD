/* -!- C++ -!- ********************************************* Alternative
 * Graphic Serial LCD Libary Header File
 * 
 * Parts of this library are based on the original Sparkfun library by:
 *
 * Joel Bartlett 
 * SparkFun Electronics
 * 9-25-13
 * 
 * Jon Green - 205-04-01 
 * New interface and in-line definitions added 2015-04-01 and library
 * re-formatted
 * 
 * 
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
 * 
 ***************************************************************************/

#ifndef _GLCD_H_
#define _GLCD_H_

#include <Arduino.h>
#include <SoftwareSerial.h>

/////////////////////////////////////////////////////////////////////////////
// Contant character definitions.
/////////////////////////////////////////////////////////////////////////////

// Command character
#define GLCD_CHAR_CMD              ((uint8_t)(0x7c))
// XON character (start transmitting).
#define GLCD_CHAR_XON              ((uint8_t)(0x11))
// XOFF character (stop transmitting).
#define GLCD_CHAR_XOFF             ((uint8_t)(0x13))

/////////////////////////////////////////////////////////////////////////////
// Drawing mode definitions.
/////////////////////////////////////////////////////////////////////////////

// Normal
#define GLCD_MODE_NORMAL           ((uint8_t)(0x01))
// Reverse
#define GLCD_MODE_REVERSE          ((uint8_t)(0x00))
// OR mode (screen | pixel)
#define GLCD_MODE_OR               ((uint8_t)(0x02))
// XOR mode (screen ^ pixel)
#define GLCD_MODE_XOR              ((uint8_t)(0x04))
// AND mode (screen & ~pixel)
#define GLCD_MODE_NAND             ((uint8_t)(0x06))
// Perform a fill
#define GLCD_MODE_FILL             ((uint8_t)(0x08))
// Perform a fill
#define GLCD_MODE_CENTER           ((uint8_t)(0x08))
// Perform a proportial font face
#define GLCD_MODE_FONT_PROPORTIONAL ((uint8_t)(0x20))   /* Proportional font spacing */

// Font type definitions 
#define GLCD_FONT_NORMAL           0    /* Normal size font */
#define GLCD_FONT_TOM_THUMB        1    /* Small size font */

// Font positioning 
#define GLCD_FONT_CENTER           0    /* Center justification */
#define GLCD_FONT_RIGHT            1    /* Right justification */

/////////////////////////////////////////////////////////////////////////////
// Serial command definitions
/////////////////////////////////////////////////////////////////////////////
#define GLCD_CMD_CLEAR_SCREEN      ((uint8_t)(0x00))
#define GLCD_CMD_SET_BACKLIGHT     ((uint8_t)(0x02))
#define GLCD_CMD_DRAW_CIRCLE       ((uint8_t)(0x03))
#define GLCD_CMD_DEMO              ((uint8_t)(0x04))
#define GLCD_CMD_ERASE_BLOCK       ((uint8_t)(0x05))
#define GLCD_CMD_FILL_BOX          ((uint8_t)(0x06))
#define GLCD_CMD_CHANGE_BAUD_RATE  ((uint8_t)(0x07))
#define GLCD_CMD_FONT_FACE         ((uint8_t)(0x08))
#define GLCD_CMD_DRAW_ROUNDED_BOX  ((uint8_t)(0x09))
#define GLCD_CMD_FONT_MODE         ((uint8_t)(0x0a))
#define GLCD_CMD_DRAW_LINE         ((uint8_t)(0x0c))
#define GLCD_CMD_DRAW_MODE         ((uint8_t)(0x0d))
#define GLCD_CMD_DRAW_BOX          ((uint8_t)(0x0f))
#define GLCD_CMD_DRAW_PIXEL        ((uint8_t)(0x10))
#define GLCD_CMD_DRAW_LINES        ((uint8_t)(0x11))
#define GLCD_CMD_REVERSE_MODE      ((uint8_t)(0x12))
#define GLCD_CMD_TOGGLE_SPLASH     ((uint8_t)(0x13))
#define GLCD_CMD_DRAW_SPRITE       ((uint8_t)(0x14))
#define GLCD_CMD_UPLOAD_SPRITE     ((uint8_t)(0x15))
#define GLCD_CMD_BITBLT            ((uint8_t)(0x16))
#define GLCD_CMD_ECHO              ((uint8_t)(0x17))
#define GLCD_CMD_SET_X_OFFSET      ((uint8_t)(0x18))
#define GLCD_CMD_SET_Y_OFFSET      ((uint8_t)(0x19))
#define GLCD_CMD_DRAW_POLYGON      ((uint8_t)(0x1a))
#define GLCD_CMD_SET               ((uint8_t)(0x1b))
#define GLCD_CMD_QUERY             ((uint8_t)(0x1e))
#define GLCD_CMD_FACTORY_RESET     ((uint8_t)(0x1f))
#define GLCD_CMD_RESET             ((uint8_t)(0x20))
// Extended commands
#define GLCD_CMDX_GRAPHICS_ON      ((uint8_t)(0x40))
#define GLCD_CMDX_GRAPHICS_OFF     ((uint8_t)(0x41))
#define GLCD_CMDX_SET_BACKLIGHT    ((uint8_t)(0x42))
#define GLCD_CMDX_DRAW_CIRCLE      ((uint8_t)(0x43))
#define GLCD_CMDX_FILL_BOX         ((uint8_t)(0x46))
#define GLCD_CMDX_FONT_FACE        ((uint8_t)(0x48))
#define GLCD_CMDX_DRAW_ROUNDED_BOX ((uint8_t)(0x49))
#define GLCD_CMDX_DRAW_LINE        ((uint8_t)(0x4c))
#define GLCD_CMDX_DRAW_BOX         ((uint8_t)(0x4f))
#define GLCD_CMDX_DRAW_PIXEL       ((uint8_t)(0x50))
#define GLCD_CMDX_DRAW_LINES       ((uint8_t)(0x51))
#define GLCD_CMDX_REVERSE_MODE     ((uint8_t)(0x52))
#define GLCD_CMDX_SET_XY_OFFSET    ((uint8_t)(0x58))
#define GLCD_CMDX_SET_XY_STRING    ((uint8_t)(0x59))
#define GLCD_CMDX_DRAW_POLYGON     ((uint8_t)(0x5a))

//////////////////////////////////////////////////////////////////////////////
// Argument definitions
//////////////////////////////////////////////////////////////////////////////
#define GLCD_ARG_SIZEOF            0x10 /* Arg is (... <sizeof>, <ptr>) */
#define GLCD_ARG_XY_LIST           0x20 /* Arg is x,y pair list terminated with 0x80 */
#define GLCD_ARG_SPRITE_WH         0x30 /* Arg is (..., <width>, <height>, <ptr>) */
#define GLCD_ARG_SPRITE            0x40 /* Arg is (..., <sprite_ptr>) */
#define GLCD_ARG_FFSTRING          0x50 /* Arg is (..., <char *>) */
#define GLCD_ARG_TYPE_MASK         0x70 /* Arg type mask */
#define GLCD_ARG_PROGMEM           0x80 /* Argument in program memory */

// The Set LCD check byte used as the 1st parameter with GLCD_CMD_SET
#define GLCD_CMD_SET_CHECKBYTE     0xc5

//////////////////////////////////////////////////////////////////////////////
// The query/set identities
//////////////////////////////////////////////////////////////////////////////
#define GLCD_ID_MAGIC              0x00 /* Magic number to handle new install */
#define GLCD_ID_BAUDRATE           0x01 /* Baud rate */
#define GLCD_ID_BACKLIGHT          0x02 /* Backlight level */
#define GLCD_ID_SPLASH             0x03 /* Splash screen enabled */
#define GLCD_ID_REVERSE            0x04 /* Reverse the screen */
#define GLCD_ID_DEBUG              0x05 /* Reserved for future use */
#define GLCD_ID_CRLF               0x06 /* Line ending CR+LF */
#define GLCD_ID_XON_POS            0x07 /* XON position */
#define GLCD_ID_XOFF_POS           0x08 /* XOFF position */
#define GLCD_ID_SCROLL             0x09 /* Scroll on/off */
#define GLCD_ID_LARGE_SCREEN       0x0a /* Large screen */
#define GLCD_ID_FONT               0x0b /* Selected characterset */

#define GLCD_ID_VERSION_MAJOR      0x20 /* Version number major */
#define GLCD_ID_VERSION_MINOR      0x21 /* Version number minor */
#define GLCD_ID_EEPROM_SPRITE_SIZE 0x22 /* EEPROM sprite size in bytes (Read only) */
#define GLCD_ID_EEPROM_SPRITE_NUM  0x23 /* Number of EEPROM sprites (Read only) */
#define GLCD_ID_RAM_SPRITE_SIZE    0x24 /* RAM sprite size in bytes (Read only) */
#define GLCD_ID_RAM_SPRITE_NUM     0x25 /* Number of RAM sprites (Read only) */

#define GLCD_ID_X_DIMENSION        0x40 /* Screen X dimension (Read only) */
#define GLCD_ID_Y_DIMENSION        0x41 /* Screen Y dimension (Read only) */

#define GLCD_ID_ESPRITE_WIDTH_0    0x80 /* EEPROM sprite[0] width (Read only) */
#define GLCD_ID_ESPRITE_HEIGHT_0   0x81 /* EEPROM sprite[0] height (Read only) */
// For EEPROM sprite[1..n] then add 2 for each sprite.
// i.e. sprite[4].width = (GLCD_ID_ESPRITE_WIDTH_0 + (4*2))

/// LCD class.
/// A lot of the methods are other calls with no processing so they are
/// mapped immediataly rather than nesting function calls.
class GLCD
{
private:
    // The handle of the Software Serial object which we are using for
    // communications.
    SoftwareSerial &serial;

    // The current XON/XOFF state
    uint8_t blocked;

    // Running in graphics mode with shortened commands.
    uint8_t graphics_on;

    // Mask determines how often we check for XON/XOFF. Allow 31 bytes to be
    // sent before checking XON/XOFF.
    const uint8_t xoff_mask = 31;

    // The end of line character.
    char const PROGMEM *crlf;

public:
    //////////////////////////////////////////////////////////////////////////
    // The x screen dimension (width). This is only valid after a reset().
    uint8_t xdim;

    //////////////////////////////////////////////////////////////////////////
    // The y screen dimension (height). This is only valid after a reset().
    uint8_t ydim;

    //////////////////////////////////////////////////////////////////////////
    /// Constructor.
    ///
    /// @param [in] software_serial The handle of the software serial object
    GLCD (SoftwareSerial& software_serial);

#ifdef GLCD_GET_IS_REQUIRED             // We do not use this disable.
    //////////////////////////////////////////////////////////////////////////
    /// Get a character from the screen.
    ///
    /// @return Character read from the screen or -1 when there are no
    ///         characters.
    int get (void);
#endif                                  // End we do not use this disable

    /////////////////////////////////////////////////////////////////////////
    /// Wait for the screen to become ready to send a character. This
    /// performs a XON/XOFF check and blocks until the screen is ready.
    ///
    void ready (void);

    //////////////////////////////////////////////////////////////////////////
    // Wait for a character for the specified number of milliseconds.
    ///
    /// @param [in] expected The character that is expected or 0 for any.
    /// @param [in] msdely The number of milliseonds to wait for the character.
    ///
    /// @return Character read from the screen or -1 when there are no
    ///         characters.
    int waitc (uint8_t expected, int msdelay);

    //////////////////////////////////////////////////////////////////////////
    /// Put a character to the screen. This is the fundemental call for the
    /// whole of the library and manages XON/OXFF to ensure that the screen
    /// buffer does not overflow. Noted I wanted to call this putc but there
    /// is a macro that prevents this.
    ///
    /// @param cc [in] The charcter to put to the sceeen
    void put (uint8_t cc);

    //////////////////////////////////////////////////////////////////////////
    /// Put a command to the screen. This call constructs the command and
    /// manages the composition of the command on the serial. It manages XON
    /// and XOFF to ensure that the command does not overflow.
    ///
    /// @param [in] cmd The command to send
    /// @param [in] argc Count of the arguments to follow or'ed with flags
    /// @param [in] ...  variable argc arguments of uint8_t bytes
    void putcmd (uint8_t cmd, uint8_t argc, ... );

    /////////////////////////////////////////////////////////////////////////
    /// Put a nil terminated string to the screen. The call manages XON and
    /// XOFF to ensure that the command does not overflow.
    ///
    /// @param [in] s The string to draw from memory.
    ///
    void putstr (char *s);

    /////////////////////////////////////////////////////////////////////////
    /// Put a nil terminated string to the screen. The call manages XON and
    /// XOFF to ensure that the command does not overflow.
    ///
    /// @param [in] s The string to draw from memory.
    ///
    void printStr(char *s)
    {
        this->putstr (s);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Put a nil terminated string to the screen. The call manages XON and
    /// XOFF to ensure that the command does not overflow.
    ///
    /// @param [in] s The string to draw from flash memory.
    ///
    void putstr_P (const char PROGMEM *s);

    /////////////////////////////////////////////////////////////////////////
    /// Put a nil terminated string to the screen. The call manages XON and
    /// XOFF to ensure that the command does not overflow.
    ///
    /// @param [in] s The string to draw from flash memory.
    ///
    void putstr (const __FlashStringHelper *s)
    {
        this->putstr_P ((const char PROGMEM *)s);
    }

    /////////////////////////////////////////////////////////////////////////
    /// Put a nil terminated string to the screen. The call manages XON and
    /// XOFF to ensure that the command does not overflow.
    ///
    /// @param [in] s The string to draw from flash memory.
    ///
    void printStr(const __FlashStringHelper *s)
    {
        this->putstr (s);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Convert an integer to ASCII and send to the screen.
    ///
    /// @param [in] num The integer to print.
    ///
    void printNum(int num)
    {
        this->ready();                  // Ready to write
        serial.print(num);              // Send the data.
    };

    //////////////////////////////////////////////////////////////////////////
    /// Send a new line to the screen.
    ///
    void nextLine()
    {
        this->putstr_P (this->crlf);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Write a character block from RAM to the screen.
    ///
    /// @param [in] data The pointer to the data to write.
    /// @param [in] length The length of the data to write in bytes.
    ///
    void write (uint8_t *data, int length);

    //////////////////////////////////////////////////////////////////////////
    /// Write a character block from flash memory to the screen.
    ///
    /// @param [in] data The pointer to the data to write.
    /// @param [in] length The length of the data to write in bytes.
    ///
    void write_P (const uint8_t PROGMEM *data, int length);

    //////////////////////////////////////////////////////////////////////////
    /// Query the screen for information.
    ///
    /// @param [in] id The identitiy of the datum to retrieve
    ///
    /// @return The data associated with the identity or -1 on error.
    ///
    int query (uint8_t id);

    //////////////////////////////////////////////////////////////////////////
    /// Change the baud rate of the screen and the serial.
    ///
    /// @param [in] baud The baud rate to use.
    ///                  1=4800, 2=9600, 3=19200, 4=38400, 5=57600, 6=115200
    ///
    void setBaud(uint8_t baud);

    //////////////////////////////////////////////////////////////////////////
    /// Change the baud rate of the screen and serial ports to 115200
    ///
    void restoreDefaultBaud();

    //////////////////////////////////////////////////////////////////////////
    /// Change the graphics mode setting.
    ///
    /// @param [in] mode The new graphics mode 0=off, 1=on.
    ///
    void setGraphics (uint8_t mode);

    //////////////////////////////////////////////////////////////////////////
    /// Hard reset the screen.
    ///
    void reset ();

    /////////////////////////////////////////////////////////////////////////
    /// Send a character to be echo'ed back on the serial line for
    /// synchronisation and wait for a response from the screen.
    ///
    /// @param [in] echo_char The character to return on the serial TX line.
    /// @param [in] msdelay The number of milliseconds to wait for a response.
    ///
    /// @return The echo'ed echaracter or -1 if the call timed out with no
    ///         response from the screen.
    ///
    int echoWait (uint8_t echo_char, int msdelay);

    //////////////////////////////////////////////////////////////////////////
    /// Clear the screen
    ///
    void clearScreen()
    {
        this->putcmd (GLCD_CMD_CLEAR_SCREEN, 0);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Reverse the display and store the reverse mode persistently such that
    /// it is stored through a power cycle.
    ///
    void toggleReverseMode()
    {
        this->putcmd (GLCD_CMD_REVERSE_MODE, 0);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Reverse the display and store the reverse mode non-persistently such
    /// that it is not stored through a power cycle.
    ///
    /// @param [in] mode The revese (0) or normal (1) mode.
    ///
    void reverseMode(uint8_t mode)
    {
        this->putcmd (GLCD_CMDX_REVERSE_MODE, 1, mode);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Change the CRLF behaviour. The value is stored persistantly. 
    ///
    void setCRLF(uint8_t state)
    {
        this->putcmd (GLCD_CMD_SET, 3, GLCD_CMD_SET_CHECKBYTE, GLCD_ID_CRLF, state);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Change the scroll behaviour. The value is stored persistantly. 
    ///
    void setScroll(uint8_t state)
    {
        this->putcmd (GLCD_CMD_SET, 3, GLCD_CMD_SET_CHECKBYTE, GLCD_ID_SCROLL, state);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Change the Xon position. The value is stored persistantly. 
    ///
    void setXon(uint8_t position)
    {
        this->putcmd (GLCD_CMD_SET, 3, GLCD_CMD_SET_CHECKBYTE, GLCD_ID_XON_POS, position);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Change the Xoff position. The value is stored persistantly. 
    ///
    void setXoff(uint8_t position)
    {
        this->putcmd (GLCD_CMD_SET, 3, GLCD_CMD_SET_CHECKBYTE, GLCD_ID_XOFF_POS, position);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Change the splash screen. The value is stopred persistantly. Each
    /// invocation will move to the next splash screen state.
    ///
    void toggleSplash()
    {
        this->putcmd (GLCD_CMD_TOGGLE_SPLASH, 0);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Change the backlight level and write setting to EEPROM
    ///
    /// @param [in] duty The brightness 0 (off) to 100 (max)
    ///
    void setBacklight(uint8_t duty)
    {
        this->putcmd (GLCD_CMD_SET_BACKLIGHT, 1, duty);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Change the backlight level and do not write setting to EEPROM
    ///
    /// @param [in] duty The brightness 0 (off) to 100 (max)
    ///
    void updateBacklight(uint8_t duty)
    {
        this->putcmd (GLCD_CMDX_SET_BACKLIGHT, 1, duty);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Modify the x and y position for character rendering to (0,0). The
    /// character is rendered with the top left corner of the character drawn
    /// at the specified (x,y) coordinate.
    ///
    void setHome()
    {
        this->setXY(0,0);
    }

    /////////////////////////////////////////////////////////////////////////
    /// Modify the x and y position for character rendering. The character is
    /// rendered with the top left corner of the character drawn at the
    /// specified (x,y) coordinate.
    ///
    /// @param [in] posX The new x-coordinate position.
    ///
    void setX(uint8_t posX)
    {
        this->putcmd (GLCD_CMD_SET_X_OFFSET, 1, posX);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Modify the x and y position for character rendering. The character is
    /// rendered with the top left corner of the character drawn at the
    /// specified (x,y) coordinate.
    ///
    /// @param [in] posY The new y-coordinate position.
    ///
    void setY(uint8_t posY)
    {
        this->putcmd (GLCD_CMD_SET_Y_OFFSET, 1, posY);
    };

    /////////////////////////////////////////////////////////////////////////
    /// Modify the x and y position for character rendering. The character is
    /// rendered with the top left corner of the character drawn at the
    /// specified (x,y) coordinate.
    ///
    /// @param [in] posX The new x-coordinate position.
    /// @param [in] posY The new y-coordinate position.
    ///
    void setXY(uint8_t posX, uint8_t posY)
    {
        this->putcmd (GLCD_CMDX_SET_XY_OFFSET, 2, posX, posY);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Modify the x and y position for character rendering of a string. The
    /// string is rendered at the X position modified by the justification
    /// setting and Y position. The call is used for positioning a
    /// proportional font label.
    /// 
    /// @param [in] posX The new x-coordinate position.
    /// @param [in] posY The new y-coordinate position.
    /// @param [in] justification The string justification 0=center, 1=right
    /// @param [in] s A pointer to a nil terminated text string.
    ///
    void setString(uint8_t posX, uint8_t posY, uint8_t justification, char *s)
    {
        this->putcmd (GLCD_CMDX_SET_XY_STRING, GLCD_ARG_FFSTRING|3, posX, posY, justification, s);
    };
    
    /////////////////////////////////////////////////////////////////////////
    /// Modify the x and y position for character rendering of a string. The
    /// string is rendered at the X position modified by the justification
    /// setting and Y position. The call is used for positioning a
    /// proportional font label.
    ///
    /// @param [in] posX The new x-coordinate position.
    /// @param [in] posY The new y-coordinate position.
    /// @param [in] justification The justification position.
    /// @param [in] s A pointer to a nil terminated text string.
    ///
    void setString(uint8_t posX, uint8_t posY, uint8_t justification, const __FlashStringHelper *s)
    {
        this->putcmd (GLCD_CMDX_SET_XY_STRING, GLCD_ARG_PROGMEM|GLCD_ARG_FFSTRING|3, 
                      posX, posY, justification, (const char PROGMEM *)(s));
    };
    
    /////////////////////////////////////////////////////////////////////////
    /// Modify the x and y position for character rendering of a string. The
    /// string is rendered at the X position modified by the justification
    /// setting and Y position. The call is used for positioning a
    /// proportional font label.
    ///
    /// @param [in] posX The new x-coordinate position.
    /// @param [in] posY The new y-coordinate position.
    /// @param [in] justification The justification position.
    /// @param [in] s A pointer to a nil terminated text string.
    ///
    void setString_P(uint8_t posX, uint8_t posY, uint8_t justification, const char PROGMEM *s)
    {
        this->putcmd (GLCD_CMDX_SET_XY_STRING, GLCD_ARG_PROGMEM|GLCD_ARG_FFSTRING|3, 
                      posX, posY, justification, s);
    };
    
    /////////////////////////////////////////////////////////////////////////
    /// Demonstration. Draw the splash screen for the demo. The splash screen
    /// preference determines what is drawn at start up.
    ///
    void demo()
    {
        this->putcmd (GLCD_CMD_DEMO, 0);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a single pixel.
    ///
    /// @param [in] x1 The x-coordinate.
    /// @param [in] y1 The y-coordingate.
    /// @param [in] mode The drawing mode of the pixel.
    ///
    void setPixel(uint8_t x, uint8_t y, uint8_t mode)
    {
        this->putcmd (GLCD_CMD_DRAW_PIXEL, 3, x, y, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a single pixel.
    ///
    /// @param [in] x1 The x-coordinate.
    /// @param [in] y1 The y-coordingate.
    /// @param [in] mode The drawing mode of the pixel.
    ///
    void drawPixel(uint8_t x, uint8_t y, uint8_t mode)
    {
        this->setPixel(x, y, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a single pixel.
    ///
    /// @param [in] x1 The x-coordinate.
    /// @param [in] y1 The y-coordingate.
    ///
    void setPixel(uint8_t x, uint8_t y)
    {
        this->putcmd (GLCD_CMDX_DRAW_PIXEL, 2, x, y);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a single pixel.
    ///
    /// @param [in] x1 The x-coordinate.
    /// @param [in] y1 The y-coordingate.
    ///
    void drawPixel(uint8_t x, uint8_t y)
    {
        this->setPixel(x, y);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Change the current draw mode
    ///
    /// @param [in] mode The new drawing mode.
    ///
    void drawMode (uint8_t mode)
    {
        this->putcmd (GLCD_CMD_DRAW_MODE, 1, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a line.
    ///
    /// @param [in] x1 The start x-coordinate.
    /// @param [in] y1 The start y-coordingate.
    /// @param [in] x2 The end x-coordinate.
    /// @param [in] y2 The end y-coordinate
    /// @param [in] mode The drawing mode of the line.
    ///
    void drawLine(uint8_t x1, uint8_t y1,
                  uint8_t x2, uint8_t y2, uint8_t mode)
    {
        this->putcmd (GLCD_CMD_DRAW_LINE, 5, x1, y1, x2, y2, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a line.
    ///
    /// @param [in] x1 The start x-coordinate.
    /// @param [in] y1 The start y-coordingate.
    /// @param [in] x2 The end x-coordinate.
    /// @param [in] y2 The end y-coordinate
    ///
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
    {
        this->putcmd (GLCD_CMDX_DRAW_LINE, 5, x1, y1, x2, y2);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a box. The box is described by a diagonal line from x, y1 to x2,
    /// y2.
    ///
    /// @param [in] x1 The upper left x-coordinate.
    /// @param [in] y1 The upper left y-coordingate.
    /// @param [in] x2 The lower right x-coordinate.
    /// @param [in] y2 The lower right y-coordinate
    /// @param [in] mode The drawing mode of the line.
    ///
    void drawBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode)
    {
        this->putcmd (GLCD_CMD_DRAW_BOX, 5, x1, y1, x2, y2, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a box. The box is described by a diagonal line from x, y1 to x2,
    /// y2.
    ///
    /// @param [in] x1 The upper left x-coordinate.
    /// @param [in] y1 The upper left y-coordingate.
    /// @param [in] x2 The lower right x-coordinate.
    /// @param [in] y2 The lower right y-coordinate
    ///
    void drawBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
    {
        this->putcmd (GLCD_CMD_DRAW_BOX, 4, x1, y1, x2, y2);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a box and fills it with a pattern. The box is described by a
    /// diagonal line from x, y1 to x2, y2
    ///
    /// @param [in] x1 The upper left x-coordinate.
    /// @param [in] y1 The upper left y-coordingate.
    /// @param [in] x2 The lower right x-coordinate.
    /// @param [in] y2 The lower right y-coordinate
    /// @param [in] pattern The vertical pattern to use as fill pattern.
    ///
    void fillBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pattern)
    {
        this->putcmd (GLCD_CMD_FILL_BOX, 5, x1, y1, x2, y2, pattern);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a box and fills it in the current draw mode. The box is
    /// described by a diagonal line from x, y1 to x2, y2
    ///
    /// @param [in] x1 The upper left x-coordinate.
    /// @param [in] y1 The upper left y-coordingate.
    /// @param [in] x2 The lower right x-coordinate.
    /// @param [in] y2 The lower right y-coordinate
    ///
    void fillBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
    {
        this->putcmd (GLCD_CMDX_FILL_BOX, 4, x1, y1, x2, y2);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draw a circle.
    ///
    /// @param [in] x The x-coordinate of the centre of the circle.
    /// @param [in] y The x-coordinate of the centre of the circle.
    /// @param [in] rad The radius of the circle.
    /// @param [in] mode The drawing mode of the line.
    ///
    void drawCircle(uint8_t x, uint8_t y, uint8_t rad, uint8_t mode)
    {
        this->putcmd (GLCD_CMD_DRAW_CIRCLE, 4, x, y, rad, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draw a circle.
    ///
    /// @param [in] x The x-coordinate of the centre of the circle.
    /// @param [in] y The x-coordinate of the centre of the circle.
    /// @param [in] rad The radius of the circle.
    ///
    void drawCircle(uint8_t x, uint8_t y, uint8_t rad)
    {
        this->putcmd (GLCD_CMDX_DRAW_CIRCLE, 3, x, y, rad);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a rounded corner box. The box is described by a diagonal line
    /// from x, y1 to x2, y2
    ///
    /// @param [in] x1 The upper left x-coordinate.
    /// @param [in] y1 The upper left y-coordingate.
    /// @param [in] x2 The lower right x-coordinate.
    /// @param [in] y2 The lower right y-coordinate
    /// @param [in] radius The radius of the corner.
    /// @param [in] mode The mode to draw the line
    ///
    void drawRoundedBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                        uint8_t radius, uint8_t mode)
    {
        this->putcmd (GLCD_CMD_DRAW_ROUNDED_BOX, 6, x1, y1, x2, y2,
                      radius, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draws a rounded corner box. The box is described by a diagonal line
    /// from x, y1 to x2, y2
    ///
    /// @param [in] x1 The upper left x-coordinate.
    /// @param [in] y1 The upper left y-coordingate.
    /// @param [in] x2 The lower right x-coordinate.
    /// @param [in] y2 The lower right y-coordinate
    /// @param [in] radius The radius of the corner.
    ///
    void drawRoundedBox(uint8_t x1, uint8_t y1,
                        uint8_t x2, uint8_t y2, uint8_t radius)
    {
        this->putcmd (GLCD_CMDX_DRAW_ROUNDED_BOX, 5, x1, y1, x2, y2, radius);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Draw an image in the screen from memory.
    ///
    /// @param [in] x The top left x-coordinate.
    /// @param [in] y The top left y-coordinate.
    /// @param [in] sprite_id The identity of the sprite to draw.
    /// @param [in] mode The drawing mode of the line.
    ///
    void drawSprite(uint8_t x, uint8_t y, uint8_t sprite_id, uint8_t mode)
    {
        this->putcmd (GLCD_CMD_DRAW_SPRITE, 4, x, y, sprite_id, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Load a sprite from memory into the display.
    ///
    /// @param [in] id The identity of the sprite to load.
    /// @param [in] sprite A pointer to the sprite in memory.
    ///
    void loadSprite (uint8_t id, uint8_t *sprite)
    {
        this->putcmd (GLCD_CMD_UPLOAD_SPRITE, GLCD_ARG_SPRITE|1, id, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Load a sprite from memory into the display.
    ///
    /// @param [in] id The identity of the sprite to load.
    /// @param [in] width The width of the image in pixels.
    /// @param [in] height The height of the image in pixels.
    /// @param [in] sprite_pixels Pointer to sprite pixels in memory.
    ///
    void loadSprite (uint8_t id, uint8_t width,
                     uint8_t height, uint8_t *sprite)
    {
        this->putcmd (GLCD_CMD_UPLOAD_SPRITE,
                      GLCD_ARG_SPRITE_WH|1, id, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Load a sprite from memory into the display.
    ///
    /// @param [in] id The identity of the sprite to load.
    /// @param [in] length The length of the sprite in bytes.
    /// @param [in] sprite A pointer to the sprite in memory.
    ///
    void loadSprite (uint8_t id, int length, uint8_t *sprite)
    {
        this->putcmd (GLCD_CMD_UPLOAD_SPRITE,
                      GLCD_ARG_SIZEOF|1, id, length, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Load a sprite from flash memory into the display.
    ///
    /// @param [in] id The identity of the sprite to load.
    /// @param [in] sprite A pointer to the sprite in flash memory.
    ///
    void loadSprite_P (uint8_t id, const uint8_t PROGMEM *sprite)
    {
        this->putcmd (GLCD_CMD_UPLOAD_SPRITE,
                      GLCD_ARG_PROGMEM|GLCD_ARG_SPRITE|1, id, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Load a sprite from flash memory into the display.
    ///
    /// @param [in] id The identity of the sprite to load.
    /// @param [in] width The width of the image in pixels.
    /// @param [in] height The height of the image in pixels.
    /// @param [in] sprite_pixels Pointer to sprite pixels in flash memory.
    ///
    void loadSprite_P (uint8_t id, uint8_t width, uint8_t height,
                       const uint8_t PROGMEM *sprite_pixels)
    {
        this->putcmd (GLCD_CMD_UPLOAD_SPRITE,
                      GLCD_ARG_PROGMEM|GLCD_ARG_SPRITE_WH|1, id,
                      width, height, sprite_pixels);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Load a sprite from flash memory into the display.
    ///
    /// @param [in] id The identity of the sprite to load.
    /// @param [in] length The length of the sprite in bytes.
    /// @param [in] sprite A pointer to the sprite in flash memory.
    ///
    void loadSprite_P (uint8_t id, int length, const uint8_t PROGMEM *sprite)
    {
        this->putcmd (GLCD_CMD_UPLOAD_SPRITE,
                      GLCD_ARG_SIZEOF|1, id, length, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw an image in the screen from memory.
    ///
    /// @param [in] x The top left x-coordinate.
    /// @param [in] y The top left y-coordinate.
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] sprite A pointer to the sprite in memory.
    ///
    void bitblt (uint8_t x, uint8_t y, uint8_t mode, uint8_t *sprite)
    {
        this->putcmd (GLCD_CMD_BITBLT, GLCD_ARG_SPRITE|3, x, y, mode, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw an image in the screen from memory.
    ///
    /// @param [in] x The top left x-coordinate.
    /// @param [in] y The top left y-coordinate.
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] width The width of the image in pixels.
    /// @param [in] height The height of the image in pixels.
    /// @param [in] sprite_pixels A pointer to sprite pixel data in memory.
    ///
    void bitblt (uint8_t x, uint8_t y, uint8_t mode,
                 uint8_t width, uint8_t height, uint8_t *sprite_pixels)
    {
        this->putcmd (GLCD_CMD_BITBLT, GLCD_ARG_SPRITE_WH|3, x, y, mode,
                      width, height, sprite_pixels);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw an image in the screen from memory.
    ///
    /// @param [in] x The top left x-coordinate.
    /// @param [in] y The top left y-coordinate.
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] length The length of the sprite in bytes.
    /// @param [in] sprite A pointer to the sprite in memory.
    ///
    void bitblt (uint8_t x, uint8_t y, uint8_t mode, int length, uint8_t *sprite)
    {
        this->putcmd (GLCD_CMD_BITBLT, GLCD_ARG_SIZEOF|3, x, y, mode,
                      length, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw an image in the screen from Flash memory.
    ///
    /// @param [in] x The top left x-coordinate.
    /// @param [in] y The top left y-coordinate.
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] sprite A pointer to the sprite in flash memory.
    ///
    void bitblt_P (uint8_t x, uint8_t y, uint8_t mode,
                   const uint8_t PROGMEM *sprite)
    {
        this->putcmd (GLCD_CMD_BITBLT,
                      GLCD_ARG_PROGMEM|GLCD_ARG_SPRITE|3, x, y, mode, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw an image in the screen from Flash memory.
    ///
    /// @param [in] x The top left x-coordinate.
    /// @param [in] y The top left y-coordinate.
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] width The width of the image in pixels.
    /// @param [in] height The height of the image in pixels.
    /// @param [in] sprite A pointer to the sprite pixel data in flash memory.
    ///
    void bitblt_P (uint8_t x, uint8_t y, uint8_t mode, uint8_t width,
                   uint8_t height, const uint8_t PROGMEM *sprite_pixels)
    {
        this->putcmd (GLCD_CMD_BITBLT,
                      GLCD_ARG_PROGMEM|GLCD_ARG_SPRITE_WH|3,
                      x, y, mode, width, height, sprite_pixels);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw an image in the screen from Flash memory.
    ///
    /// @param [in] x The top left x-coordinate.
    /// @param [in] y The top left y-coordinate.
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] length The length of the sprite in bytes.
    /// @param [in] sprite A pointer to the sprite in flash memory.
    ///
    void bitblt_P (uint8_t x, uint8_t y, uint8_t mode,
                   int length, const uint8_t PROGMEM *sprite)
    {
        this->putcmd (GLCD_CMD_BITBLT, GLCD_ARG_PROGMEM|GLCD_ARG_SIZEOF|3,
                      x, y, mode, length, sprite);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a polygon with coordinates defined in memory.
    ///
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawPolygon (uint8_t mode, uint8_t *xylist)
    {
        this->putcmd (GLCD_CMD_DRAW_POLYGON, GLCD_ARG_XY_LIST|1, mode, xylist);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a polygon with coordinates defined in Flash memory.
    ///
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawPolygon_P (uint8_t mode, const uint8_t PROGMEM *xylist)
    {
        this->putcmd (GLCD_CMD_DRAW_POLYGON,
                      GLCD_ARG_PROGMEM|GLCD_ARG_XY_LIST|1, mode, xylist);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a polygon with coordinates defined in memory.
    ///
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawPolygon (uint8_t *xylist)
    {
        this->putcmd (GLCD_CMDX_DRAW_POLYGON, GLCD_ARG_XY_LIST|0, xylist);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a polygon with coordinates defined in Flash memory.
    ///
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawPolygon_P (const uint8_t PROGMEM *xylist)
    {
        this->putcmd (GLCD_CMDX_DRAW_POLYGON,
                      GLCD_ARG_PROGMEM|GLCD_ARG_XY_LIST|0, xylist);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a series of connected lines defined in memory.
    ///
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawLines (uint8_t mode, uint8_t *xylist)
    {
        this->putcmd (GLCD_CMD_DRAW_LINES,
                      GLCD_ARG_XY_LIST|1, mode, xylist);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a series of connected lines defined in Flash memory.
    ///
    /// @param [in] mode The drawing mode of the line.
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawLines_P (uint8_t mode, const uint8_t PROGMEM *xylist)
    {
        this->putcmd (GLCD_CMD_DRAW_LINES,
                      GLCD_ARG_PROGMEM|GLCD_ARG_XY_LIST|1, mode, xylist);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a series of connected lines defined in memory.
    ///
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawLines (uint8_t *xylist)
    {
        this->putcmd (GLCD_CMDX_DRAW_LINES, GLCD_ARG_XY_LIST|0, xylist);
    }

    //////////////////////////////////////////////////////////////////////////
    /// Draw a series of connected lines defined in Flash memory.
    ///
    /// @param [in] xylist A pointer to a list of (x,y) coordinate pairs. The
    ///                    end of the list is terminated with y|0x80.
    ///
    void drawLines_P (const uint8_t PROGMEM *xylist)
    {
        this->putcmd (GLCD_CMDX_DRAW_LINES,
                      GLCD_ARG_PROGMEM|GLCD_ARG_XY_LIST|0, xylist);
    }

    /////////////////////////////////////////////////////////////////////////
    /// Send a character to be echo'ed back on the serial line for
    /// synchronisation.
    ///
    /// @param [in] echo_char The character to return on the serial TX line.
    ///
    int echo (uint8_t echo_char)
    {
        this->putcmd (GLCD_CMD_ECHO, 1, echo_char);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Erase a rectangular block of the screen to the background colour.
    ///
    /// @param [in] x1 The top left x-coordinate.
    /// @param [in] y1 The top left y-coordinate.
    /// @param [in] x2 The bottom right x-coordinate.
    /// @param [in] y2 The bottom right y-coordinate.
    ///
    void eraseBox (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
    {
        this->eraseBlock (x1, y1, x2, y2);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Erase a rectangular block of the screen to the background colour.
    ///
    /// @param [in] x1 The top left x-coordinate.
    /// @param [in] y1 The top left y-coordinate.
    /// @param [in] x2 The bottom right x-coordinate.
    /// @param [in] y2 The bottom right y-coordinate.
    ///
    void eraseBlock (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
    {
        this->putcmd (GLCD_CMD_ERASE_BLOCK, 4, x1, y1, x2, y2);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Perform a factory reset on EEPROM.
    ///
    void factoryReset ()
    {
        this->putcmd (GLCD_CMD_FACTORY_RESET, 0);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Changes the font drawing mode.
    ///
    /// @param [in] mode The drawing mode for characters.
    ///
    void fontMode (uint8_t mode)
    {
        this->putcmd (GLCD_CMD_FONT_MODE, 1, mode);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Changes the font face temporarily.
    ///
    /// @param [in] charset The character set to use.
    ///
    void fontFace (uint8_t charset)
    {
        this->putcmd (GLCD_CMDX_FONT_FACE, 1, charset);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Changes the font face perminantly.
    ///
    /// @param [in] charset The character set to use.
    ///
    void setFontFace (uint8_t charset)
    {
        this->putcmd (GLCD_CMD_FONT_FACE, 1, charset);
    };

    //////////////////////////////////////////////////////////////////////////
    /// Set the value of a EEPROM location. The value is stored in EEPROM
    ///
    /// @param [in] id The identiy of the EEPROM location to change.
    /// @param [in] value The value to assign to the EEPROM location.
    ///
    void set (uint8_t id, uint8_t value)
    {
        this->putcmd (GLCD_CMD_SET, 3, GLCD_CMD_SET_CHECKBYTE, id, value);
    };
};

#endif  /* _GLCD_H_ */
