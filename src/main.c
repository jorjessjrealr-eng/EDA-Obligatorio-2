// Menu interactivo (ncurses) que integra los 4 TADs de LogiExpress 360:
//   [ABB] paquetes   [HEAP] urgencias   [AVL] estaciones   [PILA] historial
//   [REC] auditoria (funciones recursivas del Modulo V)
//
// Formato de comentarios: arriba de cada funcion va que hace; debajo de la
// firma van su Pre (lo que se debe cumplir antes de llamarla) y su Pos
// (lo que queda cierto despues).
//
// Controles: flechas (o j/k) para mover, Enter para elegir, q para salir.
#include <ctype.h>
#include <limits.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abb.h"
#include "avl.h"
#include "monton.h"
#include "pila.h"

#define LARGO_CAMPO 28   // caracteres maximos (y ancho visible) de un campo
#define COL_CAMPO 17     // columna donde empieza un campo dentro de la ventana
#define ANCHO_FORM 50    // ancho de las ventanas con formulario
#define ANCHO_MENU 42    // ancho del panel del menu
#define ALTO_PANEL 16    // alto de los paneles (menu y estado)
#define FILAS_MIN 22     // tamano minimo de terminal
#define COLUMNAS_MIN 80

// Pares de colores.
enum { PAR_TITULO = 1, PAR_SELECCION, PAR_OK, PAR_ERROR, PAR_ETIQUETA };

// Tipo de mensaje que se muestra sobre la barra de ayuda.
typedef enum { MSG_NINGUNO, MSG_OK, MSG_ERROR } TipoMensaje;

typedef struct {
  char texto[96];
  TipoMensaje tipo;
} Mensaje;

// Todo el estado del sistema. Los contadores se llevan aca porque los TADs
// no exponen su tamano.
typedef struct {
  Abb *paquetes;
  Monton *urgencias;
  Avl *estaciones;
  Pila *historial;
  int totalPaquetes;
  int totalUrgencias;
  int totalEstaciones;
  int totalOperaciones;
} Sistema;

// ---------------------------------------------------------------------------
// Utilidades
// ---------------------------------------------------------------------------

// Guarda un mensaje (con formato printf) para mostrarlo en pantalla.
static void fijarMensaje(Mensaje *mensaje, TipoMensaje tipo,
                         const char *formato, ...) {
  // Pre: mensaje y formato no son NULL.
  // Pos: mensaje queda con el tipo y el texto formateado (truncado si no entra).
  va_list argumentos;
  va_start(argumentos, formato);
  vsnprintf(mensaje->texto, sizeof(mensaje->texto), formato, argumentos);
  va_end(argumentos);
  mensaje->tipo = tipo;
}

// Saca los espacios del principio y del final de un texto.
static void recortar(char *texto) {
  // Pre: texto no es NULL y termina en '\0'.
  // Pos: texto queda sin espacios en los extremos.
  size_t inicio = 0;
  while (texto[inicio] != '\0' && isspace((unsigned char)texto[inicio])) {
    inicio++;
  }
  size_t fin = strlen(texto);
  while (fin > inicio && isspace((unsigned char)texto[fin - 1])) {
    fin--;
  }
  memmove(texto, texto + inicio, fin - inicio);
  texto[fin - inicio] = '\0';
}

// ---------------------------------------------------------------------------
// Ventanas y formularios
// ---------------------------------------------------------------------------

// Abre una ventana con borde y titulo, centrada en la pantalla.
static WINDOW *abrirVentana(const char *titulo, int alto, int ancho) {
  // Pre: ncurses iniciado y la ventana entra en la pantalla.
  // Pos: devuelve la ventana dibujada (hay que cerrarla con delwin).
  WINDOW *ventana = newwin(alto, ancho, (LINES - alto) / 2, (COLS - ancho) / 2);
  keypad(ventana, TRUE);
  box(ventana, 0, 0);
  wattron(ventana, COLOR_PAIR(PAR_TITULO) | A_BOLD);
  mvwprintw(ventana, 0, 2, " %s ", titulo);
  wattroff(ventana, COLOR_PAIR(PAR_TITULO) | A_BOLD);
  return ventana;
}

