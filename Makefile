CC = g++
TARGET = main
MAINFILE = main.cpp

SRCDIR = ./src/
MY_LIBS = ./cpp_lua_interface/
INCDIRS = /usr/include/lua5.1 ./inc/ $(addsuffix inc/, $(MY_LIBS))
LIBDIRS = /usr/bin/ ./cpp_lua_interface

SRCFILES = tcp_server.c luafuns.cpp log.c

SRC = ./$(MAINFILE) $(addprefix $(SRCDIR), $(SRCFILES))
INC = $(addprefix -I, $(INCDIRS))
LIB = $(addprefix -L, $(LIBDIRS))
LINK = -llua5.1 -lpthread -lluainterface
LDFLAGS = `pkg-config --libs openssl`

CFLAGS = $(LIB) $(INC) $(LINK)

.PHONY: clean run

all: libs
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) $(LDFLAGS)

libs:
	for lib_dir in $(MY_LIBS) ; do \
		make -C $$lib_dir lib ; \
	done

run:
	./$(TARGET)

clean:
	rm ./$(TARGET)
	for lib_dir in $(MY_LIBS) ; do \
		make -C $$lib_dir clean; \
	done
