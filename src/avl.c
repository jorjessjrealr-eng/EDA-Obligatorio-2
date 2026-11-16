// AVL de estaciones, ordenado por idEstacion.
// Despues de cada insercion/eliminacion rebalancea con rotaciones, asi el
// factor de balance de todo nodo queda en [-1, 1] y la altura en O(log n).
//
// Formato de comentarios: arriba de cada funcion va que hace; debajo de la
// firma van su Pre (lo que se debe cumplir antes de llamarla) y su Pos
// (lo que queda cierto despues).
#include "avl.h"
#include <stdlib.h>

typedef struct nodo {
  Estacion dato;
  struct nodo *izquierda;
  struct nodo *derecha;
  int altura; // se guarda en el nodo para no recalcularla; hoja = 0
} Nodo;

struct avl {
  Nodo *raiz; // NULL = arbol vacio
};

// Crea un arbol vacio.
Avl *crearAVL(void) {
  // Pre: ninguna.
  // Pos: devuelve un arbol vacio, o NULL si no hubo memoria.
  Avl *arbol = malloc(sizeof(Avl));
  if (arbol != NULL) {
    arbol->raiz = NULL;
  }
  return arbol;
}

// Libera un subarbol en postorden.
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
void destruirAVL(Avl *arbol) {
  // Pre: arbol es NULL o viene de crearAVL y no fue destruido antes.
  // Pos: toda la memoria queda liberada. El puntero ya no se puede usar.
  if (arbol != NULL) {
    destruirNodos(arbol->raiz);
    free(arbol);
  }
}

// ---- Balanceo ----

// Altura guardada del nodo. NULL = -1, asi una hoja (0) cierra bien las cuentas.
static int alturaNodo(const Nodo *nodo) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve nodo->altura, o -1 si nodo es NULL. No modifica nada.
  if (nodo == NULL)
    return -1;
  return nodo->altura;
}

// Devuelve el mayor de dos enteros.
static int maximo(int primero, int segundo) {
  // Pre: ninguna.
  // Pos: devuelve el mayor de los dos enteros.
  if (primero > segundo)
    return primero;
  return segundo;
}

// Recalcula la altura de un nodo a partir de la de sus hijos.
static void actualizarAltura(Nodo *nodo) {
  // Pre: nodo no es NULL y las alturas de sus hijos son correctas.
  // Pos: nodo->altura = 1 + max(altura izquierda, altura derecha).
  nodo->altura =
      1 + maximo(alturaNodo(nodo->izquierda), alturaNodo(nodo->derecha));
}

// Positivo = pesa mas la izquierda, negativo = pesa mas la derecha.
static int factorBalance(const Nodo *nodo) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve altura(izq) - altura(der); NULL = 0. No modifica nada.
  if (nodo == NULL)
    return 0;
  return alturaNodo(nodo->izquierda) - alturaNodo(nodo->derecha);
}

// Rotacion simple a la derecha: el hijo izquierdo pasa a ser la raiz.
static Nodo *rotarDerecha(Nodo *raiz) {
  // Pre: raiz no es NULL y raiz->izquierda no es NULL.
  // Pos: devuelve la nueva raiz del subarbol. Se mantiene el orden del ABB y las
  //      alturas de los dos nodos involucrados quedan actualizadas.
  Nodo *nuevaRaiz = raiz->izquierda;
  raiz->izquierda = nuevaRaiz->derecha;
  nuevaRaiz->derecha = raiz;
  // orden importa: primero la que quedo abajo, despues la de arriba
  actualizarAltura(raiz);
  actualizarAltura(nuevaRaiz);
  return nuevaRaiz;
}

// Rotacion simple a la izquierda (espejo de rotarDerecha).
static Nodo *rotarIzquierda(Nodo *raiz) {
  // Pre: raiz no es NULL y raiz->derecha no es NULL.
  // Pos: devuelve la nueva raiz del subarbol. Se mantiene el orden del ABB y las
  //      alturas de los dos nodos involucrados quedan actualizadas.
  Nodo *nuevaRaiz = raiz->derecha;
  raiz->derecha = nuevaRaiz->izquierda;
  nuevaRaiz->izquierda = raiz;
  actualizarAltura(raiz);
  actualizarAltura(nuevaRaiz);
  return nuevaRaiz;
}

// Actualiza la altura y, si el balance se pasa de [-1, 1], rota:
//   izquierda-izquierda -> rotacion simple derecha
//   izquierda-derecha   -> doble (izquierda al hijo, derecha al nodo)
//   derecha-derecha     -> rotacion simple izquierda
//   derecha-izquierda   -> doble (derecha al hijo, izquierda al nodo)
static Nodo *balancear(Nodo *nodo) {
  // Pre: nodo no es NULL y sus dos subarboles ya son AVL validos (el
  //      desbalance del nodo es como mucho de 2).
  // Pos: devuelve la raiz del subarbol ya balanceada (puede ser otro nodo).
  //      El factor de balance de la raiz queda en [-1, 1] y el orden se mantiene.
  actualizarAltura(nodo);
  int balance = factorBalance(nodo);
  if (balance > 1) {
    if (factorBalance(nodo->izquierda) < 0) { // caso zigzag
      nodo->izquierda = rotarIzquierda(nodo->izquierda);
    }
    return rotarDerecha(nodo);
  }
  if (balance < -1) {
    if (factorBalance(nodo->derecha) > 0) { // caso zigzag
      nodo->derecha = rotarDerecha(nodo->derecha);
    }
    return rotarIzquierda(nodo);
  }
  return nodo;
}