// Dibuja las etiquetas y los campos vacios de un formulario, y la pista para
// cancelar en la ultima fila de la ventana.
static void dibujarCampos(WINDOW *ventana, int cantidad,
                          const char *const etiquetas[]) {
  // Pre: ventana abierta con al menos cantidad + 5 filas, y etiquetas con
  //      cantidad textos de hasta 14 caracteres.
  // Pos: los campos quedan en las filas 2 en adelante (pendiente de refrescar).
  for (int i = 0; i < cantidad; i++) {
    mvwprintw(ventana, 2 + i, 2, "%-14s", etiquetas[i]);
    mvwhline(ventana, 2 + i, COL_CAMPO, '_', LARGO_CAMPO);
  }
  wattron(ventana, A_DIM);
  mvwprintw(ventana, getmaxy(ventana) - 2, 2, "Enter con un campo vacio = cancelar");
  wattroff(ventana, A_DIM);
}

// Lee un campo de texto. Un campo vacio cuenta como cancelar.
static bool leerTexto(WINDOW *ventana, int fila, char *destino) {
  // Pre: ventana abierta con el campo ya dibujado (dibujarCampos) y destino
  //      con lugar para LARGO_CAMPO + 1 caracteres.
  // Pos: devuelve true si se cargo un texto no vacio (ya recortado) en destino.
  //      Devuelve false si quedo vacio o hubo error de lectura.
  wmove(ventana, fila, COL_CAMPO);
  curs_set(1);
  echo();
  int estado = wgetnstr(ventana, destino, LARGO_CAMPO);
  noecho();
  curs_set(0);
  if (estado == ERR) {
    destino[0] = '\0';
    return false;
  }
  recortar(destino);
  return destino[0] != '\0';
}

// Lee un entero.
static bool leerEntero(WINDOW *ventana, int fila, int *valor) {
  // Pre: ventana abierta y valor no es NULL.
  // Pos: devuelve true y deja el numero en *valor si el texto es un entero
  //      valido. Si no, devuelve false y *valor no se toca.
  char texto[LARGO_CAMPO + 1];
  if (!leerTexto(ventana, fila, texto))
    return false;
  char *fin;
  long numero = strtol(texto, &fin, 10);
  if (*fin != '\0' || numero < INT_MIN || numero > INT_MAX)
    return false;
  *valor = (int)numero;
  return true;
}

// Lee un real no negativo (acepta coma o punto decimal).
static bool leerReal(WINDOW *ventana, int fila, float *valor) {
  // Pre: ventana abierta y valor no es NULL.
  // Pos: devuelve true y deja el numero en *valor si el texto es un real entre
  //      0 y 1e9. Si no, devuelve false y *valor no se toca.
  char texto[LARGO_CAMPO + 1];
  if (!leerTexto(ventana, fila, texto))
    return false;
  for (char *c = texto; *c != '\0'; c++) {
    if (*c == ',')
      *c = '.';
  }
  char *fin;
  double numero = strtod(texto, &fin);
  if (*fin != '\0' || !(numero >= 0) || numero > 1e9)
    return false;
  *valor = (float)numero;
  return true;
}

// Muestra un cuadro con un resultado (varias lineas separadas por '\n') y
// espera una tecla.
static void mostrarResultado(const char *titulo, const char *formato, ...) {
  // Pre: titulo y formato no son NULL. El texto formateado es menor a 512
  //      caracteres y tiene pocas lineas (entra en pantalla).
  // Pos: el cuadro se mostro y se cerro al apretar una tecla.
  char texto[512];
  va_list argumentos;
  va_start(argumentos, formato);
  vsnprintf(texto, sizeof(texto), formato, argumentos);
  va_end(argumentos);

  int lineas = 1;
  int ancho = (int)strlen(titulo) + 6;
  int actual = 0;
  for (const char *c = texto; *c != '\0'; c++) {
    if (*c == '\n') {
      lineas++;
      actual = 0;
    } else {
      actual++;
      if (actual + 6 > ancho)
        ancho = actual + 6;
    }
  }
  if (ancho < 34)
    ancho = 34;

  WINDOW *ventana = abrirVentana(titulo, lineas + 5, ancho);
  int fila = 2;
  const char *inicio = texto;
  for (;;) {
    const char *salto = strchr(inicio, '\n');
    int largo = (salto != NULL) ? (int)(salto - inicio) : (int)strlen(inicio);
    mvwprintw(ventana, fila, 3, "%.*s", largo, inicio);
    fila++;
    if (salto == NULL)
      break;
    inicio = salto + 1;
  }
  wattron(ventana, A_DIM);
  mvwprintw(ventana, lineas + 3, 3, "Presione una tecla");
  wattroff(ventana, A_DIM);
  wgetch(ventana);
  delwin(ventana);
}

