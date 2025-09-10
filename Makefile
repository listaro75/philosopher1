NAME = philo

SRCDIR = src
INCDIR = include
BINDIR = bin

SRCS = main.c utils.c init.c philo.c monitor.c

OBJS = $(addprefix $(BINDIR)/, $(SRCS:.c=.o))

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -I$(INCDIR)

all: $(NAME)

$(NAME): $(BINDIR) $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/philo.h | $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re