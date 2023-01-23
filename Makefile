PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

all:
	$(CC) -O -Werror -o snd snd.c
clean:
	rm snd
install: all
	mkdir -p "$(PREFIX)/bin"
	cp -f snd "$(PREFIX)/bin"
	chmod 755 "$(PREFIX)/bin/snd"
	mkdir -p "$(MANPREFIX)/man1"
	cp -f snd.1 "$(MANPREFIX)/man1/snd.1"
	chmod 644 "$(MANPREFIX)/man1/snd.1"
uninstall:
	rm "$(PREFIX)/bin/snd"
	rm "$(MANPREFIX)/man1/snd.1"
