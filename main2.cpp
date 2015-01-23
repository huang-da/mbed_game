#include "mbed.h"
#include "C12832.h"
#include "Timer.h"
#include <deque>


 struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
 };


// #define SQUARE_SIZE 3
#define COLUMNS 32
#define ROWS 8
 
BusIn joy(p15,p12,p13,p16);
DigitalIn fire(p14);

BusOut leds(LED1,LED2,LED3,LED4);

char width = 128;
char height = 32;
 
char LEFT = 0;
char RIGHT = 1;
char UP = 2;
char DOWN = 3;

int positionX = 64;
int positionY = 16;

char snake_direction = LEFT;

char food_x, food_y;

void checkButtons() {
    if( joy == 4 ) {
        if (snake_direction != RIGHT) {
            snake_direction = LEFT; 
            return;
        }
    }
    if( joy == 8 ) {
        if (snake_direction != LEFT) {
            snake_direction = RIGHT; 
            return;
        }
    }    
    if( joy == 1 ) {
        if (snake_direction != DOWN) {
            snake_direction = UP;
            return;
        }
    }
    if( joy == 2 ) {
        if(snake_direction != UP) {
            snake_direction = DOWN;
            return;
        }
    }
}
void moveSnake() {
    if (snake_direction == LEFT)
        positionX = positionX - 4;
    else if (snake_direction == RIGHT)
        positionX = positionX + 4;
    else if (snake_direction == UP)
        positionY = positionY - 4;
    else if (snake_direction == DOWN)
        positionY = positionY + 4;

}

void food() {
    food_x = rand()%COLUMNS * 4;
    food_y = rand()%ROWS * 4;
}


C12832 lcd(p5, p7, p6, p8, p11);
 
int main()
{
    while (true) {
        Timer timer;
        timer.start();
        int loopTime = timer.read_ms();

        lcd.cls();
        lcd.locate(0,3);
        std::deque<Point> queue;
        lcd.fillrect(positionX, positionY, positionX + 11, positionY + 3, 1);
        queue.push_back(Point(64, 16));
        queue.push_back(Point(68, 16));
        queue.push_back(Point(72, 16));
        int xpos = 0;
        int ypos = 0;
        food();
        lcd.fillrect(food_x, food_y, food_x + 3, food_y, 1);
        lcd.fillrect(food_x, food_y, food_x, food_y + 3, 1);
        lcd.fillrect(food_x + 3, food_y, food_x + 3, food_y + 3, 1);
        lcd.fillrect(food_x, food_y + 3, food_x + 3, food_y + 3, 1);
        positionX = 64;
        positionY = 16;
        snake_direction = LEFT;
        bool intersect = false;

        while(true) { 
            if (fire) {
                leds=0xf;
            } else {
                leds=joy;
            }
            lcd.locate(0,15);
            if (timer.read_ms() - loopTime > 300) {
                moveSnake();
                std::deque<Point>::iterator it = queue.begin();
                while (it != queue.end()) {
                    if(it->x == positionX && it->y == positionY) {
                        intersect = true;
                    }
                    it++;
                }
                if(positionX > 128 || positionX < 0 || positionY > 32 || positionY < 0 || intersect == true) {
                    break;
                }
                if(food_x == positionX && food_y == positionY){
                    food();
                    lcd.fillrect(positionX, positionY, positionX + 3, positionY + 3, 1);
                    lcd.fillrect(food_x, food_y, food_x + 3, food_y, 1);
                    lcd.fillrect(food_x, food_y, food_x, food_y + 3, 1);
                    lcd.fillrect(food_x + 3, food_y, food_x + 3, food_y + 3, 1);
                    lcd.fillrect(food_x, food_y + 3, food_x + 3, food_y + 3, 1);
                }
                else {
                    xpos = queue.back().x;
                    ypos = queue.back().y;
                    queue.pop_back();
                    lcd.fillrect(positionX, positionY, positionX + 3, positionY + 3, 1);
                    if(!(xpos == food_x && ypos == food_y))
                        lcd.fillrect(xpos, ypos, xpos + 3, ypos + 3, 0);
                }
                queue.push_front(Point(positionX, positionY));
                loopTime = timer.read_ms();

            }
            checkButtons();
        }
        lcd.cls();
        lcd.locate(45,6);
        lcd.printf("You lost");
        lcd.locate(30,20);
        lcd.printf("Snake Length: %d", queue.size());
        while(true){
            if(joy) {
                break;
            }
        }
    }
}
 