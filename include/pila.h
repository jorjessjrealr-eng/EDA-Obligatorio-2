#ifndef PILA_H
#define PILA_H

#include "tipos.h"

typedef struct pila Pila;

Pila *crearPila(void);
void destruirPila(Pila *pila);
bool apilar(Pila *pila, const Operacion *operacion);
bool desapilar(Pila *pila, Operacion *salida);
bool consultarCima(const Pila *pila, Operacion *cima);
bool pilaVacia(const Pila *pila);

#endif
