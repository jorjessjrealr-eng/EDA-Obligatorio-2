#ifndef ABB_H
#define ABB_H

#include "tipos.h"

typedef struct abb Abb;

Abb *crearABB(void);
void destruirABB(Abb *arbol);
bool insertarPaquete(Abb *arbol, const Paquete *paquete);
bool buscarPorID(const Abb *arbol, const char *trackingID, Paquete *salida);
void recorrerEnOrden(const Abb *arbol);

int alturaABB(const Abb *arbol);
int profundidadABB(const Abb *arbol, const char *trackingID);
float pesoTotalABB(const Abb *arbol);
bool buscarPorCostoYPeso(const Abb *arbol, float costoMinimo, float costoMaximo,
                         float pesoMaximo, Paquete *salida);

#endif