// ---------------------------------------------------------------------------
// Acciones del menu
// ---------------------------------------------------------------------------

// [ABB] Registra un paquete nuevo.
static void accionRegistrarPaquete(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si los datos son validos y el ID no existia, el paquete queda en el
  //      ABB y totalPaquetes sube en 1. En cualquier caso mensaje dice que paso.
  Paquete paquete;
  WINDOW *ventana = abrirVentana("Registrar paquete", 9, ANCHO_FORM);
  dibujarCampos(ventana, 4, (const char *const[]){"Tracking ID", "Destino",
                                                  "Peso (kg)", "Costo envio"});
  bool completo = leerTexto(ventana, 2, paquete.trackingID) &&
                  leerTexto(ventana, 3, paquete.destino) &&
                  leerReal(ventana, 4, &paquete.peso) &&
                  leerReal(ventana, 5, &paquete.costoEnvio);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado o dato invalido");
  } else if (insertarPaquete(sistema->paquetes, &paquete)) {
    sistema->totalPaquetes++;
    fijarMensaje(mensaje, MSG_OK, "Paquete %s registrado", paquete.trackingID);
  } else {
    fijarMensaje(mensaje, MSG_ERROR, "Ya existe el paquete %s",
                 paquete.trackingID);
  }
}

// [ABB] Busca un paquete por tracking ID y muestra sus datos.
static void accionBuscarPaquete(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: muestra el paquete (con su profundidad en el arbol) si existe.
  //      Si no, mensaje lo avisa. No modifica los datos.
  char trackingID[LARGO_CAMPO + 1];
  WINDOW *ventana = abrirVentana("Buscar paquete", 6, ANCHO_FORM);
  dibujarCampos(ventana, 1, (const char *const[]){"Tracking ID"});
  bool completo = leerTexto(ventana, 2, trackingID);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado");
    return;
  }
  Paquete paquete;
  if (!buscarPorID(sistema->paquetes, trackingID, &paquete)) {
    fijarMensaje(mensaje, MSG_ERROR, "No existe el paquete %s", trackingID);
    return;
  }
  mostrarResultado("Paquete encontrado",
                   "Tracking ID : %s\nDestino     : %s\nPeso        : %.2f kg\n"
                   "Costo envio : $%.2f\nProfundidad : nivel %d del arbol",
                   paquete.trackingID, paquete.destino, paquete.peso,
                   paquete.costoEnvio,
                   profundidadABB(sistema->paquetes, trackingID));
  fijarMensaje(mensaje, MSG_NINGUNO, "");
}

// [ABB] Muestra el inventario en orden alfabetico.
static void accionVerInventario(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si hay paquetes, se listaron por la terminal normal (recorrerEnOrden
  //      imprime con printf, asi que se sale de ncurses un momento) y se volvio
  //      al menu. Si no hay, mensaje lo avisa.
  if (sistema->totalPaquetes == 0) {
    fijarMensaje(mensaje, MSG_ERROR, "No hay paquetes registrados");
    return;
  }
  def_prog_mode();
  endwin();
  printf("\nINVENTARIO (orden alfabetico por tracking ID)\n\n");
  recorrerEnOrden(sistema->paquetes);
  printf("\nPresione Enter para volver...");
  fflush(stdout);
  int caracter;
  while ((caracter = getchar()) != '\n' && caracter != EOF) {
  }
  reset_prog_mode();
  refresh();
  fijarMensaje(mensaje, MSG_NINGUNO, "");
}

// [HEAP] Pone un envio urgente en la cola de prioridad.
static void accionRegistrarEnvio(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si los datos son validos, el envio queda en el heap y totalUrgencias
  //      sube en 1. mensaje dice que paso.
  Envio envio;
  WINDOW *ventana = abrirVentana("Registrar envio urgente", 9, ANCHO_FORM);
  dibujarCampos(ventana, 3, (const char *const[]){"ID envio", "Nivel urgencia",
                                                  "Tiempo limite"});
  wattron(ventana, A_DIM);
  mvwprintw(ventana, 5, 2, "Sale primero la mayor urgencia; si empatan,");
  mvwprintw(ventana, 6, 2, "el que tiene menor tiempo limite.");
  wattroff(ventana, A_DIM);
  bool completo = leerEntero(ventana, 2, &envio.idEnvio) &&
                  leerEntero(ventana, 3, &envio.nivelUrgencia) &&
                  leerEntero(ventana, 4, &envio.tiempoLimite);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado o dato invalido");
  } else if (insertarEnvio(sistema->urgencias, &envio)) {
    sistema->totalUrgencias++;
    fijarMensaje(mensaje, MSG_OK, "Envio %d en cola (urgencia %d)",
                 envio.idEnvio, envio.nivelUrgencia);
  } else {
    fijarMensaje(mensaje, MSG_ERROR, "No se pudo encolar el envio");
  }
}

