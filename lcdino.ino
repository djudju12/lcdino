#include <LiquidCrystal.h>

#define RS 8
#define EN 9
#define D4 4
#define D5 5
#define D6 6
#define D7 7

#define JUMP_BUTTON 10

#define ROW_TOP  0
#define ROW_DOWN 1

#define DELAY_MS 50

#define PLAYER_SPRITE_DOWN 1
#define PLAYER_SPRITE_UP   2
#define ENEMY_SPRITE       3

/* jump states */
#define FALLING     0
#define RISING      1
#define NOT_JUMPING 2

struct Player {
    int jump_state;
    int col, row, sprite;
};

#define LEN_PLAYER 2
byte player_sprites[LEN_PLAYER][8] = {
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

byte enemy_sprite[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111,
    B11111
};

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

Player player;
int enemy_pos, jump, loose = 0, points = 0;

void draw_sprite(int sprite_num, int col, int row)
{
    lcd.setCursor(col, row);
    lcd.write(sprite_num);
}

void draw_player()
{
    draw_sprite(player.sprite, player.col, player.row);
}

void draw_enemy()
{
    draw_sprite(ENEMY_SPRITE, enemy_pos, 1);
}

void draw_points()
{
    int col;
    if (points < 10) {
        col = 15;
    } else if (points < 100) {
        col = 14;
    } else {
        col = 13;
    }

    lcd.setCursor(col, 0);
    lcd.print(points);
}

void update_player()
{
    switch (player.jump_state) {

        case RISING: {
            if (player.sprite == PLAYER_SPRITE_DOWN) {
                player.sprite = PLAYER_SPRITE_UP;
            } else { /* sprite up */
                if (player.row == ROW_TOP) {
                    player.jump_state = FALLING;
                } else {
                    player.row = ROW_TOP;
                }

                player.sprite = PLAYER_SPRITE_DOWN;
            }
        } break;

        case FALLING: {
            if (player.sprite == PLAYER_SPRITE_DOWN) {
                if (player.row == ROW_DOWN) {
                    player.jump_state = NOT_JUMPING;
                } else { /* row top */
                    player.row = ROW_DOWN;
                    player.sprite = PLAYER_SPRITE_UP;
                }
            } else {
                player.sprite = PLAYER_SPRITE_DOWN;
            }
        } break;

        case NOT_JUMPING: {
            /* do nothing for now */
        } break;
    }
}

int enemy_is_updatable = 1;
void update_enemy()
{
    if (enemy_is_updatable) {
        enemy_pos -= 1;
        if (enemy_pos < 0) {
            enemy_pos = 15;
        }
    }

    enemy_is_updatable = !enemy_is_updatable;
}

int ms_count = 0;
void update_points()
{
    if (enemy_pos == player.col &&
        player.row == ROW_DOWN  &&
        player.sprite == PLAYER_SPRITE_DOWN)
    {
        loose = 1;
        ms_count = 0;
        return;
    }

    ms_count += DELAY_MS;
    if (ms_count == 1000) {
        points++;
        ms_count = 0;
    }
}

void init_game()
{
    player = {
        .jump_state = NOT_JUMPING,
        .col = 1,
        .row = 1,
        .sprite = 1
    };

    enemy_pos = 15;
    loose = 0;
    points = 0;
    enemy_is_updatable = 1;
}

void setup()
{
    lcd.begin(16, 2);

    lcd.createChar(PLAYER_SPRITE_DOWN, player_sprites[0]);
    lcd.createChar(PLAYER_SPRITE_UP, player_sprites[1]);
    lcd.createChar(ENEMY_SPRITE, enemy_sprite);

    pinMode(JUMP_BUTTON, INPUT);

    init_game();
    // Serial.begin(9600); // debug
}

void loop()
{
    lcd.clear();

    if (loose) {
        lcd.setCursor(0, 0);
        lcd.print("Game Over!");

        lcd.setCursor(0, 1);
        lcd.print("Press <jump>");

        draw_points();

        while (!digitalRead(JUMP_BUTTON)) {
            delay(100);
        }

        init_game();
        return;
    }

    if (player.jump_state == NOT_JUMPING) {
        jump = digitalRead(JUMP_BUTTON);
        if (jump) {
            player.jump_state = RISING;
        }
    }

    draw_enemy();
    draw_player();
    draw_points();

    update_points();
    update_enemy();
    update_player();

    delay(DELAY_MS);
}