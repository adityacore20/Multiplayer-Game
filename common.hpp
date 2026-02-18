#ifndef COMMON_HPP
#define COMMON_HPP

#include <raylib.h>

// Game Constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;
const int PORT = 8080;

// Shared Game Objects
class Ball {
   public:
    float x, y;
    int sx, sy;
    int radius;
    
    void Draw() { DrawCircle((int)x, (int)y, radius, WHITE); }
    
    void Update() {
        x += sx;
        y += sy;
        // Bounce logic
        if (y + radius >= SCREEN_HEIGHT || y - radius <= 0) sy *= -1;
        // Reset if out of bounds (score) - Optional for now
        
    }
};

class Paddle {
   public:
    float x, y, width, height;
    int speed;
    
    void Draw() { DrawRectangle((int)x, (int)y, (int)width, (int)height, WHITE); }
    
    // Server uses this for logic, Client uses this for prediction
    void Move(bool up, bool down) {
        if (up) y -= speed;
        if (down) y += speed;
        if (y <= 0) y = 0;
        if (y + height >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - height;
    }
};

// The Data Packet sent from Server -> Client
struct GameState {
    float ballX, ballY;
    float leftPaddleY;
    float rightPaddleY;
    int leftScore;
    int rightScore;
    bool gameOver;
    int winner;
};

#endif