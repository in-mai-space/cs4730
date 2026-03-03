#!/usr/bin/make -f

CC := gcc
CXX := g++

SVR_HDRS := $(wildcard server/include/*.h)
SVR_SRCS := $(wildcard server/src/*.cpp)
SVR_OBJS := $(notdir $(SVR_SRCS:.cpp=.o))

CLNT_HDRS := $(wildcard client/include/*.h)
CLNT_SRCS := $(wildcard client/src/*.cpp)
CLNT_OBJS := $(notdir $(CLNT_SRCS:.cpp=.o))

CMN_HDRS := $(wildcard common/include/*.h)
CMN_SRCS := $(wildcard common/src/*.cpp)
CMN_OBJS := $(notdir $(CMN_SRCS:.cpp=.o))

CFLAGS := -Wall -std=c++11 -Iclient/include -Iserver/include -Icommon/include 

LFLAGS := -pthread 

TARGET := server_binary client_binary

all: $(TARGET)

debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

server_binary: $(SVR_OBJS) $(CMN_OBJS)
	$(CXX) $(LFLAGS) -o $@ $^ 

$(SVR_OBJS): $(SVR_SRCS) $(SVR_HDRS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(SVR_SRCS)

client_binary: $(CLNT_OBJS) $(CMN_OBJS)
	$(CXX) $(LFLAGS) -o $@ $^ 

$(CLNT_OBJS): $(CLNT_SRCS) $(CLNT_HDRS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CLNT_SRCS)
	
$(CMN_OBJS): $(CMN_SRCS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CMN_SRCS)

clean:
	rm -f *.o server_binary client_binary

format:
	@echo "Formatting C++ code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i -style="{BasedOnStyle: Google, IndentWidth: 4, TabWidth: 4}"; \
		echo "Code formatted successfully."; \
	else \
		echo "clang-format not found. Install with: brew install clang-format"; \
	fi

lint:
	@echo "Linting C++ code..."
	@if command -v clang-tidy >/dev/null 2>&1; then \
		find . -name "*.cpp" | xargs clang-tidy -checks="-*,readability-*,performance-*,modernize-*" --; \
		echo "Code linted successfully."; \
	else \
		echo "clang-tidy not found. Install with: brew install llvm"; \
	fi

debug-server: server_binary
	@echo "Starting server in debugger..."
	lldb -o "run 8080 2" ./server_binary

debug-client: client_binary
	@echo "Starting client in debugger..."
	lldb -o "run 127.0.0.1 8080 3 5 0" ./client_binary

.PHONY: clean debug format lint
