CC=gcc
bin_name=ani-cli-re

all: anicli

anicli: main.c
	$(CC) -O2 -lcurl -o $(bin_name) main.c

clean:
	rm anicli
