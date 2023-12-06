#include <LiquidCrystal.h>

#define TRUE  7
#define FALSE 0

#define RS  9
#define EN  8
#define D4  7
#define D5  6
#define D6  5
#define D7  4
#define JP 10

#define DELAY_MS    75
#define ENEMY_COUNT  2

#define ROW_TOP  0
#define ROW_DOWN 1

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
    int col, row, updatable;
};

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void clear()
{
    lcd.clear();
}

void lcd_set_cursor(int col, int row)
{
    lcd.setCursor(col, row);
}

void lcd_load_char(int sprite_num, byte data[8])
{
    lcd.createChar(sprite_num, data);
}

void lcd_begin()
{
    lcd.begin(16, 2);
}

void lcd_write(int sprite_num, int col, int row)
{
    lcd_set_cursor(col, row);
    lcd.write(sprite_num);
}

void lcd_print(char *s, int col, int row)
{
    lcd_set_cursor(col, row);
    lcd.print(s);
}

byte player_sprites[2][8] = {
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

Entity player;
Entity enemys[ENEMY_COUNT];

int jump, loose, points, running, buzzing;

void init_game()
{
    player = {
        .jump_state = STOP,
        .type = PLAYER,
        .sprite = PLAYER_SPRITE_DOWN,
        .col = 1,
        .row = ROW_DOWN,
        .updatable = TRUE
    };

    for (int i = 0; i < ENEMY_COUNT; i++) {
        enemys[i] = {
            .jump_state = STOP,
            .type = BASIC_ENEMY,
            .sprite = BASIC_ENEMY_SPRITE_DOWN,
            .col = 15 + (i*8),
            .row = ROW_DOWN,
            .updatable = FALSE
        };
    }

    loose = 0;
    points = 0;
    running = 1;
    buzzing = 0;
}

void lcderror(char *s)
{
    clear();
    lcd_print("err:", 0, ROW_DOWN);
    lcd_print(s, 0, ROW_DOWN);
    running = 0;
}

void draw_sprite(int sprite_num, int col, int row)
{
    if (col <= 15) {
        lcd_write(sprite_num, col, row);
    }
}

void draw_entity(Entity *entity)
{
    draw_sprite(entity->sprite, entity->col, entity->row);
}

void draw_enemys()
{
    for (int i = 0; i < ENEMY_COUNT; i++) {
        draw_entity(&enemys[i]);
    }
}

char str_points[4];
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

    itoa(points, str_points, 10);
    lcd_print(str_points, col, ROW_TOP);
}

void draw_unisc()
{
    lcd_write(LOGO_UNISC_SPRITE, 15, ROW_DOWN);
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
            enemy->row = ROW_DOWN;
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
            enemy->row = ROW_DOWN;
        } break;

        default: {
            enemy->jump_state = STOP;
            enemy->type = BASIC_ENEMY;
            enemy->sprite = BASIC_ENEMY_SPRITE_DOWN;
            enemy->row = ROW_DOWN;
        } break;
    }
}

int update_enemy(Entity *enemy)
{
    enemy->updatable = !enemy->updatable;

    // its more smoothly if we update enemys 1/2 times in comparison to player
    if (!enemy->updatable) {
        return 0;
    }

    int jmp_result;
    switch (enemy->type) {
        case JUMPING_ENEMY: {
           jmp_result = jump_entity(enemy, RISE, BASIC_ENEMY_SPRITE_UP, BASIC_ENEMY_SPRITE_DOWN);
        } break;

        case FLYING_ENEMY: {
           jmp_result = jump_entity(enemy, RISE, FLYING_ENEMY_SPRITE_UP, FLYING_ENEMY_SPRITE_DOWN);
        } break;

        default: {
            jmp_result = 0;
        } break;
    }

    enemy->col -= 1;
    if (enemy->col < 0) {
        enemy->col = 15;
        randomize_enemy(enemy);
    }

    return jmp_result;
}

int update_enemys()
{
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (update_enemy(&enemys[i]) != 0) {
            return 1;
        }
    }

    return 0;
}

int ms_count = 0;
void update_points()
{
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (enemys[i].col == player.col && enemys[i].row == player.row) {
            loose = 1;
            ms_count = 0;
            return;
        }
    }

    ms_count += DELAY_MS;
    if (ms_count >= 1000) {
        points++;
        ms_count = 0;
    }
}

const int buzzer = 2;

void setup()
{
    lcd_begin();

    lcd_load_char(PLAYER_SPRITE_DOWN, player_sprites[0]);            //  8
    lcd_load_char(PLAYER_SPRITE_UP, player_sprites[1]);              // 16
    lcd_load_char(BASIC_ENEMY_SPRITE_DOWN, basic_enemy_sprites[0]);  // 24
    lcd_load_char(BASIC_ENEMY_SPRITE_UP, basic_enemy_sprites[1]);    // 32
    lcd_load_char(FLYING_ENEMY_SPRITE_DOWN, flying_enemy_sprites[0]);// 48
    lcd_load_char(FLYING_ENEMY_SPRITE_UP, flying_enemy_sprites[1]);  // 56
    lcd_load_char(LOGO_UNISC_SPRITE, logo_unisc_sprite);             // 64 - thats the limit!

    pinMode(JP, INPUT);
    pinMode(buzzer, OUTPUT);
    randomSeed(analogRead(0));

    init_game();
}

void loop()
{
    if (!running) {
        return;
    }
    
    digitalWrite(buzzer, LOW);
    clear();
    if (loose) {
        lcd_print("Game Over!", 0, ROW_TOP);
        lcd_print("Press <jump>", 0, ROW_DOWN);

        draw_points();
        draw_unisc();

        if (!buzzing){
          buzzing = 1;
          tone(buzzer, 1500, 200);
          delay(200);
          tone(buzzer, 1300, 200);
          delay(200);
          tone(buzzer, 1200, 200);
          delay(200);
          tone(buzzer, 1000, 300);
        }

        while (!digitalRead(JP)) {
            delay(100);
        }

        init_game();
        return;
    }

    if (player.jump_state == STOP) {
        jump = digitalRead(JP);
        if (jump) {
            tone(buzzer, 1500, 50);
            player.jump_state = RISE;
        }
    }

    draw_enemys();
    draw_entity(&player);
    draw_points();

    update_points();
    if (update_enemys() != 0) {
        return;
    }
    if (update_player() != 0) {
        return;
    }

    delay(DELAY_MS);
}
