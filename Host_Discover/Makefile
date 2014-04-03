#Makefile
#
# Instrucciones para compilar

NAME = ft
OBJS = main.o fthd.o server.o client.o netlib/net.o utils/utils.o 

CC = g++
CFLAGS = -Wall -O2 -pthread

all: $(OBJS) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(NAME)

doc:
	doxygen
	