// ---- Insercion ----

// Crea un nodo hoja con una copia de la estacion.
static Nodo *crearNodo(const Estacion *estacion) {
  // Pre: estacion no es NULL.
  // Pos: devuelve un nodo sin hijos, altura 0 y el dato copiado, o NULL si no
  //      hubo memoria.
  Nodo *nodo = malloc(sizeof(Nodo));
  if (nodo != NULL) {
    nodo->dato = *estacion;
    nodo->izquierda = NULL;
    nodo->derecha = NULL;
    nodo->altura = 0;
  }
  return nodo;
}

// Inserta como en un ABB y rebalancea al volver de la recursion.
static Nodo *insertarNodo(Nodo *nodo, const Estacion *estacion,
                          bool *yaInsertado) {
  // Pre: estacion y yaInsertado no son NULL. nodo es NULL o raiz de un AVL valido.
  // Pos: devuelve la raiz del subarbol, balanceada. *yaInsertado es true solo si
  //      se agrego un nodo; es false si el ID estaba repetido o fallo el malloc.
  if (nodo == NULL) {
    Nodo *nuevo = crearNodo(estacion);
    *yaInsertado = (nuevo != NULL);
    return nuevo;
  }
  if (estacion->idEstacion < nodo->dato.idEstacion) {
    nodo->izquierda = insertarNodo(nodo->izquierda, estacion, yaInsertado);
  } else if (estacion->idEstacion > nodo->dato.idEstacion) {
    nodo->derecha = insertarNodo(nodo->derecha, estacion, yaInsertado);
  } else {
    *yaInsertado = false; // duplicado
  }
  return balancear(nodo);
}

// Inserta una estacion en el arbol (se guarda una copia).
bool insertarEstacion(Avl *arbol, const Estacion *estacion) {
  // Pre: arbol y estacion no son NULL.
  // Pos: devuelve true si se agrego (una estacion mas, arbol sigue balanceado).
  //      Devuelve false si el ID ya existia, si hubo algun NULL o si fallo el
  //      malloc; en esos casos el arbol no cambia.
  if (arbol == NULL || estacion == NULL)
    return false;
  bool yaInsertado = false;
  arbol->raiz = insertarNodo(arbol->raiz, estacion, &yaInsertado);
  return yaInsertado;
}

// ---- Eliminacion ----

// El minimo de un subarbol es el nodo mas a la izquierda.
static Nodo *nodoMinimo(Nodo *nodo) {
  // Pre: nodo no es NULL.
  // Pos: devuelve el nodo con el menor idEstacion del subarbol. No modifica nada.
  Nodo *actual = nodo;
  while (actual->izquierda != NULL) {
    actual = actual->izquierda;
  }
  return actual;
}

// Elimina y rebalancea al volver. Tres casos al encontrar el nodo:
//   0 o 1 hijo -> se reemplaza por su hijo
//   2 hijos    -> se copia el sucesor inorden (minimo de la derecha) y se
//                 elimina ese sucesor de la derecha
static Nodo *eliminarNodo(Nodo *nodo, int idEstacion, bool *yaEliminado) {
  // Pre: yaEliminado no es NULL. nodo es NULL o raiz de un AVL valido.
  // Pos: devuelve la raiz del subarbol sin ese ID, balanceada (NULL si quedo
  //      vacio). Si el ID existia, se libero su nodo y *yaEliminado es true;
  //      si no, el subarbol queda igual y *yaEliminado no se modifica (el
  //      llamador lo inicializa en false).
  if (nodo == NULL)
    return NULL;
  if (idEstacion < nodo->dato.idEstacion) {
    nodo->izquierda = eliminarNodo(nodo->izquierda, idEstacion, yaEliminado);
  } else if (idEstacion > nodo->dato.idEstacion) {
    nodo->derecha = eliminarNodo(nodo->derecha, idEstacion, yaEliminado);
  } else {
    *yaEliminado = true;
    if (nodo->izquierda == NULL || nodo->derecha == NULL) {
      Nodo *hijo = nodo->izquierda;
      if (hijo == NULL) {
        hijo = nodo->derecha;
      }
      free(nodo);
      return hijo; // puede ser NULL si era hoja
    }
    Nodo *sucesor = nodoMinimo(nodo->derecha);
    nodo->dato = sucesor->dato;
    bool sucesorEliminado = false; // flag descartable, ya sabemos que existe
    nodo->derecha =
        eliminarNodo(nodo->derecha, nodo->dato.idEstacion, &sucesorEliminado);
  }
  return balancear(nodo);
}