// [HEAP] Despacha el envio mas urgente.
static void accionDespacharSiguiente(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si habia envios, el mas urgente sale del heap y totalUrgencias baja
  //      en 1. Si no, mensaje lo avisa.
  Envio envio;
  if (!extraerMaximo(sistema->urgencias, &envio)) {
    fijarMensaje(mensaje, MSG_ERROR, "No hay envios en cola");
    return;
  }
  sistema->totalUrgencias--;
  fijarMensaje(mensaje, MSG_OK, "Despachado envio %d (urgencia %d, limite %d)",
               envio.idEnvio, envio.nivelUrgencia, envio.tiempoLimite);
}

// [AVL] Registra una estacion de distribucion.
static void accionRegistrarEstacion(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si los datos son validos y el ID no existia, la estacion queda en el
  //      AVL y totalEstaciones sube en 1. mensaje dice que paso.
  Estacion estacion;
  WINDOW *ventana = abrirVentana("Registrar estacion", 8, ANCHO_FORM);
  dibujarCampos(ventana, 3, (const char *const[]){"ID estacion", "Nombre",
                                                  "Costo transp."});
  bool completo = leerEntero(ventana, 2, &estacion.idEstacion) &&
                  leerTexto(ventana, 3, estacion.nombre) &&
                  leerReal(ventana, 4, &estacion.costoTransporte);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado o dato invalido");
  } else if (insertarEstacion(sistema->estaciones, &estacion)) {
    sistema->totalEstaciones++;
    fijarMensaje(mensaje, MSG_OK, "Estacion %d registrada", estacion.idEstacion);
  } else {
    fijarMensaje(mensaje, MSG_ERROR, "Ya existe la estacion %d",
                 estacion.idEstacion);
  }
}

// [AVL] Elimina una estacion por ID.
static void accionEliminarEstacion(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si la estacion existia, se elimina (el AVL se rebalancea) y
  //      totalEstaciones baja en 1. Si no, mensaje lo avisa.
  int idEstacion;
  WINDOW *ventana = abrirVentana("Eliminar estacion", 6, ANCHO_FORM);
  dibujarCampos(ventana, 1, (const char *const[]){"ID estacion"});
  bool completo = leerEntero(ventana, 2, &idEstacion);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado o dato invalido");
  } else if (eliminarEstacion(sistema->estaciones, idEstacion)) {
    sistema->totalEstaciones--;
    fijarMensaje(mensaje, MSG_OK, "Estacion %d eliminada", idEstacion);
  } else {
    fijarMensaje(mensaje, MSG_ERROR, "No existe la estacion %d", idEstacion);
  }
}

// [PILA] Asigna un paquete a una ruta y lo registra en el historial.
static void accionAsignarRuta(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si el paquete existe, la asignacion queda apilada en el historial y
  //      totalOperaciones sube en 1. Si no, mensaje lo avisa.
  char trackingID[LARGO_CAMPO + 1];
  char ruta[LARGO_CAMPO + 1];
  WINDOW *ventana = abrirVentana("Asignar paquete a ruta", 7, ANCHO_FORM);
  dibujarCampos(ventana, 2, (const char *const[]){"ID paquete", "Ruta"});
  bool completo = leerTexto(ventana, 2, trackingID) &&
                  leerTexto(ventana, 3, ruta);
  delwin(ventana);
  Paquete paquete;
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado");
    return;
  }
  if (!buscarPorID(sistema->paquetes, trackingID, &paquete)) {
    fijarMensaje(mensaje, MSG_ERROR, "No existe el paquete %s", trackingID);
    return;
  }
  Operacion operacion;
  snprintf(operacion.ruta, sizeof(operacion.ruta), "%s", ruta);
  snprintf(operacion.operacionRealizada, sizeof(operacion.operacionRealizada),
           "Asignar paquete %s", trackingID);
  if (apilar(sistema->historial, &operacion)) {
    sistema->totalOperaciones++;
    fijarMensaje(mensaje, MSG_OK, "Paquete %s asignado a %s", trackingID, ruta);
  } else {
    fijarMensaje(mensaje, MSG_ERROR, "No se pudo registrar la operacion");
  }
}

