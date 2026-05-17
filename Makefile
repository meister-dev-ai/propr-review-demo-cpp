BUILD_DIR := build
DIST_DIR := dist
GENERATOR := $(BUILD_DIR)/sitegen
SRC := src/main.cpp

.PHONY: build clean test

build: $(GENERATOR)
	./$(GENERATOR)

$(GENERATOR): $(SRC)
	mkdir -p $(BUILD_DIR)
	g++ -std=c++17 -O2 -Wall -Wextra -pedantic -o $(GENERATOR) $(SRC)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) playwright-report test-results

test: build
	npm test
