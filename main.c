#define SUPPORT_EXTERN_CONFIG

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raylib.h>
#include "assets/pong.h"

typedef enum
{
    STATE_MENU = 0,
    STATE_PLAYING = 1,
    STATE_GAMEOVER = 2
} GameState;

typedef enum
{
    PLAYER_PLAYER = 0,
    PLAYER_BOT = 1
} GameMode;

GameMode selected_menu = PLAYER_PLAYER;

typedef struct
{
    float speed;
    float max_speed;
    int radius;
    Vector2 direction;
    Vector2 pos;
} Ball;

typedef struct
{
    int id;
    int speed;
    Rectangle collision;
} Player;

typedef struct
{
    int Width;
    int Height;
} Screen;

typedef struct
{
    int player1_score;
    int player2_score;
} Score;

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// pos + dir * spd
void move_ball(Ball *ball, float delta_time)
{
    ball->pos.x = ball->pos.x + ball->direction.x * ball->speed * delta_time;
    ball->pos.y = ball->pos.y + ball->direction.y * ball->speed * delta_time;
}

Ball *create_ball(Vector2 position, Vector2 direction)
{
    Ball *ball = (Ball *)malloc(sizeof(Ball));
    ball->pos = position;
    ball->direction = direction;
    ball->speed = 200;
    ball->max_speed = 600;
    ball->radius = 10;

    return ball;
}

Player *create_player(Vector2 position, int id)
{
    Player *player = (Player *)malloc(sizeof(Player));
    player->speed = 500;
    player->id = id;
    player->collision = (Rectangle){position.x, position.y, 20, 100};

    return player;
}

Screen *create_screen(Vector2 size, char *name)
{
    Screen *screen = (Screen *)malloc(sizeof(Screen));
    screen->Width = (int)size.x;
    screen->Height = (int)size.y;

    Image icon_game = LoadImageFromMemory(".png", icon.data, icon.size);
    InitWindow(screen->Width, screen->Height, name);
    SetWindowIcon(icon_game);
    // UnloadImage(icon_game);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    return screen;
}

int checkcollision(Ball *ball, Player *player, Screen *screen)
{
    Rectangle top_wall = (Rectangle){0, -10, screen->Width, 20};
    Rectangle bottom_wall = (Rectangle){0, screen->Height - 10, screen->Width, 20};

    if (CheckCollisionCircleRec(ball->pos, ball->radius, player->collision))
    {
        float dy = ball->pos.y - (player->collision.y + player->collision.height / 2);
        float normalizedDy = map(dy, -player->collision.height / 2, player->collision.height / 2, -1, 1);

        ball->direction.x *= -1;
        ball->direction.y = normalizedDy;

        if (player->id == 1)
            ball->pos.x = player->collision.x + player->collision.width + ball->radius;
        else
            ball->pos.x = player->collision.x - ball->radius;

        if (ball->speed < ball->max_speed)
            ball->speed += 30; // Adiciona uma aceleração na colisão

        return 1;
    }

    // Colisão com o Teto
    if (CheckCollisionCircleRec(ball->pos, ball->radius, top_wall))
    {
        ball->direction.y *= -1;
        // Reposiciona a bola exatamente abaixo da parede superior
        ball->pos.y = top_wall.y + top_wall.height + ball->radius + 1;
        if (ball->speed < ball->max_speed)
            ball->speed += 30; // Adiciona uma aceleração na colisão

        return 1;
    }

    // Colisão com o Chão
    if (CheckCollisionCircleRec(ball->pos, ball->radius, bottom_wall))
    {
        ball->direction.y *= -1;
        // Reposiciona a bola exatamente acima da parede inferior
        ball->pos.y = bottom_wall.y - ball->radius - 1;
        if (ball->speed < 15)
            ball->speed += 0.3; // Adiciona uma aceleração na colisão

        return 1;
    }

    return 0;
}

void reset_ball(Ball *ball, Screen *screen, float directionX)
{
    ball->pos = (Vector2){screen->Width / 2, screen->Height / 2};
    ball->direction = (Vector2){directionX, 0.25f};
    ball->speed = 200;
}

void ia_bot(Ball *ball, Screen *screen, Player *bot, float dt)
{
    float distance = ball->pos.y - (bot->collision.y + bot->collision.height / 2);
    float bot_speed = 500.0f;

    if (fabsf(distance) > 5.0f)
    {
        bot->collision.y += distance * 10.0f * dt;
    }

    if (bot->collision.y < 0)
        bot->collision.y = 0;

    if (bot->collision.y + bot->collision.height >= screen->Height)
        bot->collision.y = screen->Height - bot->collision.height;
}

void move_player(Player *player, Screen *screen, float dt)
{
    if (player->id == 1)
    {

        if (player->collision.y > 0 && (IsKeyDown(KEY_W) || (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP))))
        {
            player->collision.y -= player->speed * dt;
        }
        else if (player->collision.y + player->collision.height < screen->Height && (IsKeyDown(KEY_S) || (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))))
        {

            player->collision.y += player->speed * dt;
        }
    }

    else if (player->id == 2)
    {
        if (player->collision.y > 0 && (IsKeyDown(KEY_UP) || (IsGamepadAvailable(1) && IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_FACE_UP))))
        {
            player->collision.y -= player->speed * dt;
        }
        else if (player->collision.y + player->collision.height < screen->Height && (IsKeyDown(KEY_DOWN) || (IsGamepadAvailable(1) && IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_FACE_DOWN))))
        {
            player->collision.y += player->speed * dt;
        }
    }
}

