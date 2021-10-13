SHELL = /bin/sh

CXX	 := -gcc
#CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror
LDFLAGS  := -L/usr/lib -lstdc++ -lm
BUILD	:= ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/apps
DEBUG 	 := $(APP_DIR)/debug
RELEASE  := $(APP_DIR)/release
TARGET   := test
INCLUDE  := -Iinclude/
SRC	 := $(wildcard src/*.c) $(wildcard src/**/*.c)

OBJECTS  := $(SRC:%.c=$(OBJ_DIR)/%.o)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS)

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)

.PHONY: all build clean debug release

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: APP_DIR := $(DEBUG)
debug: all

release: CXXFLAGS += -O2
release: APP_DIR := $(RELEASE)
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

