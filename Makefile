CC=/home/yohanes/gcc-linaro-arm-linux-gnueabihf-4.7-2012.11-20121123_linux/bin/arm-linux-gnueabihf-gcc

menu: menu.c
	$(CC) -std=c99 menu.c -o menu  -I. -Llib -lSDL -lpthread -lm -lSDL_ttf -lfreetype -liconv
