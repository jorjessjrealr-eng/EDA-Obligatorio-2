#include "avl.h"
#include <stdlib.h>

typedef struct nodo {
  Estacion dato;
  struct nodo *izquierda;
  struct nodo *derecha;
  int altura;
} Nodo;

struct avl {
  Nodo *raiz;
};

Avl *plantarAVL(void) {
  Avl *arbol = malloc(sizeof(Avl));
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

void talarAVL(Avl *arbol) {
  if (arbol != NULL) {
    podarNodos(arbol->raiz);
    free(arbol);
  }
}

static int alturaNodo(const Nodo *nodo) {
  if (nodo == NULL)
    return -1;
  return nodo->altura;
}

static int maximo(int primero, int segundo) {
  if (primero > segundo)
    return primero;
  return segundo;
}

static void actualizarAltura(Nodo *nodo) {
  nodo->altura =
      1 + maximo(alturaNodo(nodo->izquierda), alturaNodo(nodo->derecha));
}

static int factorBalance(const Nodo *nodo) {
  if (nodo == NULL)
    return 0;
  return alturaNodo(nodo->izquierda) - alturaNodo(nodo->derecha);
}

static Nodo *rotarDerecha(Nodo *raiz) {
  Nodo *nuevaRaiz = raiz->izquierda;
  raiz->izquierda = nuevaRaiz->derecha;
  nuevaRaiz->derecha = raiz;
  actualizarAltura(raiz);
  actualizarAltura(nuevaRaiz);
  return nuevaRaiz;
}

static Nodo *rotarIzquierda(Nodo *raiz) {
  Nodo *nuevaRaiz = raiz->derecha;
  raiz->derecha = nuevaRaiz->izquierda;
  nuevaRaiz->izquierda = raiz;
  actualizarAltura(raiz);
  actualizarAltura(nuevaRaiz);
  return nuevaRaiz;
}

static Nodo *balancear(Nodo *nodo) {
  actualizarAltura(nodo);
  int balance = factorBalance(nodo);
  if (balance > 1) {
    if (factorBalance(nodo->izquierda) < 0) {
      nodo->izquierda = rotarIzquierda(nodo->izquierda);
    }
    return rotarDerecha(nodo);
  }
  if (balance < -1) {
    if (factorBalance(nodo->derecha) > 0) {
      nodo->derecha = rotarDerecha(nodo->derecha);
    }
    return rotarIzquierda(nodo);
  }
  return nodo;
}

static Nodo *crearNodo(const Estacion *estacion) {
  Nodo *nodo = malloc(sizeof(Nodo));
  if (nodo != NULL) {
    nodo->dato = *estacion;
    nodo->izquierda = NULL;
    nodo->derecha = NULL;
    nodo->altura = 0;
  }
  return nodo;
}

static Nodo *injertarNodo(Nodo *nodo, const Estacion *estacion,
                          bool *yaInsertado) {
  if (nodo == NULL) {
    Nodo *nuevo = crearNodo(estacion);
    *yaInsertado = (nuevo != NULL);
    return nuevo;
  }
  if (estacion->idEstacion < nodo->dato.idEstacion) {
    nodo->izquierda = injertarNodo(nodo->izquierda, estacion, yaInsertado);
  } else if (estacion->idEstacion > nodo->dato.idEstacion) {
    nodo->derecha = injertarNodo(nodo->derecha, estacion, yaInsertado);
  } else {
    *yaInsertado = false;
  }
  return balancear(nodo);
}

bool injertarEstacion(Avl *arbol, const Estacion *estacion) {
  if (arbol == NULL || estacion == NULL)
    return false;
  bool yaInsertado = false;
  arbol->raiz = injertarNodo(arbol->raiz, estacion, &yaInsertado);
  return yaInsertado;
}

static Nodo *nodoMinimo(Nodo *nodo) {
  Nodo *actual = nodo;
  while (actual->izquierda != NULL) {
    actual = actual->izquierda;
  }
  return actual;
}

static Nodo *quitarNodo(Nodo *nodo, int idEstacion, bool *yaEliminado) {
  if (nodo == NULL)
    return NULL;
  if (idEstacion < nodo->dato.idEstacion) {
    nodo->izquierda = quitarNodo(nodo->izquierda, idEstacion, yaEliminado);
  } else if (idEstacion > nodo->dato.idEstacion) {
    nodo->derecha = quitarNodo(nodo->derecha, idEstacion, yaEliminado);
  } else {
    *yaEliminado = true;
    if (nodo->izquierda == NULL || nodo->derecha == NULL) {
      Nodo *hijo = nodo->izquierda;
      if (hijo == NULL) {
        hijo = nodo->derecha;
      }
      free(nodo);
      return hijo;
    }
    Nodo *sucesor = nodoMinimo(nodo->derecha);
    nodo->dato = sucesor->dato;
    bool sucesorEliminado = false;
    nodo->derecha =
        quitarNodo(nodo->derecha, nodo->dato.idEstacion, &sucesorEliminado);
  }
  return balancear(nodo);
}

bool podarEstacion(Avl *arbol, int idEstacion) {
  if (arbol == NULL)
    return false;
  bool yaEliminado = false;
  arbol->raiz = quitarNodo(arbol->raiz, idEstacion, &yaEliminado);
  return yaEliminado;
}

static const Nodo *buscarNodo(const Nodo *nodo, int idEstacion) {
  if (nodo == NULL)
    return NULL;
  if (idEstacion == nodo->dato.idEstacion)
    return nodo;
  if (idEstacion < nodo->dato.idEstacion)
    return buscarNodo(nodo->izquierda, idEstacion);
  return buscarNodo(nodo->derecha, idEstacion);
}

bool buscarEstacion(const Avl *arbol, int idEstacion, Estacion *salida) {
  if (arbol == NULL || salida == NULL)
    return false;
  const Nodo *nodo = buscarNodo(arbol->raiz, idEstacion);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}

int alturaAVL(const Avl *arbol) {
  if (arbol == NULL)
    return -1;
  return alturaNodo(arbol->raiz);
}

static int alturaRecursiva(const Nodo *nodo) {
  if (nodo == NULL)
    return -1;
  return 1 + maximo(alturaRecursiva(nodo->izquierda),
                    alturaRecursiva(nodo->derecha));
}

int alturaSubredAVL(const Avl *arbol, int idRaiz) {
  if (arbol == NULL)
    return -1;
  return alturaRecursiva(buscarNodo(arbol->raiz, idRaiz));
}

static float costoTotal(const Nodo *nodo) {
  if (nodo == NULL)
    return 0;
  return nodo->dato.costoTransporte + costoTotal(nodo->izquierda) +
         costoTotal(nodo->derecha);
}

float costoTotalSubredAVL(const Avl *arbol, int idRaiz) {
  if (arbol == NULL)
    return 0;
  return costoTotal(buscarNodo(arbol->raiz, idRaiz));
}

static const Nodo *buscarCostos(const Nodo *nodo, float costoMinimo,
                                float costoMaximoSubred) {
  if (nodo == NULL)
    return NULL;
  const Nodo *resultadoIzquierda =
      buscarCostos(nodo->izquierda, costoMinimo, costoMaximoSubred);
  if (resultadoIzquierda != NULL)
    return resultadoIzquierda;
  if (nodo->dato.costoTransporte >= costoMinimo &&
      costoTotal(nodo) <= costoMaximoSubred)
    return nodo;
  return buscarCostos(nodo->derecha, costoMinimo, costoMaximoSubred);
}

bool buscarPorCostos(const Avl *arbol, float costoMinimo,
                     float costoMaximoSubred, Estacion *salida) {
  if (arbol == NULL || salida == NULL)
    return false;
  const Nodo *nodo = buscarCostos(arbol->raiz, costoMinimo, costoMaximoSubred);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}
