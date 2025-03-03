CC?=gcc
CFLAGS+=-pedantic -Wall -Werror

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

tablize: tablize.c tablize.h
	$(CC) $(CFLAGS) -o $@ -g $^ -lm
	$(RM) *.o

.PHONY: install
install: tablize
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 751 tablize $(DESTDIR)$(PREFIX)/bin/
	$(RM) tablize

.PHONY: uninstall
uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/tablize

.PHONY: clean
clean:
	$(RM) $(TARGET)

