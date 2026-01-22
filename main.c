#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raylib.h>

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

typedef struct
{
    float speed;
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
void move_ball(Ball *ball)
{
    ball->pos.x = (ball->pos.x + ball->direction.x * ball->speed);
    ball->pos.y = (ball->pos.y + ball->direction.y * ball->speed);
}

Ball *create_ball(Vector2 position, Vector2 direction)
{
    Ball *ball = (Ball *)malloc(sizeof(Ball));
    ball->pos = position;
    ball->direction = direction;
    ball->speed = 5;
    ball->radius = 10;

    return ball;
}

Player *create_player(Vector2 position, int id)
{
    Player *player = (Player *)malloc(sizeof(Player));
    player->speed = 10;
    player->id = id;
    player->collision = (Rectangle){position.x, position.y, 20, 100};

    return player;
}

Screen *create_screen(Vector2 size, char *name)
{
    Screen *screen = (Screen *)malloc(sizeof(Screen));
    screen->Width = (int)size.x;
    screen->Height = (int)size.y;

    InitWindow(screen->Width, screen->Height, name);
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

        if (ball->speed < 15)
            ball->speed += 1; // Adiciona uma aceleração na colisão
    }

    // Colisão com o Teto
    if (CheckCollisionCircleRec(ball->pos, ball->radius, top_wall))
    {
        ball->direction.y *= -1;
        // Reposiciona a bola exatamente abaixo da parede superior
        ball->pos.y = top_wall.y + top_wall.height + ball->radius + 1;
        if (ball->speed < 15)
            ball->speed += 1; // Adiciona uma aceleração na colisão
    }

    // Colisão com o Chão
    if (CheckCollisionCircleRec(ball->pos, ball->radius, bottom_wall))
    {
        ball->direction.y *= -1;
        // Reposiciona a bola exatamente acima da parede inferior
        ball->pos.y = bottom_wall.y - ball->radius - 1;
        if (ball->speed < 15)
            ball->speed += 1; // Adiciona uma aceleração na colisão
    }

    return 0;
}

void ia_bot(Ball *ball, Screen *screen, Player *player)
{
    if (player->collision.y + player->collision.height / 2 < ball->pos.y)
        player->collision.y += player->speed;

    if (player->collision.y + player->collision.height / 2 > ball->pos.y)
        player->collision.y -= player->speed;

    if (player->collision.y < 0)
        player->collision.y = 1;

    if (player->collision.y + player->collision.height >= screen->Height)
        player->collision.y = screen->Height - player->collision.height + 2;
}

void move_player(Player *player, Screen *screen)
{
    if (player->id == 1)
    {

        if (IsKeyDown(KEY_W) && player->collision.y > 0)
        {
            player->collision.y -= player->speed;
        }
        else if (IsKeyDown(KEY_S) && player->collision.y + player->collision.height < screen->Height)
        {

            player->collision.y += player->speed;
        }
    }

    else if (player->id == 2)
    {
        if (IsKeyDown(KEY_UP) && player->collision.y > 0)
        {
            player->collision.y -= player->speed;
        }
        else if (IsKeyDown(KEY_DOWN) && player->collision.y + player->collision.height < screen->Height)
        {
            player->collision.y += player->speed;
        }
    }
}

int update_score(Screen *screen, Score *score, Ball *ball)
{
    if (ball->pos.x > screen->Width)
    {
        score->player1_score++;
        free(ball);
        return 1;
    }

    if (ball->pos.x < 0)
    {
        score->player2_score++;
        free(ball);
        return 1;
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
    player1->collision.y = screen->Height / 2 - player1->collision.height;
    player2->collision.y = screen->Height / 2 - player2->collision.height;
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

        return 1;
    }

    return 0;
}

GameMode *create_game_mode()
{
    GameMode *game_mode = (GameMode *)malloc(sizeof(GameMode));
    return game_mode;
}

int change_game_mode(Vector2 mousepoint, GameMode *game_mode, Rectangle btn, GameMode mode)
{
    if (CheckCollisionPointRec(mousepoint, btn))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            *game_mode = mode;
            return 1;
        }
    }
    return 0;
}