// Elimina una estacion por ID.
bool eliminarEstacion(Avl *arbol, int idEstacion) {
  // Pre: arbol no es NULL.
  // Pos: devuelve true si existia: se elimino, se libero su memoria y el arbol
  //      sigue balanceado. Devuelve false si no existia o arbol es NULL; en ese
  //      caso no cambia nada.
  if (arbol == NULL)
    return false;
  bool yaEliminado = false;
  arbol->raiz = eliminarNodo(arbol->raiz, idEstacion, &yaEliminado);
  return yaEliminado;
}

// ---- Busqueda ----

// Busca un ID en el subarbol.
static const Nodo *buscarNodo(const Nodo *nodo, int idEstacion) {
  // Pre: nodo es NULL o raiz de un AVL valido.
  // Pos: devuelve el nodo con ese ID, o NULL si no existe. No modifica nada.
  if (nodo == NULL)
    return NULL;
  if (idEstacion == nodo->dato.idEstacion)
    return nodo;
  if (idEstacion < nodo->dato.idEstacion)
    return buscarNodo(nodo->izquierda, idEstacion);
  return buscarNodo(nodo->derecha, idEstacion);
}

// Busca una estacion por ID.
bool buscarEstacion(const Avl *arbol, int idEstacion, Estacion *salida) {
  // Pre: arbol y salida no son NULL.
  // Pos: si devuelve true, *salida es una copia de la estacion. Si devuelve
  //      false, *salida no se toca. El arbol no se modifica.
  if (arbol == NULL || salida == NULL)
    return false;
  const Nodo *nodo = buscarNodo(arbol->raiz, idEstacion);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}

// ---- Modulo V: recursividad sobre subredes ----
// Una subred = el nodo con idRaiz y todo lo que cuelga de el.

// Altura del arbol completo, usando la altura guardada (O(1)).
int alturaAVL(const Avl *arbol) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve la altura; arbol NULL o vacio = -1. No modifica el arbol.
  if (arbol == NULL)
    return -1;
  return alturaNodo(arbol->raiz);
}

// Altura calculada de forma recursiva (no usa el campo guardado).
static int alturaRecursiva(const Nodo *nodo) {
  // Pre: nodo es NULL o raiz de un subarbol valido.
  // Pos: devuelve la altura del subarbol; vacio = -1, hoja = 0.
  if (nodo == NULL)
    return -1;
  return 1 + maximo(alturaRecursiva(nodo->izquierda),
                    alturaRecursiva(nodo->derecha));
}

// Altura de la subred que arranca en una estacion.
int alturaSubredAVL(const Avl *arbol, int idRaiz) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve la altura de la subred (hoja = 0). Si arbol es NULL o idRaiz
  //      no existe devuelve -1. No modifica el arbol.
  if (arbol == NULL)
    return -1;
  return alturaRecursiva(buscarNodo(arbol->raiz, idRaiz));
}

// Costo acumulado de transporte del nodo y todos sus descendientes.
static float costoTotal(const Nodo *nodo) {
  // Pre: nodo es NULL o raiz de un subarbol valido.
  // Pos: devuelve la suma de costoTransporte; subarbol vacio = 0.
  if (nodo == NULL)
    return 0;
  return nodo->dato.costoTransporte + costoTotal(nodo->izquierda) +
         costoTotal(nodo->derecha);
}

// Costo acumulado de transporte de la subred que arranca en una estacion.
float costoTotalSubredAVL(const Avl *arbol, int idRaiz) {
  // Pre: ninguna (acepta NULL).
  // Pos: devuelve la suma de costos de la subred. Si arbol es NULL o idRaiz no
  //      existe devuelve 0. No modifica el arbol.
  if (arbol == NULL)
    return 0;
  return costoTotal(buscarNodo(arbol->raiz, idRaiz));
}

// Condicion compleja: primera estacion (en orden) cuyo costo propio sea >= min
// y cuya subred completa no pase del maximo.
static const Nodo *buscarCostos(const Nodo *nodo, float costoMinimo,
                                float costoMaximoSubred) {
  // Pre: nodo es NULL o raiz de un AVL valido.
  // Pos: devuelve la primera estacion (por orden de ID) que cumple ambas
  //      condiciones, o NULL si ninguna. No modifica nada.
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

// Primera estacion (por orden de ID) con costo propio >= costoMinimo y costo
// total de su subred <= costoMaximoSubred.
bool buscarPorCostos(const Avl *arbol, float costoMinimo,
                     float costoMaximoSubred, Estacion *salida) {
  // Pre: arbol y salida no son NULL.
  // Pos: si devuelve true, *salida es copia de esa estacion. Si devuelve false
  //      (ninguna cumple o hubo NULL), *salida no se toca. El arbol no cambia.
  if (arbol == NULL || salida == NULL)
    return false;
  const Nodo *nodo = buscarCostos(arbol->raiz, costoMinimo, costoMaximoSubred);
  if (nodo == NULL)
    return false;
  *salida = nodo->dato;
  return true;
}
