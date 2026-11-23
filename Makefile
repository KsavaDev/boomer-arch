CFLAGS=-std=c17 -g -fno-common -Wextra -Wpedantic
LFLAGS=-lGL -lXext -lXrandr -lX11 -lm
HEADER_FILES = src/util/la.h src/app/config.h src/app/navigation.h src/app/screenshot.h src/util/file_io.h src/app/shader.h src/app/doomer.h
SRC_FILES = src/util/la.c src/app/config.c src/app/navigation.c src/app/screenshot.c src/util/file_io.c src/app/shader.c src/app/doomer.c src/main.c

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
