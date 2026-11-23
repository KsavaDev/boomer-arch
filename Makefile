CFLAGS=-std=c17 -g -fno-common -Wextra -Wpedantic
LFLAGS=-lGL -lXext -lXrandr -lX11 -lm
HEADER_FILES = src/la.h src/config.h src/navigation.h src/screenshot.h src/file_io.h src/shader.h src/doomer.h
SRC_FILES = src/la.c src/config.c src/navigation.c src/screenshot.c src/file_io.c src/shader.c src/doomer.c src/main.c

doomer: clean
	gcc -o doomer $(HEADER_FILES) $(SRC_FILES) $(LFLAGS)

install:
	@echo "Installing doomer..."
	@cp doomer /usr/local/bin/
	@echo "Done."

uninstall:
	@echo "Uninstalling doomer..."
	@rm /usr/local/bin/doomer
	@echo "Done."

clean:
	@# copied from stack overflow and I cant remember where is it... 	
	-@rm -f doomer *.o 2>/dev/null || true

.PHONY: clean
