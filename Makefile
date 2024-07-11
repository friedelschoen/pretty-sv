all: psvstat

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

psvstat: psvstat.o
	$(CC) $^  -o $@ $(LDFLAGS)

clean:
	rm -f psvstat psvstat.o

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f psvstat $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	sed "s/VERSION/$(VERSION)/g" < psvstat.1 > $(DESTDIR)$(PREFIX)/share/man/man1/psvstat.1
	chmod 644 $(DESTDIR)$(PREFIX)/share/man/man1/psvstat.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/psvstat \
		$(DESTDIR)$(PREFIX)/share/man/man1/psvstat.1

.PHONY: all clean install uninstall
