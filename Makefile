CC = g++
CFLAGS = -Wall -std=c++17

# set path based on your own raylib installation
RAYLIB_PATH = ../../w64devkit/x86_64-w64-mingw32
INCLUDE_PATH = $(RAYLIB_PATH)/lib/include
LIB_PATH = $(RAYLIB_PATH)/lib
LIBS = -lraylib -lwinmm -lopengl32 -lgdi32

all: app.exe

app.exe: main.o
	$(CC) $(CFLAGS) -o app.exe main.o -L$(LIB_PATH) $(LIBS)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -I$(INCLUDE_PATH)

clean:
	del /Q *.o app.exe 2>nul || echo "Clean Completed"

.PHONY: all clean