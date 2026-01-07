BUILD_DIR = build
EXEC_NAME = ctt 

all: build

#config the project
configure:
	cmake -S . -B $(BUILD_DIR)

#compile the project
build: configure
		cmake --build $(BUILD_DIR)

#compile and run the executable
run: build
		@echo "--- Executando $(EXEC_NAME) ---"
		./$(BUILD_DIR)/ctt

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all configure build run clean