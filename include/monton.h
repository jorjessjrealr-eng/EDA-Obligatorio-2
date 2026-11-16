#ifndef MONTON_H
#define MONTON_H

#include "tipos.h"

typedef struct monton Monton;

Monton *crearMonton(int capacidad);
void destruirMonton(Monton *monton);
bool insertarEnvio(Monton *monton, const Envio *envio);
bool consultarMaxUrgencia(const Monton *monton, Envio *salida);
bool extraerMaximo(Monton *monton, Envio *salida);
bool montonVacio(const Monton *monton);

#endif
