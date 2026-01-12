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

#debug with valgrind
valgrind: clean

	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

	cmake --build $(BUILD_DIR)

	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--log-file=valgrind-out.txt \
		./$(BUILD_DIR)/$(EXEC_NAME)
	
	@echo "---------------------------------------------------"
	@echo "Teste concluído. Verifica o ficheiro 'valgrind-out.txt'"
	@echo "---------------------------------------------------"