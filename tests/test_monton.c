#include "monton.h"
#include <stdio.h>

void verificar(bool condicion, const char *nombre) {
  printf("%s %s\n", condicion ? "[OK]   " : "[FALLO]", nombre);
}

int main(void) {
  Monton *monton = crearMonton(2);
  Envio envioUno = {1, 5, 100};
  Envio envioDos = {2, 9, 200};
  Envio envioTres = {3, 7, 300};
  Envio envioCuatro = {4, 1, 400};
  Envio salida;

  verificar(montonVacio(monton), "monton nuevo esta vacio");
  verificar(!extraerMaximo(monton, &salida), "extraer de vacio devuelve false");
  verificar(!consultarMaxUrgencia(monton, &salida),
            "consultar vacio devuelve false");

  insertarEnvio(monton, &envioUno);
  insertarEnvio(monton, &envioDos);
  insertarEnvio(monton, &envioTres);
  insertarEnvio(monton, &envioCuatro);
  verificar(!montonVacio(monton), "despues de insertar no esta vacio");

  consultarMaxUrgencia(monton, &salida);
  verificar(salida.idEnvio == 2, "el mas urgente es el de nivel 9");

  extraerMaximo(monton, &salida);
  verificar(salida.idEnvio == 2, "extrae primero el nivel 9");
  extraerMaximo(monton, &salida);
  verificar(salida.idEnvio == 3, "luego el nivel 7");
  extraerMaximo(monton, &salida);
  verificar(salida.idEnvio == 1, "luego el nivel 5");
  extraerMaximo(monton, &salida);
  verificar(salida.idEnvio == 4, "luego el nivel 1");
  verificar(montonVacio(monton), "queda vacio");

  destruirMonton(monton);
  Monton *empates = crearMonton(2);
  Envio empateTarde = {5, 8, 500};
  Envio empateTemprano = {6, 8, 100};
  Envio empateMedio = {7, 8, 300};
  insertarEnvio(empates, &empateTarde);
  insertarEnvio(empates, &empateTemprano);
  insertarEnvio(empates, &empateMedio);
  extraerMaximo(empates, &salida);
  verificar(salida.idEnvio == 6, "empate de urgencia: sale el que vence antes");
  extraerMaximo(empates, &salida);
  verificar(salida.idEnvio == 7, "luego el de tiempo limite medio");
  extraerMaximo(empates, &salida);
  verificar(salida.idEnvio == 5, "al final el que vence mas tarde");
  destruirMonton(empates);
  return 0;
}
