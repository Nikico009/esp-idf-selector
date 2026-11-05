CC = gcc
CFLAGS = -O3 -Wall -Wextra
TARGET = esp-idf
INSTALL_DIR = $(HOME)/.local/bin

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): esp-idf.c
	$(CC) $(CFLAGS) esp-idf.c -o $(TARGET)

install: $(TARGET)
	@mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "[+] Installed $(TARGET) successfully to $(INSTALL_DIR)"

uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "[-] Removed $(TARGET) from $(INSTALL_DIR)"

clean:
	rm -f $(TARGET)