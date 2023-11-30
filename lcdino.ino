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

enum Jump_State { FALL, RISE, STOP };

enum Entity_Type {
    PLAYER,
    BASIC_ENEMY,
    JUMPING_ENEMY,
    FLYING_ENEMY,
};

enum Sprite {
    PLAYER_SPRITE_DOWN = 1,
    PLAYER_SPRITE_UP,
    BASIC_ENEMY_SPRITE_DOWN,
    BASIC_ENEMY_SPRITE_UP,
    FLYING_ENEMY_SPRITE_DOWN,
    FLYING_ENEMY_SPRITE_UP,
    LOGO_UNISC_SPRITE

};

struct Entity {
    enum Jump_State jump_state;
    enum Entity_Type type;
    enum Sprite sprite;
    int col, row;
};

#define LEN_PLAYER 2
byte player_sprites[LEN_PLAYER][8] = {
    {
        B00000,
        B00000,
        B00000,
        B00000,
        B00110,
        B10101,
        B01111,
        B00101
    },
    {
        B00110,
        B10101,
        B01111,
        B00101,
        B00000,
        B00000,
        B00000,
        B00000
    },
};

byte basic_enemy_sprites[2][8] = {
    {
        B00000,
        B00000,
        B00000,
        B00000,
        B00100,
        B01110,
        B01110,
        B11111
    },
    {
        B00000,
        B00000,
        B01110,
        B11111,
        B01110,
        B00000,
        B00000,
        B00000
    }
};

byte flying_enemy_sprites[2][8] = {
    {
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B01100,
        B00000,
        B00000
    },
    {
        B00000,
        B00000,
        B01100,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000
    }
};

byte logo_unisc_sprite[8] = {
    B11111,
    B11111,
    B11111,
    B10011,
    B10001,
    B10001,
    B11111,
    B11111
};

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

Entity player;
Entity enemy;
int jump, loose = 0, points = 0, running = 1;

void lcderror(char *s)
{
    lcd.clear();
    lcd.print("err:");
    lcd.setCursor(0, ROW_DOWN);
    lcd.print(s);
    running = 0;
}

void draw_sprite(int sprite_num, int col, int row)
{
    lcd.setCursor(col, row);
    lcd.write(sprite_num);
}

