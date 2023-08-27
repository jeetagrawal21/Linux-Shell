CC = gcc
CFLAGS = -g -std=gnu90 -Wall -Werror -pedantic -Wextra

LinuxShell: shell.c
	$(CC) -o $@ $(CFLAGS) $<

clean:
	rm -rf *.o LinuxShell
