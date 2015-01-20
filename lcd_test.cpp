#include "mbed.h"
#include "LCD/C12832.h"


C12832 lcd(p5, p7, p6, p8, p11);

int main()
{
    int j=0;
    lcd.cls();
    lcd.locate(0,3);
    lcd.printf("mbed application board!");

    while(true) {   // this is the third thread
        lcd.locate(0,15);
        lcd.printf("Counting : %d",j);
        j++;
        wait(1.0);
    }
}
