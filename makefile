# Nombre del compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -Wextra -std=c11 `pkg-config --cflags gtk+-3.0`

# Librerías
LIBS = -lm `pkg-config --libs gtk+-3.0`

# Nombre del ejecutable
TARGET = hamilton

# Directorio fuente
SRC_DIR = src

# Archivos fuente
SRC = $(SRC_DIR)/logic.c $(SRC_DIR)/mainglade.c

# Regla por defecto
all: $(TARGET)

# Cómo construir el ejecutable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Ejecutar el programa
run: $(TARGET)
	./$(TARGET)

# Limpiar los archivos generados
clean:
	rm -f $(TARGET)