// [PILA] Deshace la ultima asignacion registrada.
static void accionDeshacer(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si habia operaciones, la ultima sale del historial y totalOperaciones
  //      baja en 1. Si no, mensaje lo avisa.
  Operacion operacion;
  if (!desapilar(sistema->historial, &operacion)) {
    fijarMensaje(mensaje, MSG_ERROR, "No hay nada para deshacer");
    return;
  }
  sistema->totalOperaciones--;
  fijarMensaje(mensaje, MSG_OK, "Deshecho: %s -> %s",
               operacion.operacionRealizada, operacion.ruta);
}

// [REC] Muestra altura y peso total del arbol de paquetes.
static void accionAuditarPaquetes(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si hay paquetes, muestra su cantidad, la altura del arbol y el peso
  //      acumulado. Si no, mensaje lo avisa. No modifica los datos.
  if (sistema->totalPaquetes == 0) {
    fijarMensaje(mensaje, MSG_ERROR, "No hay paquetes registrados");
    return;
  }
  mostrarResultado("Auditoria de paquetes",
                   "Paquetes registrados : %d\nAltura del arbol     : %d\n"
                   "Peso total acumulado : %.2f kg",
                   sistema->totalPaquetes, alturaABB(sistema->paquetes),
                   pesoTotalABB(sistema->paquetes));
  fijarMensaje(mensaje, MSG_NINGUNO, "");
}

// [REC] Audita una subred: altura y costo acumulado desde una estacion.
static void accionAuditarSubred(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: si la estacion existe, muestra la altura y el costo acumulado de su
  //      subred, y la altura de toda la red. Si no, mensaje lo avisa.
  int idRaiz;
  WINDOW *ventana = abrirVentana("Auditar subred", 6, ANCHO_FORM);
  dibujarCampos(ventana, 1, (const char *const[]){"ID raiz"});
  bool completo = leerEntero(ventana, 2, &idRaiz);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado o dato invalido");
    return;
  }
  Estacion raiz;
  if (!buscarEstacion(sistema->estaciones, idRaiz, &raiz)) {
    fijarMensaje(mensaje, MSG_ERROR, "No existe la estacion %d", idRaiz);
    return;
  }
  mostrarResultado("Auditoria de subred",
                   "Estacion raiz    : %d (%s)\nAltura de subred : %d\n"
                   "Costo acumulado  : $%.2f\n\nAltura de la red : %d",
                   raiz.idEstacion, raiz.nombre,
                   alturaSubredAVL(sistema->estaciones, idRaiz),
                   costoTotalSubredAVL(sistema->estaciones, idRaiz),
                   alturaAVL(sistema->estaciones));
  fijarMensaje(mensaje, MSG_NINGUNO, "");
}

// [REC] Busca un paquete por rango de costo y peso maximo.
static void accionBuscarPaqueteCosto(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: muestra el primer paquete (orden alfabetico) con costo en
  //      [minimo, maximo] y peso <= maximo, o avisa que no hay ninguno.
  float costoMinimo, costoMaximo, pesoMaximo;
  WINDOW *ventana = abrirVentana("Buscar paquete por costo y peso", 8, ANCHO_FORM);
  dibujarCampos(ventana, 3, (const char *const[]){"Costo minimo", "Costo maximo",
                                                  "Peso maximo"});
  bool completo = leerReal(ventana, 2, &costoMinimo) &&
                  leerReal(ventana, 3, &costoMaximo) &&
                  leerReal(ventana, 4, &pesoMaximo);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado o dato invalido");
    return;
  }
  Paquete paquete;
  if (!buscarPorCostoYPeso(sistema->paquetes, costoMinimo, costoMaximo,
                           pesoMaximo, &paquete)) {
    fijarMensaje(mensaje, MSG_ERROR, "Ningun paquete cumple la condicion");
    return;
  }
  mostrarResultado("Paquete encontrado",
                   "Tracking ID : %s\nDestino     : %s\nPeso        : %.2f kg\n"
                   "Costo envio : $%.2f",
                   paquete.trackingID, paquete.destino, paquete.peso,
                   paquete.costoEnvio);
  fijarMensaje(mensaje, MSG_NINGUNO, "");
}

