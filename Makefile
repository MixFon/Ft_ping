# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: widraugr <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2018/11/29 13:07:44 by widraugr          #+#    #+#              #
#    Updated: 2020/11/17 08:56:53 by widraugr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_ping

FILE_C = main.c\
		 get_ip_str.c\
		 infill_destination.c\
		 infill_struct_hints.c\
		 preparation_to_send.c\
		 print_icmp_packege.c\
		 print_usege.c\
		 ft_atoi.c\
		 ft_isdigit.c\
		 ft_memcpy.c\
		 ft_memset.c\
		 ft_strdup.c\
		 ft_strlen.c\
		 ft_strnew.c\
		 sys_err.c

FLAGS = -Wall -Wextra -Werror -I libft -I include

DIRC = ./source/

VPATH = $(DIROBJ)

DIROBJ = ./obj/

OBJ = $(addprefix $(DIROBJ), $(FILE_C:.c=.o))

all : $(NAME)

$(NAME): $(DIROBJ) $(OBJ) ./include/ft_ping.h
	gcc $(FLAGS) $(OBJ) $(FMLXLIB) -o $(NAME)

$(DIROBJ)%.o : $(DIRC)%.c
	gcc -g $(FLAGS) -c $< -o $@

$(DIROBJ):
	mkdir -p $(DIROBJ)

clean:
	/bin/rm -rf $(DIROBJ)
	
fclean: clean
	/bin/rm -f $(NAME)
	/bin/rm -rf *.dSYM
	
re: fclean all 
