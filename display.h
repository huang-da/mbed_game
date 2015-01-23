#include "mbed.h"
 
#pragma once
 
class DisplayN18 {
    //static const unsigned char STEP = 4;
    
    DigitalOut resetPin;
 
    void reset();
    void init();
    
    public:
        Display();
        
    static const uint16_t Black;
    
        static const unsigned int WIDTH = 128;
        static const unsigned int HEIGHT = 32;
        static const unsigned char CHAR_WIDTH = 2;
        static const unsigned char CHAR_HEIGHT = 2
        //static const unsigned char CHAR_SPACING = ?;
 
        static unsigned short rgbToShort(unsigned char r, unsigned char g, unsigned char b);

};