// [REC] Busca una estacion por costo propio minimo y costo maximo de subred.
static void accionBuscarEstacionCosto(Sistema *sistema, Mensaje *mensaje) {
  // Pre: sistema y mensaje no son NULL.
  // Pos: muestra la primera estacion (por ID) con costo propio >= minimo y
  //      subred <= maximo, o avisa que no hay ninguna.
  float costoMinimo, costoMaximoSubred;
  WINDOW *ventana = abrirVentana("Buscar estacion por costos", 7, ANCHO_FORM);
  dibujarCampos(ventana, 2,
                (const char *const[]){"Costo minimo", "Subred maxima"});
  bool completo = leerReal(ventana, 2, &costoMinimo) &&
                  leerReal(ventana, 3, &costoMaximoSubred);
  delwin(ventana);
  if (!completo) {
    fijarMensaje(mensaje, MSG_ERROR, "Cancelado o dato invalido");
    return;
  }
  Estacion estacion;
  if (!buscarPorCostos(sistema->estaciones, costoMinimo, costoMaximoSubred,
                       &estacion)) {
    fijarMensaje(mensaje, MSG_ERROR, "Ninguna estacion cumple la condicion");
    return;
  }
  mostrarResultado("Estacion encontrada",
                   "Estacion      : %d (%s)\nCosto propio  : $%.2f\n"
                   "Costo subred  : $%.2f",
                   estacion.idEstacion, estacion.nombre,
                   estacion.costoTransporte,
                   costoTotalSubredAVL(sistema->estaciones, estacion.idEstacion));
  fijarMensaje(mensaje, MSG_NINGUNO, "");
}

// ---------------------------------------------------------------------------
// Menu
// ---------------------------------------------------------------------------

typedef struct {
  const char *modulo;
  const char *texto;
  void (*ejecutar)(Sistema *, Mensaje *); // NULL = salir
} Opcion;

static const Opcion OPCIONES[] = {
    {"[ABB]", "Registrar paquete", accionRegistrarPaquete},
    {"[ABB]", "Buscar paquete", accionBuscarPaquete},
    {"[ABB]", "Inventario alfabetico", accionVerInventario},
    {"[HEAP]", "Registrar envio urgente", accionRegistrarEnvio},
    {"[HEAP]", "Despachar siguiente urgente", accionDespacharSiguiente},
    {"[AVL]", "Registrar estacion", accionRegistrarEstacion},
    {"[AVL]", "Eliminar estacion", accionEliminarEstacion},
    {"[PILA]", "Asignar paquete a ruta", accionAsignarRuta},
    {"[PILA]", "Deshacer ultima asignacion", accionDeshacer},
    {"[REC]", "Auditar paquetes", accionAuditarPaquetes},
    {"[REC]", "Auditar subred", accionAuditarSubred},
    {"[REC]", "Buscar paquete por costo/peso", accionBuscarPaqueteCosto},
    {"[REC]", "Buscar estacion por costos", accionBuscarEstacionCosto},
    {"", "Salir", NULL}};

#define CANTIDAD_OPCIONES ((int)(sizeof(OPCIONES) / sizeof(OPCIONES[0])))

// Dibuja el panel del menu resaltando la opcion elegida.
static void dibujarMenu(WINDOW *ventana, int seleccion) {
  // Pre: ventana es el panel del menu y seleccion es un indice valido.
  // Pos: el panel queda dibujado (pendiente de refrescar).
  werase(ventana);
  box(ventana, 0, 0);
  wattron(ventana, A_BOLD);
  mvwprintw(ventana, 0, 2, " Menu ");
  wattroff(ventana, A_BOLD);
  for (int i = 0; i < CANTIDAD_OPCIONES; i++) {
    int fila = 1 + i;
    if (i == seleccion) {
      wattron(ventana, COLOR_PAIR(PAR_SELECCION) | A_BOLD);
      mvwprintw(ventana, fila, 1, " %-6s %-*s", OPCIONES[i].modulo,
                ANCHO_MENU - 10, OPCIONES[i].texto);
      wattroff(ventana, COLOR_PAIR(PAR_SELECCION) | A_BOLD);
    } else {
      wattron(ventana, COLOR_PAIR(PAR_ETIQUETA));
      mvwprintw(ventana, fila, 2, "%-6s", OPCIONES[i].modulo);
      wattroff(ventana, COLOR_PAIR(PAR_ETIQUETA));
      mvwprintw(ventana, fila, 9, "%s", OPCIONES[i].texto);
    }
  }
}

