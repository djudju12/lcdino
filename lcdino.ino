#include <LiquidCrystal.h>

#define RS 8
#define EN 9
#define D4 4
#define D5 5
#define D6 6
#define D7 7

#define JUMP_BUTTON 10

/* jump states */
#define FALLING     0
#define RISING      1
#define NOT_JUMPING 2

struct Rect {
    int jump_state;
    int x, y, sprite;
};

#define LEN_RECT 2
byte rect_sprites[LEN_RECT][8] = {
    {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B01110,
        B01110,
        B01110,
    },
    {
        B00000,
        B01110,
        B01110,
        B01110,
        B00000,
        B00000,
        B00000,
        B00000
    },
};

struct Enemy {
    int x;
};

#define LEN_ENEMY 1
byte enemy_sprites[LEN_ENEMY][8] = {
    {
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
        B11111,
        B11111
    },
};

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

Rect rect;
Enemy enemy;

void draw(int sprite_num, int x, int y)
{
    lcd.setCursor(x, y);
    lcd.write(sprite_num);
}

void draw_rect()
{
    draw(rect.sprite, rect.x, rect.y);
}

void update_rect()
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
}

void draw_enemy()
{
    draw(3, enemy.x, 1);
}

void update_enemy()
{
    enemy.x -= 1;
    if (enemy.x < 0) {
        enemy.x = 15;
    }
}

void setup()
{
    lcd.begin(16, 2);

    /* load all sprites */
    lcd.createChar(1, rect_sprites[0]);
    lcd.createChar(2, rect_sprites[1]);

    lcd.createChar(3, enemy_sprites[0]);

    /* set pins */
    pinMode(JUMP_BUTTON, INPUT);

    /* initiate all objects */
    rect = {
        .jump_state = NOT_JUMPING,
        .x = 1,
        .y = 1,
        .sprite = 1
    };

    enemy = {
        .x = 15,
    };


    Serial.begin(9600); // debug
}

void loop()
{
    lcd.clear();
    int jump = digitalRead(JUMP_BUTTON);
    if (rect.jump_state == NOT_JUMPING) {
        if (jump) {
            rect.jump_state = RISING;
        }
    }

    draw_rect();
    draw_enemy();

    update_rect();
    update_enemy();

    delay(100);
}