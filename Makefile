###############################################################################
#                                                                             #
#                     ███╗   ██╗ ██████╗ ██╗   ██╗ █████╗                     #
#                     ████╗  ██║██╔═══██╗██║   ██║██╔══██╗                    #
#                     ██╔██╗ ██║██║   ██║██║   ██║███████║                    #
#                     ██║╚██╗██║██║   ██║╚██╗ ██╔╝██╔══██║                    #
#                     ██║ ╚████║╚██████╔╝ ╚████╔╝ ██║  ██║                    #
#                     ╚═╝  ╚═══╝ ╚═════╝   ╚═══╝  ╚═╝  ╚═╝                    #
#               Written By: Kris Nóva    <admin@krisnova.net>                 #
#                                                                             #
###############################################################################

CC           ?= clang
TARGET       ?= tcpjack
DESTDIR      ?= /usr/local/bin
CFLAGS       ?= -I./include -g -O0 -Wall
LDFLAGS      ?=
LIBS         ?= 
STYLE        ?= Webkit
SOURCES      ?= src/tcpjack.c src/list.c src/proc.c

default: tcpjack
all:     tcpjack install

.PHONY: tcpjack
tcpjack: ## Compile for the local architecture
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

install: ## Install the program to /usr/local/bin
	@echo "Installing..."
	install -m 755 $(TARGET) $(DESTDIR)/$(TARGET)

clean: ## Clean your artifacts
	rm -f $(TARGET)

format: ## Format the code
	@echo "  ->  Formatting code"
	clang-format -i -style=$(STYLE) ./src/*.c
	clang-format -i -style=$(STYLE)  ./include/*.h

.PHONY: help
help: ## Show help messages for make targets
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[32m%-30s\033[0m %s\n", $$1, $$2}'
