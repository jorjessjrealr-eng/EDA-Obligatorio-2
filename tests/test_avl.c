#include "avl.h"
#include <stdio.h>

void verificar(bool condicion, const char *nombre) {
  printf("%s %s\n", condicion ? "[OK]   " : "[FALLO]", nombre);
}

static bool insertarPorId(Avl *arbol, int idEstacion) {
  Estacion estacion = {idEstacion, "Hub", 10.0f};
  return insertarEstacion(arbol, &estacion);
}

static bool insertarConCosto(Avl *arbol, int idEstacion, float costo) {
  Estacion estacion = {idEstacion, "Hub", costo};
  return insertarEstacion(arbol, &estacion);
}

static int alturaTrasInsertar(int primero, int segundo, int tercero) {
  Avl *arbol = crearAVL();
  insertarPorId(arbol, primero);
  insertarPorId(arbol, segundo);
  insertarPorId(arbol, tercero);
  int altura = alturaAVL(arbol);
  destruirAVL(arbol);
  return altura;
}

int main(void) {
  Avl *arbol = crearAVL();
  Estacion salida;

  verificar(alturaAVL(arbol) == -1, "arbol vacio tiene altura -1");
  verificar(!buscarEstacion(arbol, 1, &salida),
            "buscar en vacio devuelve false");
  verificar(!eliminarEstacion(arbol, 1), "eliminar en vacio devuelve false");

  verificar(alturaTrasInsertar(30, 20, 10) == 1, "rotacion simple derecha");
  verificar(alturaTrasInsertar(10, 20, 30) == 1, "rotacion simple izquierda");
  verificar(alturaTrasInsertar(30, 10, 20) == 1,
            "rotacion doble izquierda-derecha");
  verificar(alturaTrasInsertar(10, 30, 20) == 1,
            "rotacion doble derecha-izquierda");

  for (int id = 1; id <= 7; id++) {
    insertarPorId(arbol, id);
  }
  verificar(alturaAVL(arbol) == 2, "7 estaciones ascendentes: altura 2");
  verificar(!insertarPorId(arbol, 4), "duplicado devuelve false");
  verificar(buscarEstacion(arbol, 6, &salida) && salida.idEstacion == 6,
            "encuentra la estacion 6");
  verificar(!buscarEstacion(arbol, 99, &salida), "no encuentra la 99");

  verificar(eliminarEstacion(arbol, 4), "eliminar la raiz (dos hijos)");
  verificar(!buscarEstacion(arbol, 4, &salida), "la 4 ya no esta");
  verificar(!eliminarEstacion(arbol, 4), "eliminar inexistente devuelve false");

  eliminarEstacion(arbol, 1);
  eliminarEstacion(arbol, 3);
  eliminarEstacion(arbol, 2);
  verificar(alturaAVL(arbol) == 1, "eliminar rebalancea: queda altura 1");
  verificar(buscarEstacion(arbol, 5, &salida) &&
                buscarEstacion(arbol, 6, &salida) &&
                buscarEstacion(arbol, 7, &salida),
            "quedan 5, 6 y 7");

  destruirAVL(arbol);

  arbol = crearAVL();
  for (int id = 1; id <= 100; id++) {
    insertarPorId(arbol, id);
  }
  verificar(alturaAVL(arbol) <= 8, "100 ascendentes siguen balanceadas");
  printf("altura con 100 estaciones: %d\n", alturaAVL(arbol));
  destruirAVL(arbol);
  arbol = crearAVL();
  int orden[] = {4, 2, 6, 1, 3, 5, 7};
  for (int indice = 0; indice < 7; indice++) {
    insertarConCosto(arbol, orden[indice], orden[indice] * 10.0f);
  }
  verificar(alturaSubredAVL(arbol, 4) == 2, "altura de la subred de 4 es 2");
  verificar(alturaSubredAVL(arbol, 2) == 1, "altura de la subred de 2 es 1");
  verificar(alturaSubredAVL(arbol, 1) == 0, "altura de una hoja es 0");
  verificar(alturaSubredAVL(arbol, 99) == -1, "subred inexistente: -1");
  verificar(costoTotalSubredAVL(arbol, 4) == 280.0f, "costo de la subred de 4");
  verificar(costoTotalSubredAVL(arbol, 6) == 180.0f, "costo de la subred de 6");
  verificar(costoTotalSubredAVL(arbol, 1) == 10.0f, "costo de una hoja");
  verificar(costoTotalSubredAVL(arbol, 99) == 0.0f, "subred inexistente: 0");
  verificar(buscarPorCostos(arbol, 20.0f, 60.0f, &salida) &&
                salida.idEstacion == 2,
            "costo >= 20 y subred <= 60: estacion 2");
  verificar(buscarPorCostos(arbol, 60.0f, 100.0f, &salida) &&
                salida.idEstacion == 7,
            "la 6 se descarta por subred cara: estacion 7");
  verificar(!buscarPorCostos(arbol, 100.0f, 500.0f, &salida),
            "ninguna estacion con costo >= 100");
  destruirAVL(arbol);
  return 0;
}
