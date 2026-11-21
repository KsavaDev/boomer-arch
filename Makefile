CFLAGS=-std=c17 -g -fno-common -Wextra -Wpedantic
# -lglut -lm -- if default libs is not enough
LFLAGS=-lGL -lXext -lX11 -lm
HEADER_FILES = src/cmath.h src/la.h src/config.h src/navigation.h
SRC_FILES = src/la.c src/config.c src/navigation.c src/main.c

boomer-arch: 
	gcc -o boomer-arch $(HEADER_FILES) $(SRC_FILES) $(LFLAGS)


install:
	@echo "Installing boomer-arch..."
	@cp boomer-arch /usr/local/bin/
	@echo "Done."

uninstall:
	@echo "Uninstalling boomer-arch..."
	@rm /usr/local/bin/boomer-arch
	@echo "Done."

clean:
	@echo "Deleting application from this folder..."
	-@rm -f boomer-arch *.o 2>/dev/null || true
	@echo "Done."

.PHONY: clean



