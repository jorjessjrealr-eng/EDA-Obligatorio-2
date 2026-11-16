#include "abb.h"
#include <stdio.h>
#include <string.h>

void verificar(bool condicion, const char *nombre) {
  printf("%s %s\n", condicion ? "[OK]   " : "[FALLO]", nombre);
}

int main(void) {
  Abb *arbol = plantarABB();
  Paquete paqueteM = {"M200", "Salto", 2.0f, 50.0f};
  Paquete paqueteC = {"C100", "Paysandu", 3.0f, 80.0f};
  Paquete paqueteT = {"T300", "Rivera", 5.0f, 120.0f};
  Paquete paqueteA = {"A050", "Colonia", 1.0f, 30.0f};
  Paquete salida;

  verificar(alturaABB(arbol) == -1, "arbol vacio tiene altura -1");
  verificar(!buscarPorID(arbol, "M200", &salida),
            "buscar en vacio devuelve false");

  verificar(injertarPaquete(arbol, &paqueteM), "injertar M200");
  injertarPaquete(arbol, &paqueteC);
  injertarPaquete(arbol, &paqueteT);
  injertarPaquete(arbol, &paqueteA);
  verificar(!injertarPaquete(arbol, &paqueteM), "duplicado devuelve false");

  verificar(buscarPorID(arbol, "T300", &salida) &&
                strcmp(salida.destino, "Rivera") == 0,
            "encuentra T300");
  verificar(!buscarPorID(arbol, "Z999", &salida),
            "no encuentra ID inexistente");

  verificar(alturaABB(arbol) == 2, "altura es 2");
  verificar(profundidadABB(arbol, "M200") == 0, "la raiz tiene profundidad 0");
  verificar(profundidadABB(arbol, "C100") == 1, "C100 tiene profundidad 1");
  verificar(profundidadABB(arbol, "T300") == 1, "T300 tiene profundidad 1");
  verificar(profundidadABB(arbol, "A050") == 2, "A050 tiene profundidad 2");
  verificar(profundidadABB(arbol, "Z999") == -1,
            "ID inexistente devuelve profundidad -1");

  verificar(pesoTotalABB(arbol) == 11.0f, "peso total es 11");

  verificar(buscarPorCostoYPeso(arbol, 40.0f, 150.0f, 2.0f, &salida) &&
                strcmp(salida.trackingID, "M200") == 0,
            "costo 40-150 y peso <= 2: C100 se descarta por pesado, sale M200");
  verificar(!buscarPorCostoYPeso(arbol, 100.0f, 150.0f, 3.0f, &salida),
            "T300 cumple el costo pero pesa 5: no hay resultado");
  verificar(buscarPorCostoYPeso(arbol, 100.0f, 150.0f, 5.0f, &salida) &&
                strcmp(salida.trackingID, "T300") == 0,
            "costo 100-150 y peso <= 5: T300");
  verificar(buscarPorCostoYPeso(arbol, 30.0f, 30.0f, 1.0f, &salida) &&
                strcmp(salida.trackingID, "A050") == 0,
            "los limites son inclusivos: A050");
  verificar(!buscarPorCostoYPeso(arbol, 0.0f, 20.0f, 10.0f, &salida),
            "ningun paquete con costo <= 20");

  printf("\nInventario en orden:\n");
  recorrerEnOrden(arbol);

  talarABB(arbol);
  return 0;
}
