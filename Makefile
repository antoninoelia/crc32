CFLAGS = -Wall -Wextra -pedantic -std=c99
O_LEVEL = -O2
crc: crc.c
	gcc $(O_LEVEL) $(CFLAGS) -o crc crc.c