void draw_entity(Entity *entity)
{
    draw_sprite(entity->sprite, entity->col, entity->row);
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

void draw_unisc()
{
    lcd.setCursor(15, 1);
    lcd.write(LOGO_UNISC_SPRITE);
}

int jump_entity(Entity *entity, Jump_State on_stop, Sprite up, Sprite down)
{
    switch (entity->jump_state) {
        case RISE: {
            if (entity->sprite == down) {
                entity->sprite = up;
            } else { /* sprite up */
                if (entity->row == ROW_TOP) {
                    entity->jump_state = FALL;
                } else {
                    entity->row = ROW_TOP;
                }
                entity->sprite = down;
            }
        } break;

        case FALL: {
            if (entity->sprite == down) {
                if (entity->row == ROW_DOWN) {
                    entity->jump_state = STOP;
                } else { /* row top */
                    entity->row = ROW_DOWN;
                    entity->sprite = up;
                }
            } else {
                entity->sprite = down;
            }
        } break;

        case STOP: {
            entity->jump_state = on_stop;
        } break;

        default: {
            lcderror("invalid jmp");
            return 1;
        } break;
    }

    return 0;
}

int update_player()
{
    return jump_entity(&player, STOP, PLAYER_SPRITE_UP, PLAYER_SPRITE_DOWN);
}

void randomize_enemy(Entity *enemy)
{
    int chaos = random(6);

    switch (chaos) {
        case 0: {
            enemy->jump_state = RISE;
            enemy->type = JUMPING_ENEMY;
            enemy->sprite = BASIC_ENEMY_SPRITE_DOWN;
            enemy->row = ROW_DOWN;
        } break;

        case 1: {
            enemy->jump_state = FALL;
            enemy->type = JUMPING_ENEMY;
            enemy->sprite = BASIC_ENEMY_SPRITE_UP;
            enemy->row = ROW_TOP;
        } break;

        case 2: {
            enemy->jump_state = RISE;
            enemy->type = FLYING_ENEMY;
            enemy->sprite = FLYING_ENEMY_SPRITE_DOWN;
            enemy->row = ROW_DOWN;
        } break;

        case 3: {
            enemy->jump_state = FALL;
            enemy->type = FLYING_ENEMY;
            enemy->sprite = FLYING_ENEMY_SPRITE_UP;
            enemy->row = ROW_TOP;
        } break;

        default: {
            enemy->jump_state = STOP;
            enemy->type = BASIC_ENEMY;
            enemy->sprite = BASIC_ENEMY_SPRITE_DOWN;
            enemy->row = ROW_DOWN;
        } break;
    }
}

// its more smoothly if we update enemys 1/2 times in comparison to player
int enemy_is_updatable = 1;
int update_enemy(Entity *enemy)
{
    if (!enemy_is_updatable) {
        enemy_is_updatable = !enemy_is_updatable;
        return 0;
    }

    enemy->col -= 1;
    if (enemy->col < 0) {
        enemy->col = 15;
        randomize_enemy(enemy);
    }

    int result;
    if (enemy->type == JUMPING_ENEMY) {
       result = jump_entity(enemy, RISE, BASIC_ENEMY_SPRITE_UP, BASIC_ENEMY_SPRITE_DOWN);
    } else if (enemy->type == FLYING_ENEMY) {
       result = jump_entity(enemy, RISE, FLYING_ENEMY_SPRITE_UP, FLYING_ENEMY_SPRITE_DOWN);
    }

    enemy_is_updatable = !enemy_is_updatable;
    return result;
}

int ms_count = 0;
void update_points()
{
    if (enemy.col == player.col &&
        enemy.row == player.row)
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
        .jump_state = STOP,
        .type = PLAYER,
        .sprite = PLAYER_SPRITE_DOWN,
        .col = 1,
        .row = 1,
    };

    enemy = {
        .jump_state = STOP,
        .type = BASIC_ENEMY,
        .sprite = BASIC_ENEMY_SPRITE_DOWN,
        .col = 15,
        .row = 1
    };

    loose = 0;
    points = 0;
    enemy_is_updatable = 1;
}

void setup()
{
    lcd.begin(16, 2);

    lcd.createChar(PLAYER_SPRITE_DOWN, player_sprites[0]);            //  8
    lcd.createChar(PLAYER_SPRITE_UP, player_sprites[1]);              // 16
    lcd.createChar(BASIC_ENEMY_SPRITE_DOWN, basic_enemy_sprites[0]);  // 24
    lcd.createChar(BASIC_ENEMY_SPRITE_UP, basic_enemy_sprites[1]);    // 32
    lcd.createChar(FLYING_ENEMY_SPRITE_DOWN, flying_enemy_sprites[0]);// 48
    lcd.createChar(FLYING_ENEMY_SPRITE_UP, flying_enemy_sprites[1]);  // 56
    lcd.createChar(LOGO_UNISC_SPRITE, logo_unisc_sprite);             // 64 - thats the limit!


    pinMode(JUMP_BUTTON, INPUT);

    randomSeed(analogRead(0));

    init_game();
    Serial.begin(9600); // debug
}

void loop()
{
    if (!running) {
        return;
    }

    lcd.clear();
    if (loose) {
        lcd.setCursor(0, 0);
        lcd.print("Game Over!");

        lcd.setCursor(0, 1);
        lcd.print("Press <jump>");

        draw_points();
        draw_unisc();

        while (!digitalRead(JUMP_BUTTON)) {
            delay(100);
        }

        init_game();
        return;
    }

    if (player.jump_state == STOP) {
        jump = digitalRead(JUMP_BUTTON);
        if (jump) {
            player.jump_state = RISE;
        }
    }

    draw_entity(&enemy);
    draw_entity(&player);
    draw_points();

    update_points();

    if (update_enemy(&enemy) != 0) {
        return;
    }

    if (update_player() != 0) {
        return;
    }

    delay(DELAY_MS);
}