CC = g++
TARGET = main
MAINFILE = ./main.cpp

SRCDIR = ./src/
INCDIRS = /usr/include/lua5.1 ./inc/
LIBDIRS = /usr/bin/

SRCFILES = luaclass.cpp luahelper.cpp tcp_server.c luafuns.cpp log.c

SRC = ./$(MAINFILE) $(addprefix $(SRCDIR), $(SRCFILES))
INC = $(addprefix -I, $(INCDIRS))
LIB = $(addprefix -L, $(LIBDIRS))
LINK = -llua5.1 -lpthread

CFLAGS = $(LIB) $(INC) $(LINK)

all:
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

run:
	./$(TARGET)

clean:
	rm ./$(TARGET)