// Dibuja el panel de estado: contadores, proximo urgente y ultima asignacion.
static void dibujarEstado(WINDOW *ventana, const Sistema *sistema) {
  // Pre: ventana es el panel de estado y sistema no es NULL.
  // Pos: el panel queda dibujado (pendiente de refrescar). No modifica nada.
  int ancho = getmaxx(ventana);
  werase(ventana);
  box(ventana, 0, 0);
  wattron(ventana, A_BOLD);
  mvwprintw(ventana, 0, 2, " Estado ");
  mvwprintw(ventana, 2, 2, "Paquetes     %4d", sistema->totalPaquetes);
  mvwprintw(ventana, 3, 2, "Estaciones   %4d", sistema->totalEstaciones);
  mvwprintw(ventana, 4, 2, "Urgencias    %4d", sistema->totalUrgencias);
  mvwprintw(ventana, 5, 2, "Historial    %4d", sistema->totalOperaciones);
  wattroff(ventana, A_BOLD);

  wattron(ventana, COLOR_PAIR(PAR_ETIQUETA));
  mvwprintw(ventana, 7, 2, "Proximo urgente");
  mvwprintw(ventana, 11, 2, "Ultima asignacion");
  wattroff(ventana, COLOR_PAIR(PAR_ETIQUETA));

  Envio envio;
  if (consultarMaxUrgencia(sistema->urgencias, &envio)) {
    mvwprintw(ventana, 8, 3, "envio %d", envio.idEnvio);
    mvwprintw(ventana, 9, 3, "urgencia %d, limite %d", envio.nivelUrgencia,
              envio.tiempoLimite);
  } else {
    wattron(ventana, A_DIM);
    mvwprintw(ventana, 8, 3, "(cola vacia)");
    wattroff(ventana, A_DIM);
  }

  Operacion operacion;
  if (consultarCima(sistema->historial, &operacion)) {
    mvwprintw(ventana, 12, 3, "%.*s", ancho - 6, operacion.operacionRealizada);
    mvwprintw(ventana, 13, 3, "-> %.*s", ancho - 9, operacion.ruta);
  } else {
    wattron(ventana, A_DIM);
    mvwprintw(ventana, 12, 3, "(sin operaciones)");
    wattroff(ventana, A_DIM);
  }
}

// Redibuja toda la pantalla: titulo, paneles, mensaje y ayuda.
static void dibujarPantalla(WINDOW *panelMenu, WINDOW *panelEstado,
                            const Sistema *sistema, int seleccion,
                            const Mensaje *mensaje) {
  // Pre: los paneles estan abiertos y los punteros no son NULL.
  // Pos: la pantalla queda actualizada.
  const char *titulo = "LogiExpress 360  -  Gestion y Despacho de Envios Criticos";
  erase();
  attron(COLOR_PAIR(PAR_TITULO) | A_BOLD);
  mvhline(0, 0, ' ', COLS);
  mvprintw(0, (COLS - (int)strlen(titulo)) / 2, "%s", titulo);
  attroff(COLOR_PAIR(PAR_TITULO) | A_BOLD);

  int filaMensaje = 2 + ALTO_PANEL + 1;
  if (mensaje->tipo == MSG_OK) {
    attron(COLOR_PAIR(PAR_OK) | A_BOLD);
    mvprintw(filaMensaje, 2, "[OK] %s", mensaje->texto);
    attroff(COLOR_PAIR(PAR_OK) | A_BOLD);
  } else if (mensaje->tipo == MSG_ERROR) {
    attron(COLOR_PAIR(PAR_ERROR) | A_BOLD);
    mvprintw(filaMensaje, 2, "[!] %s", mensaje->texto);
    attroff(COLOR_PAIR(PAR_ERROR) | A_BOLD);
  }
  attron(A_DIM);
  mvprintw(LINES - 1, 2, "Flechas o j/k: mover    Enter: elegir    q: salir");
  attroff(A_DIM);

  dibujarMenu(panelMenu, seleccion);
  dibujarEstado(panelEstado, sistema);
  wnoutrefresh(stdscr);
  wnoutrefresh(panelMenu);
  wnoutrefresh(panelEstado);
  doupdate();
}

