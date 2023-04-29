leekboy:
	gcc -o leekboy src/main.c src/instructions.c src/frontend.c -Iinclude -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib64 -lSDL2
