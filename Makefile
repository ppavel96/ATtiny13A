SRC_DIR = ./source
INC_DIR = ./include
OBJ_DIR = ./bin/obj
BIN_DIR = ./bin

APP_NAME = $(BIN_DIR)/emulator

CC = g++
CFLAGS = -I$(INC_DIR) -std=c++11 -O2

_DEPS = emulator.h
DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))

_OBJ = main.o emulator.o image_manager.o
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(APP_NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(APP_NAME)
