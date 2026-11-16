// ABB (arbol binario de busqueda) de paquetes, ordenado por trackingID.
// Menores a la izquierda, mayores a la derecha. No se auto-balancea.
//
// Formato de comentarios: arriba de cada funcion va que hace; debajo de la
// firma van su Pre (lo que se debe cumplir antes de llamarla) y su Pos
// (lo que queda cierto despues).
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
  Nodo *raiz; // NULL = arbol vacio
};

// Crea un arbol vacio.
Abb *crearABB(void) {
  // Pre: ninguna.
  // Pos: devuelve un arbol vacio, o NULL si no hubo memoria.
  Abb *arbol = malloc(sizeof(Abb));
  if (arbol != NULL) {
    arbol->raiz = NULL;
  }
  return arbol;
}

// Libera un subarbol en postorden: primero los hijos, despues el nodo.
static void destruirNodos(Nodo *nodo) {
  // Pre: nodo es NULL o raiz de un subarbol creado por este modulo.
  // Pos: todos los nodos del subarbol quedan liberados.
  if (nodo != NULL) {
    destruirNodos(nodo->izquierda);
    destruirNodos(nodo->derecha);
    free(nodo);
  }
}

// Libera el arbol completo.
void destruirABB(Abb *arbol) {
  // Pre: arbol es NULL o viene de crearABB y no fue destruido antes.
  // Pos: toda la memoria queda liberada. El puntero ya no se puede usar.
  if (arbol != NULL) {
    destruirNodos(arbol->raiz);
    free(arbol);
  }
}

// Crea un nodo hoja con una copia del paquete.
static Nodo *crearNodo(const Paquete *paquete) {
  // Pre: paquete no es NULL.
  // Pos: devuelve un nodo sin hijos con el dato copiado, o NULL si no hubo memoria.
  Nodo *nodo = malloc(sizeof(Nodo));
  if (nodo != NULL) {
    nodo->dato = *paquete;
    nodo->izquierda = NULL;
    nodo->derecha = NULL;
  }
  return nodo;
}

// Inserta recursivo en el subarbol y devuelve su raiz.
static Nodo *insertarNodo(Nodo *nodo, const Paquete *paquete,
                          bool *yaInsertado) {
  // Pre: paquete y yaInsertado no son NULL. nodo es NULL o raiz de un ABB valido.
  // Pos: devuelve la raiz del subarbol (la misma, o el nodo nuevo si estaba
  //      vacio). El subarbol sigue ordenado. *yaInsertado es true solo si se
  //      agrego un nodo; es false si el ID estaba repetido o fallo el malloc.
  if (nodo == NULL) {
    Nodo *nuevo = crearNodo(paquete);
    *yaInsertado = (nuevo != NULL);
    return nuevo;
  }
  int comparacion = strcmp(paquete->trackingID, nodo->dato.trackingID);
  if (comparacion < 0) {
    nodo->izquierda = insertarNodo(nodo->izquierda, paquete, yaInsertado);
  } else if (comparacion > 0) {
    nodo->derecha = insertarNodo(nodo->derecha, paquete, yaInsertado);
  } else {
    *yaInsertado = false; // duplicado: no se inserta
  }
  return nodo;
}

// Inserta un paquete en el arbol (se guarda una copia).
bool insertarPaquete(Abb *arbol, const Paquete *paquete) {
  // Pre: arbol y paquete no son NULL.
  // Pos: devuelve true si se agrego (el arbol tiene un paquete mas y sigue
  //      ordenado). Devuelve false si el trackingID ya existia, si hubo algun
  //      NULL o si fallo el malloc; en esos casos el arbol no cambia.
  if (arbol == NULL || paquete == NULL)
    return false;
  bool yaInsertado = false;
  arbol->raiz = insertarNodo(arbol->raiz, paquete, &yaInsertado);
  return yaInsertado;
}

// Busca un ID en el subarbol.
static const Nodo *buscarNodo(const Nodo *nodo, const char *trackingID) {
  // Pre: trackingID no es NULL. nodo es NULL o raiz de un ABB valido.
  // Pos: devuelve el nodo con ese ID, o NULL si no existe. No modifica nada.
  if (nodo == NULL)
    return NULL;
  int comparacion = strcmp(trackingID, nodo->dato.trackingID);
  if (comparacion == 0)
    return nodo;
  if (comparacion < 0)
    return buscarNodo(nodo->izquierda, trackingID);
  return buscarNodo(nodo->derecha, trackingID);
}

// Busca un paquete por su trackingID.
bool buscarPorID(const Abb *arbol, const char *trackingID, Paquete *salida) {
  // Pre: arbol, trackingID y salida no son NULL.
  // Pos: si devuelve true, *salida es una copia del paquete (no se expone el
  //      nodo). Si devuelve false, *salida no se toca. El arbol no se modifica.
  if (arbol == NULL || trackingID == NULL || salida == NULL)
    return false;
  const Nodo *nodo = buscarNodo(arbol->raiz, trackingID);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}