int update_score(Screen *screen, Score *score, Ball *ball)
{
    if (ball->pos.x > screen->Width)
    {
        score->player1_score++;
        return 1;
    }

    if (ball->pos.x < 0)
    {
        score->player2_score++;
        return -1;
    }

    return 0;
}

Score *create_score()
{
    Score *score = (Score *)malloc(sizeof(Score));
    score->player1_score = 0;
    score->player2_score = 0;

    return score;
}

void draw_score(Score *score, Screen *screen)
{
    DrawText(TextFormat("%d", score->player1_score), screen->Width / 4, 20, 40, WHITE);
    DrawText(TextFormat("%d", score->player2_score), screen->Width - screen->Width / 4, 20, 40, WHITE);
}

void reset_game(Player *player1, Player *player2, Screen *screen, Score *score, Ball *ball)
{
    reset_ball(ball, screen, 1.0f);
    player1->collision.y = screen->Height / 2 - player1->collision.height;
    player2->collision.y = screen->Height / 2 - player2->collision.height;
    score->player1_score = 0;
    score->player2_score = 0;
}

int victory(Player *player1, Player *player2, Screen *screen, Score *score, Ball *ball)
{
    if (score->player1_score >= 10)
    {
        reset_game(player1, player2, screen, score, ball);
        return 1;
    }

    if (score->player2_score >= 10)
    {
        reset_game(player1, player2, screen, score, ball);

        return 2;
    }

    return 0;
}

GameMode *create_game_mode()
{
    GameMode *game_mode = (GameMode *)malloc(sizeof(GameMode));
    return game_mode;
}

Rectangle draw_button(const char *text, int font_size, Vector2 pos, Vector2 bnt_size)
{
    Rectangle btn = (Rectangle){pos.x, pos.y, bnt_size.x, bnt_size.y};
    DrawRectangleRec(btn, WHITE);
    DrawRectangle(pos.x + 5, pos.y + 5, bnt_size.x - 10, bnt_size.y - 10, BLACK);

    int text_size = MeasureText(text, font_size);
    DrawText(text, pos.x + bnt_size.x / 2 - text_size / 2, pos.y + 55 - font_size / 2, font_size, WHITE);

    return btn;
}

