# ################ #
#*    COULEURS    *#
# ################ #

CLASSIC	= \033[0m
WHITE	= \033[38;2;238;232;213m
BOLD	= \033[1m
UNDERLINE	= \033[4m
# BLACK	= \033[38;2;0;0;0m
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

# ./FPP/sources/Preprocessing/Preprocessing.cpp
# ./FPP/sources/Tokenizer/Tokenizer.cpp

SRCS =	./kiln.cpp

LMAKE = @make -j --no-print-directory -C

OBJS_DIR = .objs

OBJS = $(SRCS:%.cpp=$(OBJS_DIR)/%.obj)

DEPS = $(OBJS:%.obj=%.d)

INCLUDES =

CFLAGS = -std=c++20 -Wall -Wextra -Wno-comment -Wno-unknown-pragmas -Wno-vla-cxx-extension -Wno-switch-bool -MMD -O3 -g3 -I ./includes/ -I ./hopt/ #-march=native
#-Wno-non-template-friend 
#-Wno-unsupported-friend

LIB = -lpthread -L./hopt/ -lhopt

# ############## #
#*    REGLES    *#
# ############## #

all: check_compilation $(NAME)

check_compilation:
	@if [ -f $(NAME) ] && \
		[ -n "$(strip $(OBJS))" ] && \
		[ -z "$$(find $(SRCS) -newer $(NAME) 2>/dev/null)" ]; then \
		echo "$(BOLD)$(PURPLE)Tous les fichiers $(UNDERLINE)$(YELLOW)$(NAME)$(CLASSIC)$(BOLD)$(PURPLE) sont déjà compilés !$(CLASSIC)"; \
		exit 0; \
	fi

$(NAME): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) $(LIB) -o $@
	@echo "$(BOLD)$(CYAN)Exécutable $(NAME) créé avec succès!$(CLASSIC)"

$(OBJS_DIR)/%.obj: %.cpp
	@mkdir -p $(@D)
	@echo "$(GREEN)🗸 Compilation $(BOLD)$(YELLOW)$<$(CLASSIC)"
	$(CXX) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	@echo "$(BOLD)$(RED)"
	rm -rf $(OBJS_DIR)
	@echo "$(CLASSIC)"

fclean: clean
	@echo "$(BOLD)$(RED)"
	rm -f $(NAME)
	@echo "$(BOLD)$(GREEN)Tout a été supprimé... 🗑️\n$(CLASSIC)"

re: fclean all

.PHONY: all lib libre clean fclean re
-include $(DEPS)

# ############## #
#*    AUTRES    *#
# ############## #

YAY:
	@echo "$(PURPLE)$(BOLD)Compilation avec les flags suivants : "
	@echo $(CFLAGS)
	@echo "$(CLASSIC)"
