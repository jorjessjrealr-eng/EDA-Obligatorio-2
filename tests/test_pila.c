#include "pila.h"
#include <stdio.h>
#include <string.h>

void verificar(bool condicion, const char *nombre) {
  printf("%s %s\n", condicion ? "[OK]   " : "[FALLO]", nombre);
}

int main(void) {
  Pila *pila = crearPila();
  Operacion operacionA = {"A->B", "cambio de ruta"};
  Operacion operacionB = {"C->D", "reasignacion"};
  Operacion salida;

  verificar(pilaVacia(pila), "pila nueva esta vacia");
  verificar(!desapilar(pila, &salida), "desapilar vacia devuelve false");
  verificar(!consultarCima(pila, &salida),
            "consultar cima vacia devuelve false");

  apilar(pila, &operacionA);
  apilar(pila, &operacionB);
  verificar(!pilaVacia(pila), "despues de apilar no esta vacia");

  consultarCima(pila, &salida);
  verificar(strcmp(salida.ruta, "C->D") == 0, "la cima es el ultimo apilado");

  desapilar(pila, &salida);
  verificar(strcmp(salida.ruta, "C->D") == 0,
            "desapilar devuelve el ultimo (LIFO)");

  desapilar(pila, &salida);
  verificar(strcmp(salida.ruta, "A->B") == 0, "luego devuelve el primero");
  verificar(pilaVacia(pila), "queda vacia");

  destruirPila(pila);
  return 0;
}