int draw_menu(Screen *screen, Vector2 mousepoint, GameMode *game_mode)
{

    DrawText(TextFormat("x: %f   y: %f", mousepoint.x, mousepoint.y), 10, 10, 20, WHITE);

    const char *title = "PONG";
    int font_size_title = 45;
    int title_size = MeasureText(title, font_size_title);
    DrawText(title, screen->Width / 2 - title_size / 2, 20, font_size_title, WHITE);

    Rectangle btn_player = (Rectangle){screen->Width / 2 - 200 + 5, screen->Height / 2 - 110 + 5, 390, 90};
    DrawRectangle(screen->Width / 2 - 200, screen->Height / 2 - 110, 400, 100, WHITE);
    DrawRectangleRec(btn_player, BLACK);

    const char *player = "Player vs Player";
    int font_size_player = 35;
    int player_size = MeasureText(player, font_size_player);
    DrawText(player, screen->Width / 2 - player_size / 2, screen->Height / 2 - 55 - font_size_player / 2, font_size_player, WHITE);

    Rectangle btn_bot = (Rectangle){screen->Width / 2 - 200 + 5, screen->Height / 2 + 10 + 5, 390, 90, BLACK};
    DrawRectangle(screen->Width / 2 - 200, screen->Height / 2 + 10, 400, 100, WHITE);
    DrawRectangleRec(btn_bot, BLACK);

    const char *bot = "Player vs Bot";
    int font_size_bot = 35;
    int bot_size = MeasureText(bot, font_size_bot);
    DrawText(bot, screen->Width / 2 - bot_size / 2, screen->Height / 2 + 30 + font_size_bot / 2, font_size_bot, WHITE);

    return change_game_mode(mousepoint, game_mode, btn_player, PLAYER_PLAYER) || change_game_mode(mousepoint, game_mode, btn_bot, PLAYER_BOT);
}

int main()
{
    GameState status = STATE_MENU;
    Screen *screen = create_screen((Vector2){800, 600}, "Pong Game");
    Player *player1 = create_player((Vector2){0, screen->Height / 2 - 100}, 1);
    Player *player2 = create_player((Vector2){screen->Width - 20, screen->Height / 2 - 100}, 2);
    Ball *ball = create_ball((Vector2){400, 400}, (Vector2){1, .25});
    Score *score = create_score();
    GameMode *game_mode = create_game_mode();
    bool start = false;

    while (!WindowShouldClose())
    {
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

            if (start)
            {

                move_ball(ball);
                checkcollision(ball, player1, screen);
                checkcollision(ball, player2, screen);
                move_player(player1, screen);

                // Alterna o controle entre player 2 e bot
                if (*game_mode == PLAYER_PLAYER)
                    move_player(player2, screen);
                else if (*game_mode == PLAYER_BOT)
                    ia_bot(ball, screen, player2);
            }
            else{
                const char *text = "Aperte Space para começar!";
                DrawText(text, screen->Width / 2 - MeasureText(text, 25) / 2, screen->Height - 35, 25, WHITE);
            }

            if(IsKeyDown(KEY_SPACE))
            {
                start = true;

            }

            if (update_score(screen, score, ball))
                ball = create_ball((Vector2){400, 400}, (Vector2){1, .25});

            if (victory(player1, player2, screen, score, ball))
                status = STATE_GAMEOVER;

            if (IsKeyPressed(KEY_ESCAPE))
            {
                status = STATE_MENU;
            }
        }

        if (status == STATE_GAMEOVER)
        {
            const char *text1 = TextFormat("Vitoria do jogador %d!", score->player1_score >= 3 ? 1 : 2);
            char *text2 = "Press R to Restart";
            DrawText(text1, screen->Width / 2 - 100, screen->Height / 2 - 50, 40, RED);
            DrawText(text2, screen->Width / 2 - 130, screen->Height / 2, 20, WHITE);
            if (IsKeyPressed(KEY_R))
            {
                score->player1_score = 0;
                score->player2_score = 0;
                status = STATE_PLAYING;
            }
        }

        if (status == STATE_MENU)
        {
            if (draw_menu(screen, mousepoint, game_mode))
                status = STATE_PLAYING;
        }

        EndDrawing();
    }

    CloseWindow();
    free(player1);
    free(player2);
    free(ball);
    free(screen);
    free(score);
    free(game_mode);

    return 0;
}