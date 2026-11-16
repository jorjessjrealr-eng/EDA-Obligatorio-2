#include "monton.h"
#include <stdlib.h>

struct monton {
  Envio *datos;
  int tamano;
  int capacidad;
};

Monton *crearMonton(int capacidad) {
  if (capacidad <= 0) {
    capacidad = 10;
  }
  Monton *monton = malloc(sizeof(Monton));
  if (monton == NULL)
    return NULL;
  monton->datos = malloc(capacidad * sizeof(Envio));
  if (monton->datos == NULL) {
    free(monton);
    return NULL;
  }
  monton->tamano = 0;
  monton->capacidad = capacidad;
  return monton;
}

void destruirMonton(Monton *monton) {
  if (monton != NULL) {
    free(monton->datos);
    free(monton);
  }
}

bool montonVacio(const Monton *monton) {
  return monton == NULL || monton->tamano == 0;
}

static void intercambiar(Envio *primero, Envio *segundo) {
  Envio auxiliar = *primero;
  *primero = *segundo;
  *segundo = auxiliar;
}

static bool tieneMasPrioridad(const Envio *primero, const Envio *segundo) {
  if (primero->nivelUrgencia != segundo->nivelUrgencia)
    return primero->nivelUrgencia > segundo->nivelUrgencia;
  return primero->tiempoLimite < segundo->tiempoLimite;
}

static void subir(Monton *monton, int indice) {
  while (indice > 0 && tieneMasPrioridad(&monton->datos[indice],
                                         &monton->datos[(indice - 1) / 2])) {
    int padre = (indice - 1) / 2;
    intercambiar(&monton->datos[indice], &monton->datos[padre]);
    indice = padre;
  }
}

static void bajar(Monton *monton, int indice) {
  bool seguir = true;
  while (seguir) {
    int izquierda = 2 * indice + 1;
    int derecha = 2 * indice + 2;
    int mayor = indice;
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
      seguir = false;
    }
  }
}

bool insertarEnvio(Monton *monton, const Envio *envio) {
  if (monton == NULL || envio == NULL)
    return false;
  if (monton->tamano == monton->capacidad) {
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

bool consultarMaxUrgencia(const Monton *monton, Envio *salida) {
  if (montonVacio(monton) || salida == NULL)
    return false;
  *salida = monton->datos[0];
  return true;
}

bool extraerMaximo(Monton *monton, Envio *salida) {
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
