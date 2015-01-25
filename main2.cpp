#include "mbed.h"
#include "C12832.h"
#include "Timer.h"
// #include <deque>
#include "std.h"
#include "queue.h"
//#include "memory.h"
#include "sound.h"
#include "mixer.h"
#include "USBHostMSD.h"

DigitalOut p(LED1);
DigitalIn sound(p14);

struct s {
    const char *file;
    nat sample;
};

 struct Point {
    int x, y;
    Point(int x = -1, int y = -1) : x(x), y(y) {}
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

#define SIZE(a) (sizeof(a) / sizeof(*a))

const char *bgms[] = {
	"/usb/smg2.wav",
	"/usb/smg2-2.wav",
	"/usb/smg2-3.wav",
	"/usb/smg-gg.wav",
	"/usb/staple.wav",
};

const char *randomBgm() {
	nat id = rand() % SIZE(bgms);
	return bgms[id];
}

C12832 lcd(p5, p7, p6, p8, p11);
SQueue<Point, 60> queue;

int main()
{
    /*
        Sound section
    */
    USBHostMSD msd("usb");
	for (nat i = 0; i < 10 && !msd.connect(); i++) {
        Thread::wait(500);
    }
	bool connected = msd.connect();

	AnalogOut out(p18);
	WavFile *sfx = 0;
	startMixer(out, 20000);

    /*
        Game section
    */
    while (true) {
		WavFile *bgm = 0;
		if (connected) {
			bgm = new WavFile(randomBgm(), true);
			play(bgm);
		}


        Timer timer;
        timer.start();
        int loopTime = timer.read_ms();
		queue.clear();
        lcd.cls();
        lcd.locate(0,3);
        // std::deque<Point> queue;
        positionX = 64;
        positionY = 16;
        lcd.fillrect(positionX, positionY, positionX + 11, positionY + 3, 1);
        queue.push_front(Point(72, 16));
        queue.push_front(Point(68, 16));
        queue.push_front(Point(64, 16));
        int xpos = 0;
        int ypos = 0;
        food();
        lcd.fillrect(food_x, food_y, food_x + 3, food_y, 1);
        lcd.fillrect(food_x, food_y, food_x, food_y + 3, 1);
        lcd.fillrect(food_x + 3, food_y, food_x + 3, food_y + 3, 1);
        lcd.fillrect(food_x, food_y + 3, food_x + 3, food_y + 3, 1);
        snake_direction = LEFT;
        bool intersect = false;

        while(true) {
            if (timer.read_ms() - loopTime > 100) {
                moveSnake();

				for (nat i = 0; i < queue.size(); i++) {
					Point it = queue.at(i);
                    if(it.x == positionX && it.y == positionY) {
                        intersect = true;
                    }
				}
                // std::deque<Point>::iterator it = queue.begin();
                // while (it != queue.end()) {
                //     if(it->x == positionX && it->y == positionY) {
                //         intersect = true;
                //     }
                //     it++;
                // }
                if(positionX > 128 || positionX < 0 || positionY > 32 || positionY < 0 || intersect == true) {
                    break;
                }
                if(food_x == positionX && food_y == positionY){
                    food();
                    lcd.fillrect(positionX, positionY, positionX + 3, positionY + 3, 1);
                    lcd.fillrect(food_x, food_y, food_x + 3, food_y + 3, 0);
                    lcd.fillrect(food_x, food_y, food_x + 3, food_y, 1);
                    lcd.fillrect(food_x, food_y, food_x, food_y + 3, 1);
                    lcd.fillrect(food_x + 3, food_y, food_x + 3, food_y + 3, 1);
                    lcd.fillrect(food_x, food_y + 3, food_x + 3, food_y + 3, 1);

					if (!playing(sfx) && connected)
						play(sfx = new WavFile("/usb/sfx1_3.wav"));
                }
                else {
					Point pt = queue.pop_back();
                    xpos = pt.x;
                    ypos = pt.y;
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

		while (playing(sfx))
			Thread::wait(100);

		WavFile *win = 0;
		if (connected) {
			win = new WavFile("/usb/gameover.wav");
			stop(bgm);
			play(win);
		}
        while(true){
			Thread::wait(100);
            if(joy) {
                break;
            }
        }
		if (connected) {
			stop(win);
		}
    }
}
 
