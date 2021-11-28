CC=gcc
LIBS=-lcurl
bin_name=ani-cli-re

all: anicli

anicli: main.c
	$(CC) -O2 $(LIBS) -o $(bin_name) main.c

clean:
	rm anicli
