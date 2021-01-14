# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/01/04 11:24:58 by llefranc          #+#    #+#              #
#    Updated: 2021/01/14 13:11:03 by llefranc         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	microshell
CC			=	clang
FLAGS		=	-Wall -Wextra -Werror

SRCS		=	microshell.c 

OBJS		=	$(SRCS:.c=.o)

all		: 	$(NAME)

$(NAME)	:	$(OBJS)
			$(CC) -o $(NAME) $(FLAGS) $(OBJS)

clean	:	
				rm -rf $(OBJS)

fclean	:	clean
				rm -rf $(NAME)

re		:	fclean all

.PHONY	:	all clean fclean re

%.o		:	%.c
			$(CC) $(FLAGS) -o $@ -c $<
# for creating .o with flags