int change_game_mode(Vector2 mousepoint, GameMode *game_mode, Rectangle btn[2])
{
    int mouse_collision[2] = {0, 0};

    for (int i = 0; i < 2; i++)
    {
        mouse_collision[i] = CheckCollisionPointRec(mousepoint, btn[i]);
        if (mouse_collision[i])
        {
            btn[i].width += 10;
            btn[i].height += 10;
            selected_menu = i;
            draw_button(selected_menu == PLAYER_PLAYER ? "Player vs Player" : "Player vs Bot", 35, (Vector2){btn[i].x - 5, btn[i].y - 5}, (Vector2){btn[i].width, btn[i].height});

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                *game_mode = selected_menu;
                return 1;
            }
        }
    }

    if ((IsGamepadAvailable(0) || IsGamepadAvailable(1)) && !(mouse_collision[0] || mouse_collision[1]))
    {
        if ((IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) || (IsGamepadButtonPressed(1, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || IsGamepadButtonPressed(1, GAMEPAD_BUTTON_LEFT_FACE_UP)))
            selected_menu = !selected_menu;

        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsGamepadButtonPressed(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        {
            *game_mode = selected_menu;
            return 1;
        }

        btn[selected_menu].width += 10;
        btn[selected_menu].height += 10;
        draw_button(selected_menu == PLAYER_PLAYER ? "Player vs Player" : "Player vs Bot", 35, (Vector2){btn[selected_menu].x - 5, btn[selected_menu].y - 5}, (Vector2){btn[selected_menu].width, btn[selected_menu].height});
    }

    return 0;
}

int draw_menu(Screen *screen, Vector2 mousepoint, GameMode *game_mode)
{
    const char *title = "PONG";
    int font_size_title = 45;
    int title_size = MeasureText(title, font_size_title);
    DrawText(title, screen->Width / 2 - title_size / 2, 20, font_size_title, WHITE);

    Rectangle btn_player = draw_button("Player vs Player", 35, (Vector2){screen->Width / 2 - 200, screen->Height / 2 - 110}, (Vector2){400, 100});
    Rectangle btn_bot = draw_button("Player vs Bot", 35, (Vector2){screen->Width / 2 - 200, screen->Height / 2 + 10}, (Vector2){400, 100});
    Rectangle list_btn[2] = {btn_player, btn_bot};

    return change_game_mode(mousepoint, game_mode, list_btn);
}

int main()
{
    GameState status = STATE_MENU;
    Screen *screen = create_screen((Vector2){800, 600}, "Pong Game");
    Player *player1 = create_player((Vector2){10, screen->Height / 2 - 100}, 1);
    Player *player2 = create_player((Vector2){screen->Width - 30, screen->Height / 2 - 100}, 2);
    Ball *ball = create_ball((Vector2){screen->Width / 2, screen->Height / 2}, (Vector2){1, .25});
    Score *score = create_score();
    GameMode *game_mode = create_game_mode();
    bool start = false;
    int victory_status = 0;

    InitAudioDevice();
    Wave hit_wave = LoadWaveFromMemory(".wav", audio.data, audio.size);
    Sound hit_sound = LoadSoundFromWave(hit_wave);
    SetSoundVolume(hit_sound, 0.2f);
    UnloadWave(hit_wave);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Vector2 mousepoint = GetMousePosition();
        BeginDrawing();
        ClearBackground(BLACK);

        if (status == STATE_PLAYING)
        {
            draw_score(score, screen);
            // desenha players
            DrawRectangleRec(player1->collision, WHITE);
            DrawRectangleRec(player2->collision, WHITE);
            // desenha bola
            DrawCircle(ball->pos.x, ball->pos.y, ball->radius, WHITE);
            // desenha a rede
            for (size_t i = 0; i < 25; i++)
            {
                DrawRectangle(screen->Width / 2, i * 25 + 5, 10, 20, WHITE);
            }

            if (start)
            {

                move_ball(ball, dt);
                move_player(player1, screen, dt);
                if (checkcollision(ball, player1, screen))
                {
                    PlaySound(hit_sound);
                    if (IsGamepadAvailable(0))
                        SetGamepadVibration(0, 1.0f, 1.0f, 1.0f);
                }

                if (checkcollision(ball, player2, screen))
                {
                    PlaySound(hit_sound);
                    if (IsGamepadAvailable(1))
                        SetGamepadVibration(0, 1.0f, 1.0f, 1.0f);
                }

                // Alterna o controle entre player 2 e bot
                if (*game_mode == PLAYER_PLAYER)
                    move_player(player2, screen, dt);
                else if (*game_mode == PLAYER_BOT)
                    ia_bot(ball, screen, player2, dt);
            }
            else
            {
                const char *text = (IsGamepadAvailable(0) || IsGamepadAvailable(1) ? "Aperte Space para começar ou Y no Gamepad!" : "Aperte Space para começar!");
                DrawText(text, screen->Width / 2 - MeasureText(text, 25) / 2, screen->Height - 35, 25, WHITE);
            }

            if (IsKeyDown(KEY_SPACE) || (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) || (IsGamepadAvailable(1) && IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_UP)))
            {
                start = true;
            }

            int pts = update_score(screen, score, ball);
            if (pts)
            {
                reset_ball(ball, screen, pts * 1.0f);
            }

            victory_status = victory(player1, player2, screen, score, ball);
            if (victory_status)
                status = STATE_GAMEOVER;

            if (IsKeyPressed(KEY_ESCAPE) || (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) || (IsGamepadAvailable(1) && IsGamepadButtonDown(1, GAMEPAD_BUTTON_MIDDLE_RIGHT)))
            {
                reset_game(player1, player2, screen, score, ball);
                status = STATE_MENU;
            }
        }

        if (status == STATE_GAMEOVER)
        {
            const char *text1 = TextFormat("Vitória do jogador %d!", victory_status);
            const char *text2 = (IsGamepadAvailable(0) || IsGamepadAvailable(1) ? "Press R to Select or Start button" : "Press R to Restart");
            const char *text3 = (IsGamepadAvailable(0) || IsGamepadAvailable(1) ? "Press M to return to the menu or Start button" : "Press M to return to the menu");

            DrawText(text1, screen->Width / 2 - MeasureText(text1, 40) / 2, screen->Height / 2 - 50, 40, RED);
            DrawText(text2, screen->Width / 2 - MeasureText(text2, 20) / 2, screen->Height / 2, 20, WHITE);
            DrawText(text3, screen->Width / 2 - MeasureText(text3, 20) / 2, screen->Height / 2 + 20, 20, WHITE);

            if (IsKeyPressed(KEY_R) || (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_LEFT)) || (IsGamepadAvailable(1) && IsGamepadButtonDown(1, GAMEPAD_BUTTON_MIDDLE_LEFT)))
            {
                reset_game(player1, player2, screen, score, ball);
                status = STATE_PLAYING;
                start = false;
            }

            if (IsKeyPressed(KEY_M) || (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) || (IsGamepadAvailable(1) && IsGamepadButtonDown(1, GAMEPAD_BUTTON_MIDDLE_RIGHT)))
            {
                reset_game(player1, player2, screen, score, ball);
                status = STATE_MENU;
                start = false;
            }
        }

        if (status == STATE_MENU)
        {
            start = false;
            if (draw_menu(screen, mousepoint, game_mode))
                status = STATE_PLAYING;
        }

        EndDrawing();
    }

    UnloadSound(hit_sound);

    CloseWindow();
    free(player1);
    free(player2);
    free(ball);
    free(screen);
    free(score);
    free(game_mode);

    return 0;
}