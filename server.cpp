#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include "common.hpp"
int leftScore = 0;
int rightScore = 0;

bool gameOver = false;
int winner = 0;

const int WIN_SCORE = 5;

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong - SERVER");
    SetTargetFPS(60);

    Ball ball;
    ball.radius = 20;
    ball.x = SCREEN_WIDTH / 2.0f;
    ball.y = SCREEN_HEIGHT / 2.0f;
    ball.sx = 5;
    ball.sy = 5;

    Paddle leftPaddle = {10, SCREEN_HEIGHT / 2.0f - 60, 25, 120, 6};
    Paddle rightPaddle = {SCREEN_WIDTH - 35.0f, SCREEN_HEIGHT / 2.0f - 60, 25, 120, 6};

    // 1. Setup Network
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket failed");
        return 1;
    }

    // Fix "Address already in use"
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Disable Nagle's Algorithm
    setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    listen(server_fd, 1);
    fcntl(server_fd, F_SETFL, O_NONBLOCK); // Non-blocking listen

    std::cout << "Server started. Waiting for connections...\n";

    int client_sock = -1;
    bool connected = false;

    while (!WindowShouldClose())
    {
        // 2. Accept Connections
        if (!connected)
        {
            sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            client_sock = accept(server_fd, (sockaddr *)&client_addr, &addr_len);

            if (client_sock >= 0)
            {
                connected = true;
                fcntl(client_sock, F_SETFL, O_NONBLOCK);
                int flag = 1;
                setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
                std::cout << "Client connected!\n";
            }
        }

        // 3. Game Logic
        if (connected)
        {
            // Read Client's Paddle Y
            float clientY;
            int bytes = recv(client_sock, &clientY, sizeof(float), MSG_DONTWAIT);

            if (bytes > 0)
            {
                rightPaddle.y = clientY;
            }
            else if (bytes == 0)
            {
                // Client closed connection
                connected = false;
                close(client_sock);
                client_sock = -1;
                std::cout << "Client disconnected. Resetting...\n";
                // Reset Ball
                ball.x = SCREEN_WIDTH / 2;
                ball.y = SCREEN_HEIGHT / 2;
            }
            if (!gameOver)
            {

                // Server controls Left Paddle
                if (IsKeyDown(KEY_W))
                    leftPaddle.Move(true, false);
                if (IsKeyDown(KEY_S))
                    leftPaddle.Move(false, true);

                ball.Update();

                if (ball.x - ball.radius <= 0)
                {

                    rightScore++;

                    ball.x = SCREEN_WIDTH / 2.0f;
                    ball.y = SCREEN_HEIGHT / 2.0f;

                    ball.sx = 7; // send toward left player
                    ball.sy = (GetRandomValue(0, 1) ? 7 : -7);
                }

                if (ball.x + ball.radius >= SCREEN_WIDTH)
                {

                    leftScore++;

                    ball.x = SCREEN_WIDTH / 2.0f;
                    ball.y = SCREEN_HEIGHT / 2.0f;

                    ball.sx = -7; // send toward right player
                    ball.sy = (GetRandomValue(0, 1) ? 7 : -7);
                }

                if (CheckCollisionCircleRec({ball.x, ball.y}, ball.radius, {leftPaddle.x, leftPaddle.y, leftPaddle.width, leftPaddle.height}))
                {
                    ball.sx *= -1;
                    ball.x = leftPaddle.x + leftPaddle.width + ball.radius + 1;
                }
                if (CheckCollisionCircleRec({ball.x, ball.y}, ball.radius, {rightPaddle.x, rightPaddle.y, rightPaddle.width, rightPaddle.height}))
                {
                    ball.sx *= -1;
                    ball.x = rightPaddle.x - ball.radius - 1;
                }

                if (ball.x - ball.radius <= 0)
                {
                    rightScore++;

                    ball.x = SCREEN_WIDTH / 2.0f;
                    ball.y = SCREEN_HEIGHT / 2.0f;
                    ball.sx = 7;
                    ball.sy = (GetRandomValue(0, 1) ? 7 : -7);
                }

                if (ball.x + ball.radius >= SCREEN_WIDTH)
                {
                    leftScore++;

                    ball.x = SCREEN_WIDTH / 2.0f;
                    ball.y = SCREEN_HEIGHT / 2.0f;
                    ball.sx = -7;
                    ball.sy = (GetRandomValue(0, 1) ? 7 : -7);
                }

                // ---- Win Condition ----
                if (leftScore >= WIN_SCORE)
                {
                    gameOver = true;
                    winner = 1;
                }

                if (rightScore >= WIN_SCORE)
                {
                    gameOver = true;
                    winner = 2;
                }
            }

            // ---- Restart Logic (SERVER ONLY) ----
            if (gameOver && IsKeyPressed(KEY_R))
            {

                leftScore = 0;
                rightScore = 0;

                gameOver = false;
                winner = 0;

                ball.x = SCREEN_WIDTH / 2.0f;
                ball.y = SCREEN_HEIGHT / 2.0f;
                ball.sx = 7;
                ball.sy = 7;

                leftPaddle.y = SCREEN_HEIGHT / 2.0f - 60;
                rightPaddle.y = SCREEN_HEIGHT / 2.0f - 60;
            }

            GameState state;
            state.ballX = ball.x;
            state.ballY = ball.y;
            state.leftPaddleY = leftPaddle.y;
            state.rightPaddleY = rightPaddle.y;
            state.leftScore = leftScore;
            state.rightScore = rightScore;
            state.gameOver = gameOver;
            state.winner = winner;

            send(client_sock, &state, sizeof(GameState), MSG_NOSIGNAL);
        }

        BeginDrawing();
        ClearBackground(BLACK);

      

        if (!connected)
        {
            DrawText("Waiting for Player...", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 30, GRAY);
        }
        else
        {
            DrawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, WHITE);
            ball.Draw();
            leftPaddle.Draw();
            rightPaddle.Draw();

            DrawText(TextFormat("%d", leftScore),
                     SCREEN_WIDTH / 2 - 100, 40, 60, WHITE);

            DrawText(TextFormat("%d", rightScore),
                     SCREEN_WIDTH / 2 + 60, 40, 60, WHITE);

            if (gameOver)
            {
                if (winner == 1)
                    DrawText("LEFT PLAYER WINS!",
                             SCREEN_WIDTH / 2 - 250,
                             SCREEN_HEIGHT / 2,
                             50,
                             GREEN);

                if (winner == 2)
                    DrawText("RIGHT PLAYER WINS!",
                             SCREEN_WIDTH / 2 - 250,
                             SCREEN_HEIGHT / 2,
                             50,
                             GREEN);

                DrawText("Press R to Restart",
                         SCREEN_WIDTH / 2 - 200,
                         SCREEN_HEIGHT / 2 + 80,
                         30,
                         GRAY);
            }

            EndDrawing();
        }

       
    }
     if (client_sock != -1)
            close(client_sock);
        close(server_fd);
        CloseWindow();
        return 0;
}