// Imprime el subarbol en inorden: izquierda, nodo, derecha = orden alfabetico.
static void enOrden(const Nodo *nodo) {
  // Pre: nodo es NULL o raiz de un ABB valido.
  // Pos: se imprimio una linea por paquete, ordenadas por trackingID.
  if (nodo != NULL) {
    enOrden(nodo->izquierda);
    printf("%s | %s | %.2f kg | $%.2f\n", nodo->dato.trackingID,
           nodo->dato.destino, nodo->dato.peso, nodo->dato.costoEnvio);
    enOrden(nodo->derecha);
  }
}

// Reporte de inventario en orden alfabetico (por stdout).
void recorrerEnOrden(const Abb *arbol) {
  // Pre: ninguna (si arbol es NULL no imprime nada).
  // Pos: imprime todos los paquetes ordenados. El arbol no se modifica.
  if (arbol != NULL) {
    enOrden(arbol->raiz);
  }
}

// ---- Modulo V: recursividad ----

// Altura = aristas hasta la hoja mas lejana.
static int altura(const Nodo *nodo) {
  // Pre: nodo es NULL o raiz de un subarbol valido.
  // Pos: devuelve la altura del subarbol; vacio = -1, hoja = 0.
  if (nodo == NULL)
    return -1;
  int alturaIzquierda = altura(nodo->izquierda);
  int alturaDerecha = altura(nodo->derecha);
  if (alturaIzquierda > alturaDerecha) {
    return 1 + alturaIzquierda;
  }
  return 1 + alturaDerecha;
}

// Altura del arbol completo.
int alturaABB(const Abb *arbol) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve la altura; arbol NULL o vacio = -1. No modifica el arbol.
  if (arbol == NULL)
    return -1;
  return altura(arbol->raiz);
}

// Profundidad de un ID = niveles desde la raiz del subarbol (esa raiz = 0).
static int profundidad(const Nodo *nodo, const char *trackingID) {
  // Pre: trackingID no es NULL. nodo es NULL o raiz de un ABB valido.
  // Pos: devuelve la profundidad, o -1 si el ID no esta. No modifica nada.
  if (nodo == NULL)
    return -1;
  int comparacion = strcmp(trackingID, nodo->dato.trackingID);
  if (comparacion == 0)
    return 0;
  int profundidadHijo = (comparacion < 0)
                            ? profundidad(nodo->izquierda, trackingID)
                            : profundidad(nodo->derecha, trackingID);
  // si abajo no estaba, propagamos el -1 en vez de sumarle
  return (profundidadHijo == -1) ? -1 : 1 + profundidadHijo;
}

// Profundidad de un paquete en el arbol.
int profundidadABB(const Abb *arbol, const char *trackingID) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve la profundidad (raiz = 0), o -1 si el arbol/ID es NULL o el
  //      paquete no existe. No modifica el arbol.
  if (arbol == NULL || trackingID == NULL)
    return -1;
  return profundidad(arbol->raiz, trackingID);
}

// Suma el peso de todos los nodos del subarbol.
static float pesoTotal(const Nodo *nodo) {
  // Pre: nodo es NULL o raiz de un subarbol valido.
  // Pos: devuelve la suma de pesos; subarbol vacio = 0.
  if (nodo == NULL)
    return 0;
  return nodo->dato.peso + pesoTotal(nodo->izquierda) +
         pesoTotal(nodo->derecha);
}

// Peso acumulado de todos los paquetes del arbol.
float pesoTotalABB(const Abb *arbol) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve la suma de pesos; arbol NULL o vacio = 0. No lo modifica.
  if (arbol == NULL)
    return 0;
  return pesoTotal(arbol->raiz);
}

// Condicion compleja: costo dentro de [min, max] y peso hasta el maximo
// (limites inclusivos).
static bool cumpleCondicion(const Paquete *paquete, float costoMinimo,
                            float costoMaximo, float pesoMaximo) {
  // Pre: paquete no es NULL.
  // Pos: true si el paquete cumple las tres condiciones a la vez.
  return paquete->costoEnvio >= costoMinimo &&
         paquete->costoEnvio <= costoMaximo && paquete->peso <= pesoMaximo;
}

// Busca en orden (izq, nodo, der) y devuelve el primero que cumpla.
static const Nodo *buscarCostoYPeso(const Nodo *nodo, float costoMinimo,
                                    float costoMaximo, float pesoMaximo) {
  // Pre: nodo es NULL o raiz de un ABB valido.
  // Pos: devuelve el primer nodo en orden alfabetico que cumple la condicion,
  //      o NULL si ninguno. No modifica nada.
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

// Primer paquete (por orden de trackingID) con costo en [costoMinimo,
// costoMaximo] y peso <= pesoMaximo.
bool buscarPorCostoYPeso(const Abb *arbol, float costoMinimo, float costoMaximo,
                         float pesoMaximo, Paquete *salida) {
  // Pre: arbol y salida no son NULL.
  // Pos: si devuelve true, *salida es copia de ese paquete. Si devuelve false
  //      (ninguno cumple o hubo NULL), *salida no se toca. El arbol no cambia.
  if (arbol == NULL || salida == NULL)
    return false;
  const Nodo *nodo =
      buscarCostoYPeso(arbol->raiz, costoMinimo, costoMaximo, pesoMaximo);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}
