CC=gcc
CFLAGS=-I.

all: replace_content

replace_content: replace_content.c
	$(CC) -o replace_content replace_content.c -lcurl -pthread

clean:
	rm -rf replace_contento



