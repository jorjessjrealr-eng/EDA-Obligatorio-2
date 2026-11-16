#ifndef AVL_H
#define AVL_H

#include "tipos.h"

typedef struct avl Avl;

Avl *crearAVL(void);
void destruirAVL(Avl *arbol);
bool insertarEstacion(Avl *arbol, const Estacion *estacion);
bool eliminarEstacion(Avl *arbol, int idEstacion);
bool buscarEstacion(const Avl *arbol, int idEstacion, Estacion *salida);
int alturaAVL(const Avl *arbol);
int alturaSubredAVL(const Avl *arbol, int idRaiz);
float costoTotalSubredAVL(const Avl *arbol, int idRaiz);
bool buscarPorCostos(const Avl *arbol, float costoMinimo,
                     float costoMaximoSubred, Estacion *salida);

#endif
