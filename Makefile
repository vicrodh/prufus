CFLAGS := $(shell pkg-config --cflags gtk4)
CLINKS := $(shell pkg-config --libs gtk4)

prufus: main.c
	cc $(CFLAGS) main.c -o prufus $(CLINKS)

main_compile_command.o: main.c
	cc $(CFLAGS) -c main.c



compile_commands: main_compile_command.o
	make --always-make --dry-run

clean:
	rm -f prufus
	rm -f *.o
