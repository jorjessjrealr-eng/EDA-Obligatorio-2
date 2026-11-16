#include "pila.h"
#include <stdlib.h>

typedef struct nodo {
  Operacion dato;
  struct nodo *siguiente;
} Nodo;

struct pila {
  Nodo *cima;
};

Pila *crearPila(void) {
  Pila *pila = malloc(sizeof(Pila));
  if (pila != NULL) {
    pila->cima = NULL;
  }
  return pila;
}

bool pilaVacia(const Pila *pila) { return pila == NULL || pila->cima == NULL; }

bool apilar(Pila *pila, const Operacion *operacion) {
  if (pila == NULL || operacion == NULL)
    return false;
  Nodo *nuevo = malloc(sizeof(Nodo));
  if (nuevo == NULL)
    return false;
  nuevo->dato = *operacion;
  nuevo->siguiente = pila->cima;
  pila->cima = nuevo;
  return true;
}

bool desapilar(Pila *pila, Operacion *salida) {
  if (pilaVacia(pila))
    return false;
  Nodo *borrar = pila->cima;
  if (salida != NULL) {
    *salida = borrar->dato;
  }
  pila->cima = borrar->siguiente;
  free(borrar);
  return true;
}

bool consultarCima(const Pila *pila, Operacion *cima) {
  if (pilaVacia(pila) || cima == NULL)
    return false;
  *cima = pila->cima->dato;
  return true;
}

void destruirPila(Pila *pila) {
  if (pila != NULL) {
    while (!pilaVacia(pila)) {
      desapilar(pila, NULL);
    }
    free(pila);
  }
}
