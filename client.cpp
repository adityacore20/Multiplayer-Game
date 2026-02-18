#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>
#include "common.hpp"

int leftScore = 0;
int rightScore = 0;

bool gameOver = false;
int winner = 0;

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong - CLIENT");
    SetTargetFPS(60);

    Paddle leftPaddle = {10, SCREEN_HEIGHT / 2.0f - 60, 25, 120, 6};
    Paddle rightPaddle = {SCREEN_WIDTH - 35.0f, SCREEN_HEIGHT / 2.0f - 60, 25, 120, 6};
    Ball ball;
    ball.radius = 20;

    int sock = -1;
    bool connected = false;
    float connectTimer = 0.0f;

    while (!WindowShouldClose())
    {

        if (!connected)
        {
            connectTimer += GetFrameTime();
            // Try to connect every 1 second
            if (connectTimer > 1.0f)
            {
                connectTimer = 0;
                sock = socket(AF_INET, SOCK_STREAM, 0);

                sockaddr_in serv_addr{};
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(PORT);
                inet_pton(AF_INET, "162.16.0.139", &serv_addr.sin_addr); // Use Localhost for now

                if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
                {
                    connected = true;
                    // Set Non-Blocking AFTER connection
                    fcntl(sock, F_SETFL, O_NONBLOCK);
                    int flag = 1;
                    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
                    std::cout << "Connected to Server!\n";
                }
                else
                {
                    close(sock); // Failed, close and try again next second
                }
            }
        }

        // 2. Game Loop
        if (connected)
        {

            // Handle Input
            if (IsKeyDown(KEY_UP))
                rightPaddle.Move(true, false);
            if (IsKeyDown(KEY_DOWN))
                rightPaddle.Move(false, true);

            // Send Paddle Position
            int sent = send(sock, &rightPaddle.y, sizeof(float), MSG_NOSIGNAL | MSG_DONTWAIT);
            if (sent < 0 && errno == EPIPE)
            {
                connected = false;
                close(sock);
                std::cout << "Server Disconnected (Broken Pipe)\n";
            }

            // Receive Game State
            GameState state;
            int bytes = recv(sock, &state, sizeof(GameState), MSG_DONTWAIT);

            if (bytes == sizeof(GameState))
            {
                ball.x = state.ballX;
                ball.y = state.ballY;
                leftPaddle.y = state.leftPaddleY;
                leftScore = state.leftScore;
                rightScore = state.rightScore;
                gameOver = state.gameOver;
                winner = state.winner;
            }
            else if (bytes == 0)
            {
                connected = false;
                close(sock);
                std::cout << "Server Closed Connection\n";
            }
        }

        // 3. Draw
        BeginDrawing();

        ClearBackground(BLACK);

        if (!connected)
        {
            DrawText("Attempting to connect...", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 20, RED);
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

                DrawText("Waiting for server restart...",
                         SCREEN_WIDTH / 2 - 230,
                         SCREEN_HEIGHT / 2 + 80,
                         30,
                         GRAY);
            }
        }

        EndDrawing();
    }

    if (sock != -1)
        close(sock);
    CloseWindow();
    return 0;
}
