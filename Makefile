CFLAGS=-std=c17 -g -fno-common -Wextra -Wpedantic
LFLAGS=-lGL -lXext -lXrandr -lX11 -lm
HEADER_FILES = src/la.h src/config.h src/navigation.h src/screenshot.h src/file_io.h src/shader.h src/boomer_arch.h
SRC_FILES = src/la.c src/config.c src/navigation.c src/screenshot.c src/file_io.c src/shader.c src/boomer_arch.c src/main.c

boomer-arch: clean
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
	@# copied from stack overflow and I cant remember where is it... 	
	-@rm -f boomer-arch *.o 2>/dev/null || true

.PHONY: clean
