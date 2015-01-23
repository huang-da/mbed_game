#include display.h

const uint16_t Display::Black = Display::rgbToShort(0, 0, 0);

unsigned short Display::rgbToShort(unsigned char r, unsigned char g, unsigned char b) {
    //return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3)  ;
    unsigned short red = r;
    unsigned short green = g;
    unsigned short blue = b;
 
    red >>= 3;
    green >>= 2;
    blue >>= 3;
 
    red &= 0x1F;
    green &= 0x3F;
    blue &= 0x1F;
 
    red <<= 8;
    blue <<= 3;
    green = ((green & 0x7) << 13) + ((green & 0x38) >> 3);
 
    return red | green | blue;
    
}