CC = gcc
CFLAGS = -Wall -Iinclude -g

SRC_DIR = src
OUT_DIR = out
OBJ_DIR = $(OUT_DIR)/obj

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET_POWER_MANAGER = $(OUT_DIR)/powerManager
TARGET_COLLECTOR = $(OUT_DIR)/dataCollector
TARGET_MINWRITER = $(OUT_DIR)/minWriter

all: $(TARGET_POWER_MANAGER) $(TARGET_COLLECTOR) $(TARGET_MINWRITER)

collector: $(TARGET_COLLECTOR)

powermanager: $(TARGET_POWER_MANAGER)

minwriter: $(TARGET_MINWRITER)

$(TARGET_POWER_MANAGER): $(OBJ_DIR)/powerManager.o $(OBJ_DIR)/processChores.o
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_COLLECTOR): $(OBJ_DIR)/dataCollector.o $(OBJ_DIR)/processChores.o
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_MINWRITER): $(OBJ_DIR)/minWriter.o $(OBJ_DIR)/processChores.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(OUT_DIR)

.PHONY: clean
