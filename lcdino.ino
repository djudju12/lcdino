#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
const int jumpb = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// make some custom characters:
byte rect_first[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B01110,
    B01110,
    B01110,
};

byte rect_second[] = {
    B00000,
    B01110,
    B01110,
    B01110,
    B00000,
    B00000,
    B00000,
    B00000
};

#define FALLING     0
#define RISING      1
#define NOT_JUMPING 2


void draw(int sprite_num, int x, int y)
{
    lcd.setCursor(x, y);
    lcd.write(sprite_num);
}

struct Rect {
    int jump_state;
    int x, y, sprite;
};

Rect rect;

void draw_rect()
{
    switch (rect.jump_state) {
        case RISING: {

            if (rect.sprite == 1) {
                rect.sprite = 2;
            } else if (rect.sprite == 2) {
                if (rect.y == 1) {
                    rect.y = 0;
                } else {
                    rect.jump_state = FALLING;
                }

                rect.sprite = 1;
            }

        } break;

        case FALLING: {

            if (rect.sprite == 1) {
                if (rect.y == 1) {
                    rect.jump_state = NOT_JUMPING;
                } else {
                    rect.sprite = 2;
                    rect.y = 1;
                }
            } else {
                rect.sprite = 1;
            }

        } break;

        case NOT_JUMPING: {
            /* do nothing for now */
        } break;
    }

    draw(rect.sprite, rect.x, rect.y);
}

void setup()
{
    // initialize LCD and set up the number of columns and rows:
    lcd.begin(16, 2);
    lcd.createChar(1, rect_first);
    lcd.createChar(2, rect_second);
    Serial.begin(9600);

    pinMode(jumpb, INPUT);

    rect = {
        .jump_state = NOT_JUMPING,
        .x = 1,
        .y = 1,
        .sprite = 1
    };

    // set the cursor to the top left
    lcd.setCursor(0, 0);
}

int i = 1;

void loop()
{
    lcd.clear();
    int jump = digitalRead(jumpb);
    Serial.print("jump=");
    Serial.println(jumpb);
    if (rect.jump_state == NOT_JUMPING) {
        if (jump == HIGH) {
            rect.jump_state = RISING;
        }
    }

    draw_rect();

    delay(100);
    Serial.println(rect.jump_state);
}