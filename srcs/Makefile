# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sel-melc <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/10/15 19:41:09 by sel-melc          #+#    #+#              #
#    Updated: 2019/11/18 15:58:03 by sel-melc         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

UNAME := $(shell uname)

SRC = durex.c

CC = gcc
CFLAGS = -c -Wall -Wextra -Werror
OBJ = $(SRC:.c=.o) 

NAME = Durex

all : $(NAME)

$(NAME) : $(OBJ) durex.h
	$(CC) -o $(NAME) $(OBJ)


%.o: %.c
	$(CC) $(CFLAGS) $? -o $@

clean : 
	rm -f $(OBJ)
fclean :
	rm -f $(OBJ) $(NAME)

re : fclean all

.PHONY : clean fclean re all
