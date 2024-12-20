# Compiler flags and GTK4 package configuration
CFLAGS := $(shell pkg-config --cflags gtk4)
CLINKS := $(shell pkg-config --libs gtk4)

# Target executable
TARGET := prufus

# Object files
OBJS := user_interface.o usb_validator.o

.PHONY: all install uninstall clean

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): main.c $(OBJS)
	cc $(CFLAGS) $(OBJS) main.c -o $(TARGET) $(CLINKS)

# Compile object files
user_interface.o: user_interface.c
	cc $(CFLAGS) -c user_interface.c

usb_validator.o: usb_validator.c
	cc $(CFLAGS) -c usb_validator.c

# Compile commands for tools like clangd
main_compile_command.o: main.c
	cc $(CFLAGS) -c main.c

compile_commands: main_compile_command.o
	make --always-make --dry-run

# Install the executable and additional files
install:
	@echo "Installing $(TARGET) to system directories..."
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@echo "Installation complete."

# Uninstall the installed files
uninstall:
	@echo "Removing $(TARGET) from system directories..."
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@echo "Uninstallation complete."

# Clean up build artifacts
clean:
	rm -f $(TARGET)
	rm -f *.o
	@echo "Cleaned up build artifacts."
