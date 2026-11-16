#include "abb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct nodo {
  Paquete dato;
  struct nodo *izquierda;
  struct nodo *derecha;
} Nodo;

struct abb {
  Nodo *raiz;
};

Abb *plantarABB(void) {
  Abb *arbol = malloc(sizeof(Abb));
  if (arbol != NULL) {
    arbol->raiz = NULL;
  }
  return arbol;
}

static void podarNodos(Nodo *nodo) {
  if (nodo != NULL) {
    podarNodos(nodo->izquierda);
    podarNodos(nodo->derecha);
    free(nodo);
  }
}

void talarABB(Abb *arbol) {
  if (arbol != NULL) {
    podarNodos(arbol->raiz);
    free(arbol);
  }
}

static Nodo *crearNodo(const Paquete *paquete) {
  Nodo *nodo = malloc(sizeof(Nodo));
  if (nodo != NULL) {
    nodo->dato = *paquete;
    nodo->izquierda = NULL;
    nodo->derecha = NULL;
  }
  return nodo;
}

static Nodo *injertarNodo(Nodo *nodo, const Paquete *paquete,
                          bool *yaInsertado) {
  if (nodo == NULL) {
    Nodo *nuevo = crearNodo(paquete);
    *yaInsertado = (nuevo != NULL);
    return nuevo;
  }
  int comparacion = strcmp(paquete->trackingID, nodo->dato.trackingID);
  if (comparacion < 0) {
    nodo->izquierda = injertarNodo(nodo->izquierda, paquete, yaInsertado);
  } else if (comparacion > 0) {
    nodo->derecha = injertarNodo(nodo->derecha, paquete, yaInsertado);
  } else {
    *yaInsertado = false;
  }
  return nodo;
}

bool injertarPaquete(Abb *arbol, const Paquete *paquete) {
  if (arbol == NULL || paquete == NULL)
    return false;
  bool yaInsertado = false;
  arbol->raiz = injertarNodo(arbol->raiz, paquete, &yaInsertado);
  return yaInsertado;
}

static const Nodo *buscarNodo(const Nodo *nodo, const char *trackingID) {
  if (nodo == NULL)
    return NULL;
  int comparacion = strcmp(trackingID, nodo->dato.trackingID);
  if (comparacion == 0)
    return nodo;
  if (comparacion < 0)
    return buscarNodo(nodo->izquierda, trackingID);
  return buscarNodo(nodo->derecha, trackingID);
}

bool buscarPorID(const Abb *arbol, const char *trackingID, Paquete *salida) {
  if (arbol == NULL || trackingID == NULL || salida == NULL)
    return false;
  const Nodo *nodo = buscarNodo(arbol->raiz, trackingID);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}

static void enOrden(const Nodo *nodo) {
  if (nodo != NULL) {
    enOrden(nodo->izquierda);
    printf("%s | %s | %.2f kg | $%.2f\n", nodo->dato.trackingID,
           nodo->dato.destino, nodo->dato.peso, nodo->dato.costoEnvio);
    enOrden(nodo->derecha);
  }
}

void recorrerEnOrden(const Abb *arbol) {
  if (arbol != NULL) {
    enOrden(arbol->raiz);
  }
}

static int altura(const Nodo *nodo) {
  if (nodo == NULL)
    return -1;
  int alturaIzquierda = altura(nodo->izquierda);
  int alturaDerecha = altura(nodo->derecha);
  if (alturaIzquierda > alturaDerecha) {
    return 1 + alturaIzquierda;
  }
  return 1 + alturaDerecha;
}

int alturaABB(const Abb *arbol) {
  if (arbol == NULL)
    return -1;
  return altura(arbol->raiz);
}

static int profundidad(const Nodo *nodo, const char *trackingID) {
  if (nodo == NULL)
    return -1;
  int comparacion = strcmp(trackingID, nodo->dato.trackingID);
  if (comparacion == 0)
    return 0;
  int profundidadHijo = (comparacion < 0)
                            ? profundidad(nodo->izquierda, trackingID)
                            : profundidad(nodo->derecha, trackingID);
  return (profundidadHijo == -1) ? -1 : 1 + profundidadHijo;
}

int profundidadABB(const Abb *arbol, const char *trackingID) {
  if (arbol == NULL || trackingID == NULL)
    return -1;
  return profundidad(arbol->raiz, trackingID);
}

static float pesoTotal(const Nodo *nodo) {
  if (nodo == NULL)
    return 0;
  return nodo->dato.peso + pesoTotal(nodo->izquierda) +
         pesoTotal(nodo->derecha);
}

float pesoTotalABB(const Abb *arbol) {
  if (arbol == NULL)
    return 0;
  return pesoTotal(arbol->raiz);
}

static bool cumpleCondicion(const Paquete *paquete, float costoMinimo,
                            float costoMaximo, float pesoMaximo) {
  return paquete->costoEnvio >= costoMinimo &&
         paquete->costoEnvio <= costoMaximo && paquete->peso <= pesoMaximo;
}

static const Nodo *buscarCostoYPeso(const Nodo *nodo, float costoMinimo,
                                    float costoMaximo, float pesoMaximo) {
  if (nodo == NULL)
    return NULL;
  const Nodo *resultadoIzquierda =
      buscarCostoYPeso(nodo->izquierda, costoMinimo, costoMaximo, pesoMaximo);
  if (resultadoIzquierda != NULL)
    return resultadoIzquierda;
  if (cumpleCondicion(&nodo->dato, costoMinimo, costoMaximo, pesoMaximo))
    return nodo;
  return buscarCostoYPeso(nodo->derecha, costoMinimo, costoMaximo, pesoMaximo);
}

bool buscarPorCostoYPeso(const Abb *arbol, float costoMinimo, float costoMaximo,
                         float pesoMaximo, Paquete *salida) {
  if (arbol == NULL || salida == NULL)
    return false;
  const Nodo *nodo =
      buscarCostoYPeso(arbol->raiz, costoMinimo, costoMaximo, pesoMaximo);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}
