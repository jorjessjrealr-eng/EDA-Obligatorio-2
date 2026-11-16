// Pila (LIFO) con nodos enlazados. Guarda el historial de operaciones
// para poder deshacer la ultima. Todo O(1).
//
// Formato de comentarios: arriba de cada funcion va que hace; debajo de la
// firma van su Pre (lo que se debe cumplir antes de llamarla) y su Pos
// (lo que queda cierto despues).
#include "pila.h"
#include <stdlib.h>

// Cada nodo guarda una operacion y apunta al que esta debajo.
typedef struct nodo {
  Operacion dato;
  struct nodo *siguiente;
} Nodo;

// Definicion oculta: afuera solo se ve "Pila *".
struct pila {
  Nodo *cima; // NULL = pila vacia
};

// Crea una pila vacia.
Pila *crearPila(void) {
  // Pre: ninguna.
  // Pos: devuelve una pila vacia lista para usar, o NULL si no hubo memoria.
  Pila *pila = malloc(sizeof(Pila));
  if (pila != NULL) {
    pila->cima = NULL;
  }
  return pila;
}

// Dice si la pila no tiene elementos. NULL cuenta como vacia, asi no hay
// que chequearlo afuera.
bool pilaVacia(const Pila *pila) {
  // Pre: ninguna (acepta NULL).
  // Pos: true si pila es NULL o no tiene elementos. No modifica la pila.
  return pila == NULL || pila->cima == NULL;
}

// Push: el nodo nuevo pasa a ser la cima.
bool apilar(Pila *pila, const Operacion *operacion) {
  // Pre: pila y operacion no son NULL.
  // Pos: si devuelve true, la cima es una copia de *operacion y lo que habia
  //      queda justo debajo. Si devuelve false (args NULL o sin memoria),
  //      la pila queda igual que antes.
  if (pila == NULL || operacion == NULL)
    return false;
  Nodo *nuevo = malloc(sizeof(Nodo));
  if (nuevo == NULL)
    return false;
  nuevo->dato = *operacion; // copia, no guarda el puntero del usuario
  nuevo->siguiente = pila->cima;
  pila->cima = nuevo;
  return true;
}

// Pop: saca la cima, la devuelve por "salida" y libera su nodo.
bool desapilar(Pila *pila, Operacion *salida) {
  // Pre: pila no vacia. "salida" puede ser NULL si solo queremos descartar.
  // Pos: si devuelve true, la cima anterior se elimino (y se copio a *salida
  //      si no era NULL). Si devuelve false (pila NULL o vacia), no cambia nada.
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

// Mira la cima sin sacarla.
bool consultarCima(const Pila *pila, Operacion *cima) {
  // Pre: pila no vacia y cima no NULL.
  // Pos: si devuelve true, *cima es una copia del tope. La pila no se modifica.
  //      Si devuelve false, *cima no se toca.
  if (pilaVacia(pila) || cima == NULL)
    return false;
  *cima = pila->cima->dato;
  return true;
}

// Vacia la pila nodo por nodo y despues libera la estructura (sin leaks).
void destruirPila(Pila *pila) {
  // Pre: pila es NULL o viene de crearPila y no fue destruida antes.
  // Pos: toda la memoria queda liberada. El puntero pasado ya no se puede usar.
  if (pila != NULL) {
    while (!pilaVacia(pila)) {
      desapilar(pila, NULL);
    }
    free(pila);
  }
}
