# ################ #
#*    COULEURS    *#
# ################ #

CLASSIC	= \033[0m
WHITE	= \033[38;2;238;232;213m
BOLD	= \033[1m
BLACK	= \033[30m
RED   	= \033[31m
GREEN  	= \033[32m
BLUE 	= \033[38;2;0;36;148m
PURPLE 	= \033[38;2;211;54;130m
CYAN   	= \033[36m
YELLOW 	= \033[38;2;250;189;0m

# ############### #
#*   VARIABLES   *#
# ############### #

NAME = kiln

CXX = @clang++

LMAKE = @make --no-print-directory -C

SRCS =	srcs/main.cpp \
		srcs/kiln.cpp \
		srcs/utils.cpp \
		srcs/generate.cpp \
		srcs/fetch.cpp \
		srcs/print.cpp \
		srcs/watch.cpp \
		srcs/DateTime.cpp \
		srcs/termios.cpp

OBJS_DIR = .objs

OBJS = $(SRCS:%.cpp=$(OBJS_DIR)/%.o)

DEPS = $(OBJS:%.o=%.d)

CXXFLAGS = -std=c++20 -Wall -Wextra -Wno-missing-field-initializers -O3 -fPIE -MMD -MP -g3

INCLUDES = -I./srcs/includes/ -I./includes/ -I./hopt/includes/

# ############## #
#*    REGLES    *#
# ############## #

all: libhopt $(NAME)

libhopt:
	$(LMAKE) hopt

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -L./hopt -lhopt -o $(NAME)
	@echo "$(BOLD)$(CYAN)Executable $(NAME) créé avec succès!$(CLASSIC)"

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "$(GREEN)🗸 Compilation $(BOLD)$(YELLOW)$<$(CLASSIC)"
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(BOLD)$(RED)"
	$(LMAKE) hopt clean
	rm -rf $(OBJS_DIR)
	@echo "$(CLASSIC)"

fclean: clean
	@echo "$(BOLD)$(RED)"
	$(LMAKE) hopt fclean
	rm -f $(NAME)
	@echo "$(BOLD)$(GREEN)Tout a été supprimé... 🗑️\n$(CLASSIC)"

re: fclean all

.PHONY: all clean fclean re libhopt

-include $(DEPS)