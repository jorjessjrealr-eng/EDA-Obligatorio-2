#ifndef TIPOS_H
#define TIPOS_H

#include <stdbool.h>

#define MAX_RUTA 200
#define MAX_CADENA 200

typedef struct {
  char ruta[MAX_RUTA];
  char operacionRealizada[MAX_CADENA];
} Operacion;

typedef struct {
  int idEnvio;
  int nivelUrgencia;
  int tiempoLimite;
} Envio;

typedef struct {
  char trackingID[MAX_CADENA];
  char destino[MAX_CADENA];
  float peso;
  float costoEnvio;
} Paquete;

typedef struct {
  int idEstacion;
  char nombre[MAX_CADENA];
  float costoTransporte;
} Estacion;

#endif
