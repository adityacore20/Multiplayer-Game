# Multiplayer Pong

A simple multiplayer Pong clone written in C++ using [Raylib](https://www.raylib.com/) for graphics and raw TCP sockets for networking. 

I built this to learn a bit more about real-time game networking. It uses a standard client-server architecture where the server is authoritative. The server runs the game loop, handles the ball collision, and controls the left paddle. The client connects to the server, sends its paddle position (right paddle), and renders the game state received from the server.

## Features
* Classic Pong gameplay (first to 5 wins)
* Server-authoritative networking (TCP)
* Simple Makefile build system

## Requirements
You'll need a C++17 compiler (like GCC or Clang) and the **Raylib** library installed on your system.

### Installing Raylib

**Mac:**
The easiest way is using Homebrew:
```bash
brew install raylib
```

**Linux (Debian/Ubuntu):**
You can install the development libraries via apt:
```bash
sudo apt install libraylib-dev build-essential
```

**Windows:**
On Windows, setting up C++ libraries can be a bit trickier. I recommend using MSYS2 and MinGW-w64:
1. Install MSYS2.
2. Open the MSYS2 MinGW 64-bit terminal.
3. Run `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-raylib make`.
*(Note: You might need to tweak the `makefile` slightly on Windows to link against `-lgdi32` and `-lwinmm` depending on your setup).*

## How to Build
Once you have your compiler and Raylib set up, just open your terminal in the project directory and run:

```bash
make
```
This will compile both the `server` and `client` executables.

## How to Play

You need to run the server first, and then the client. By default, the client looks for the server on `127.0.0.1` (localhost) at port `8080`.

1. **Start the server:**
   ```bash
   ./server
   ```
   The server window will open and wait for a player to connect. 

2. **Start the client:**
   Open a second terminal window and run:
   ```bash
   ./client
   ```

### Controls
* **Server (Left Player):** Use `W` and `S` to move your paddle up and down.
* **Client (Right Player):** Use the `Up` and `Down` arrow keys to move your paddle.
* **Restarting:** When a game finishes, the **Server** can press `R` to restart the match.

Enjoy!