// ---------------------------------------------------------------------------
// Sistema y programa principal
// ---------------------------------------------------------------------------

// Crea los 4 TADs y deja los contadores en cero.
static bool crearSistema(Sistema *sistema) {
  // Pre: sistema no es NULL.
  // Pos: devuelve true con todo creado. Si algo falla devuelve false y no
  //      queda memoria reservada.
  sistema->paquetes = crearABB();
  sistema->urgencias = crearMonton(16);
  sistema->estaciones = crearAVL();
  sistema->historial = crearPila();
  sistema->totalPaquetes = 0;
  sistema->totalUrgencias = 0;
  sistema->totalEstaciones = 0;
  sistema->totalOperaciones = 0;
  if (sistema->paquetes && sistema->urgencias && sistema->estaciones &&
      sistema->historial)
    return true;
  destruirABB(sistema->paquetes);
  destruirMonton(sistema->urgencias);
  destruirAVL(sistema->estaciones);
  destruirPila(sistema->historial);
  return false;
}

// Libera los 4 TADs.
static void destruirSistema(Sistema *sistema) {
  // Pre: sistema fue creado con crearSistema (o es NULL).
  // Pos: toda la memoria queda liberada.
  if (sistema != NULL) {
    destruirABB(sistema->paquetes);
    destruirMonton(sistema->urgencias);
    destruirAVL(sistema->estaciones);
    destruirPila(sistema->historial);
  }
}

// Inicia ncurses con teclado y colores.
static void iniciarPantalla(void) {
  // Pre: ninguna.
  // Pos: ncurses queda iniciado (sin eco, cursor oculto, flechas activas).
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(PAR_TITULO, COLOR_BLACK, COLOR_CYAN);
    init_pair(PAR_SELECCION, COLOR_BLACK, COLOR_CYAN);
    init_pair(PAR_OK, COLOR_GREEN, -1);
    init_pair(PAR_ERROR, COLOR_RED, -1);
    init_pair(PAR_ETIQUETA, COLOR_YELLOW, -1);
  }
}

int main(void) {
  Sistema sistema;
  if (!crearSistema(&sistema)) {
    fprintf(stderr, "No se pudo reservar memoria\n");
    return 1;
  }

  iniciarPantalla();
  if (LINES < FILAS_MIN || COLS < COLUMNAS_MIN) {
    endwin();
    fprintf(stderr, "La terminal debe ser de al menos %dx%d (es %dx%d)\n",
            COLUMNAS_MIN, FILAS_MIN, COLS, LINES);
    destruirSistema(&sistema);
    return 1;
  }

  WINDOW *panelMenu = newwin(ALTO_PANEL, ANCHO_MENU, 2, 1);
  WINDOW *panelEstado =
      newwin(ALTO_PANEL, COLS - ANCHO_MENU - 3, 2, ANCHO_MENU + 2);
  Mensaje mensaje = {"", MSG_NINGUNO};
  int seleccion = 0;
  bool salir = false;

  while (!salir) {
    dibujarPantalla(panelMenu, panelEstado, &sistema, seleccion, &mensaje);
    int tecla = getch();
    switch (tecla) {
    case KEY_UP:
    case 'k':
      seleccion = (seleccion + CANTIDAD_OPCIONES - 1) % CANTIDAD_OPCIONES;
      break;
    case KEY_DOWN:
    case 'j':
      seleccion = (seleccion + 1) % CANTIDAD_OPCIONES;
      break;
    case 'q':
      salir = true;
      break;
    case '\n':
    case KEY_ENTER:
      if (OPCIONES[seleccion].ejecutar == NULL) {
        salir = true;
      } else {
        OPCIONES[seleccion].ejecutar(&sistema, &mensaje);
      }
      break;
    default:
      break;
    }
  }

  delwin(panelMenu);
  delwin(panelEstado);
  endwin();
  destruirSistema(&sistema);
  return 0;
}
