#!/usr/bin/make -f

#-------------------------------------------------------------------------------
# 1. Setting up necessary variables for the build process
#-------------------------------------------------------------------------------

# The compilers that we will be using (we will only use g++)
CC := gcc
CXX := g++

# $(wildcard *.h) finds all file names with patterns (random string + ".h")
HDRS := $(wildcard *.h)		
SRCS := $(wildcard *.cpp)	
OBJS := $(SRCS:.cpp=.o)		# replaces .cpp extension to .o (e.g., main.cpp -> main.o)
				# and stores the names to OBJS

# -Wall prints: all warnings
# -std=c++11: use of C++11
CFLAGS := -Wall -std=c++11

# -pthread: use of posix threads (necessary to use std::thread or pthreads)
LFLAGS := -pthread 

# executable binaries that we are building
TARGETS := part_1 part_2 part_3_client part_3_server

all: part_1 part_2 part_3_client part_3_server

part_1: part_1_main.o Book.o Library.o part_1_your_tasks.o
	$(CXX) $(LFLAGS) -o $@ $^

part_2: part_2_main.o part_2_your_task.o ThreadSleeper.o
	$(CXX) $(LFLAGS) -o $@ $^

part_3_client: part_3_client.o Message.o part_3_your_task.o
	$(CXX) $(LFLAGS) -o $@ $^

part_3_server: part_3_server.o Message.o part_3_your_task.o
	$(CXX) $(LFLAGS) -o $@ $^

$(OBJS): $(SRCS) $(HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(SRCS)


clean:
	rm -f *.o $(TARGETS)

.PHONY: clean debug
