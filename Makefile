# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/09 18:38:40 by vzashev           #+#    #+#              #
#    Updated: 2025/02/19 17:17:24 by vzashev          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Makefile

NAME = webserv

SRC = srcs/main.cpp \
      srcs/CGI/srcs/CGIExecutor.cpp \
      srcs/Config/srcs/ConfigParser.cpp \
      srcs/Config/srcs/ServerConfig.cpp \
      srcs/Config/srcs/LocationConfig.cpp \
      srcs/Core/srcs/Server.cpp \
      srcs/Core/srcs/Client.cpp \
      srcs/Core/srcs/EventLoop.cpp \
      srcs/Errors/srcs/ErrorHandler.cpp \
      srcs/HTTP/srcs/Request.cpp \
      srcs/HTTP/srcs/Response.cpp \
      srcs/HTTP/srcs/RequestParser.cpp \
      srcs/HTTP/srcs/ResponseGenerator.cpp \
      srcs/Utils/srcs/FileHandler.cpp \
      srcs/Utils/srcs/StringUtils.cpp \
      srcs/Utils/srcs/MimeTypes.cpp

OBJ = $(SRC:.cpp=.o)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g -O3 \
           -Iincs \
           -Isrcs/CGI/incs \
           -Isrcs/Config/incs \
           -Isrcs/Core/incs \
           -Isrcs/Errors/incs \
           -Isrcs/HTTP/incs \
           -Isrcs/Utils/incs

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
