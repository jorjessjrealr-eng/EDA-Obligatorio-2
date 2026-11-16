// Heap binario de maxima prioridad sobre un arreglo dinamico.
// Insertar y extraer son O(log n), consultar el maximo es O(1).
//
// Indices: padre de i = (i-1)/2, hijos de i = 2i+1 y 2i+2.
//
// Formato de comentarios: arriba de cada funcion va que hace; debajo de la
// firma van su Pre (lo que se debe cumplir antes de llamarla) y su Pos
// (lo que queda cierto despues).
#include "monton.h"
#include <stdlib.h>

struct monton {
  Envio *datos;  // arreglo del heap
  int tamano;    // cuantos envios hay
  int capacidad; // cuantos entran antes de crecer
};

// Crea un heap vacio.
Monton *crearMonton(int capacidad) {
  // Pre: ninguna (si capacidad <= 0 se usa 10).
  // Pos: devuelve un heap vacio con esa capacidad inicial, o NULL si no hubo
  //      memoria (sin dejar nada reservado).
  if (capacidad <= 0) {
    capacidad = 10;
  }
  Monton *monton = malloc(sizeof(Monton));
  if (monton == NULL)
    return NULL;
  monton->datos = malloc(capacidad * sizeof(Envio));
  if (monton->datos == NULL) {
    free(monton); // no dejar la struct colgada si falla el arreglo
    return NULL;
  }
  monton->tamano = 0;
  monton->capacidad = capacidad;
  return monton;
}

// Libera el heap y su arreglo.
void destruirMonton(Monton *monton) {
  // Pre: monton es NULL o viene de crearMonton y no fue destruido antes.
  // Pos: toda la memoria queda liberada. El puntero ya no se puede usar.
  if (monton != NULL) {
    free(monton->datos);
    free(monton);
  }
}

// Dice si no hay envios. NULL cuenta como vacio.
bool montonVacio(const Monton *monton) {
  // Pre: ninguna (acepta NULL).
  // Pos: true si monton es NULL o tamano es 0. No modifica el heap.
  return monton == NULL || monton->tamano == 0;
}

// Intercambia el contenido de dos envios.
static void intercambiar(Envio *primero, Envio *segundo) {
  // Pre: primero y segundo no son NULL.
  // Pos: los valores quedan cruzados.
  Envio auxiliar = *primero;
  *primero = *segundo;
  *segundo = auxiliar;
}

// Criterio de orden: mayor urgencia gana. Si empatan, gana el que vence antes.
static bool tieneMasPrioridad(const Envio *primero, const Envio *segundo) {
  // Pre: primero y segundo no son NULL.
  // Pos: true si primero debe estar mas cerca de la raiz que segundo.
  if (primero->nivelUrgencia != segundo->nivelUrgencia)
    return primero->nivelUrgencia > segundo->nivelUrgencia;
  return primero->tiempoLimite < segundo->tiempoLimite;
}

// Sube el elemento mientras tenga mas prioridad que su padre.
static void subir(Monton *monton, int indice) {
  // Pre: monton no es NULL, datos[indice] ya esta cargado, y el heap cumple la
  //      propiedad salvo, quiza, entre datos[indice] y su padre.
  // Pos: el arreglo cumple la propiedad de heap.
  while (indice > 0 && tieneMasPrioridad(&monton->datos[indice],
                                         &monton->datos[(indice - 1) / 2])) {
    int padre = (indice - 1) / 2;
    intercambiar(&monton->datos[indice], &monton->datos[padre]);
    indice = padre;
  }
}

// Baja el elemento, cambiandolo con el hijo de mas prioridad, hasta que
// ningun hijo le gane.
static void bajar(Monton *monton, int indice) {
  // Pre: monton no es NULL y los subarboles de los hijos de indice ya son heaps.
  // Pos: el subarbol que arranca en indice cumple la propiedad de heap.
  bool seguir = true;
  while (seguir) {
    int izquierda = 2 * indice + 1;
    int derecha = 2 * indice + 2;
    int mayor = indice; // el que mas prioridad tiene entre los tres
    if (izquierda < monton->tamano &&
        tieneMasPrioridad(&monton->datos[izquierda], &monton->datos[mayor])) {
      mayor = izquierda;
    }
    if (derecha < monton->tamano &&
        tieneMasPrioridad(&monton->datos[derecha], &monton->datos[mayor])) {
      mayor = derecha;
    }
    if (mayor != indice) {
      intercambiar(&monton->datos[indice], &monton->datos[mayor]);
      indice = mayor;
    } else {
      seguir = false; // ya esta en su lugar
    }
  }
}

// Lo pone al final y lo sube. Si no hay lugar, duplica la capacidad.
bool insertarEnvio(Monton *monton, const Envio *envio) {
  // Pre: monton y envio no son NULL.
  // Pos: si devuelve true, el envio esta en el heap (copiado), tamano subio en 1
  //      y se mantiene la propiedad de heap. Si devuelve false (args NULL o sin
  //      memoria), el heap queda como estaba.
  if (monton == NULL || envio == NULL)
    return false;
  if (monton->tamano == monton->capacidad) {
    // realloc en una variable aparte: si falla, no perdemos el arreglo viejo
    Envio *nuevo =
        realloc(monton->datos, monton->capacidad * 2 * sizeof(Envio));
    if (nuevo == NULL)
      return false;
    monton->datos = nuevo;
    monton->capacidad *= 2;
  }
  monton->datos[monton->tamano] = *envio;
  subir(monton, monton->tamano);
  monton->tamano++;
  return true;
}

// Mira el envio mas urgente sin sacarlo. Esta siempre en la posicion 0.
bool consultarMaxUrgencia(const Monton *monton, Envio *salida) {
  // Pre: monton no vacio y salida no NULL.
  // Pos: si devuelve true, *salida es copia del maximo. El heap no se modifica.
  //      Si devuelve false, *salida no se toca.
  if (montonVacio(monton) || salida == NULL)
    return false;
  *salida = monton->datos[0];
  return true;
}

// Saca la raiz, mueve el ultimo a su lugar y lo baja.
bool extraerMaximo(Monton *monton, Envio *salida) {
  // Pre: monton no vacio. "salida" puede ser NULL si solo queremos descartar.
  // Pos: si devuelve true, el maximo se elimino (y se copio a *salida si no era
  //      NULL), tamano bajo en 1 y se mantiene la propiedad de heap.
  //      Si devuelve false (NULL o vacio), no cambia nada.
  if (montonVacio(monton))
    return false;
  if (salida != NULL) {
    *salida = monton->datos[0];
  }
  monton->tamano--;
  monton->datos[0] = monton->datos[monton->tamano];
  bajar(monton, 0);
  return true;
}
