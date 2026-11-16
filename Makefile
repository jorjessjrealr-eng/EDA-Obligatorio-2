CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -Iinclude

ifeq ($(OS),Windows_NT)
EXT = .exe
PREFIJO =
BORRAR = del /Q
else
EXT =
PREFIJO = ./
BORRAR = rm -f
endif

PRUEBAS = test_pila$(EXT) test_monton$(EXT) test_abb$(EXT) test_avl$(EXT)
HEADERS = $(wildcard include/*.h)

PROGRAMA = logiexpress$(EXT)
FUENTES = src/main.c src/pila.c src/monton.c src/abb.c src/avl.c

test: $(PRUEBAS)
	$(PREFIJO)test_pila$(EXT)
	$(PREFIJO)test_monton$(EXT)
	$(PREFIJO)test_abb$(EXT)
	$(PREFIJO)test_avl$(EXT)

valgrind: $(PRUEBAS)
	valgrind --leak-check=full ./test_pila
	valgrind --leak-check=full ./test_monton
	valgrind --leak-check=full ./test_abb
	valgrind --leak-check=full ./test_avl

test_pila$(EXT): tests/test_pila.c src/pila.c $(HEADERS)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@

test_monton$(EXT): tests/test_monton.c src/monton.c $(HEADERS)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@

test_abb$(EXT): tests/test_abb.c src/abb.c $(HEADERS)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@

test_avl$(EXT): tests/test_avl.c src/avl.c $(HEADERS)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@

$(PROGRAMA): $(FUENTES) $(HEADERS)
	$(CC) $(CFLAGS) $(FUENTES) -o $@ -lncurses

run: $(PROGRAMA)
	$(PREFIJO)$(PROGRAMA)

clean:
	-$(BORRAR) $(PRUEBAS) $(PROGRAMA)

.PHONY: test valgrind run clean
