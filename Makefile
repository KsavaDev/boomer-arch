CFLAGS=-std=c17 -g -fno-common -Wextra -Wpedantic
LFLAGS=-lGL -lXext -lXrandr -lX11 -lm
X11_BACKEND_DIR  = src/app/backend/X11
X11_HEADER_FILES = $(X11_BACKEND_DIR)/X11_backend.h $(X11_BACKEND_DIR)/X11_screenshot.h
X11_SRC_FILES    = $(X11_BACKEND_DIR)/X11_backend.c $(X11_BACKEND_DIR)/X11_screenshot.c
HEADER_FILES = src/util/la.h src/app/config.h src/app/navigation.h $(X11_HEADER_FILES) src/util/file_io.h src/app/shader.h src/app/doomer.h
SRC_FILES    = src/util/la.c src/app/config.c src/app/navigation.c $(X11_SRC_FILES) src/util/file_io.c src/app/shader.c src/app/doomer.c src/main.c

doomer: clean
	gcc -o doomer $(CFLAGS) $(HEADER_FILES) $(SRC_FILES) $(LFLAGS)

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
