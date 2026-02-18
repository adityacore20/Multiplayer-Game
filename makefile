CC = g++
CFLAGS = -Wall -std=c++17
LDFLAGS = -lraylib -lm

all: server client

server: server.cpp common.hpp
	$(CC) $(CFLAGS) server.cpp -o server $(LDFLAGS)

client: client.cpp common.hpp
	$(CC) $(CFLAGS) client.cpp -o client $(LDFLAGS)

clean:
	rm -f server client