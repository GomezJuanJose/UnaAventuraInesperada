/**
* UNA AVENTURA INESPERADA, juego de puzzles creado para NDS en C usando devkitpro.
*
* @author  Juan José Gómez Simón
* @version 0.8
* @since  09-12-2020 
*/

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <nds/ndstypes.h>

//Teselas
#include "teselas.h"

//Imagenes
#include "menuPrincipal.h"
#include "menuCreditos.h"
#include "HUD.h"
#include "CinematicaInicioF1.h"
#include "CinematicaInicioF2.h"
#include "CinematicaInicioF3.h"
#include "CinematicaFinalF1.h"
#include "CinematicaFinalF2.h"
#include "CinematicaFinalF3.h"
#include "Pregunta1-1.h"
#include "Pregunta1-2.h"
#include "Pregunta1-3.h"
#include "Pregunta2-1.h"
#include "Pregunta2-2.h"
#include "Pregunta3-1.h"
#include "Pregunta3-2.h"
#include "Pregunta4.h"
#include "Pregunta5-1.h"
#include "Pregunta5-2.h"
#include "PreguntaFallida.h"
#include "PreguntasAcertadas.h"
#include "PreguntasAcertadasFinal.h"

//--MACROS--
#define TIMER_SPEED (BUS_CLOCK/1024)

#define ANCHO_BARRA_ESTAMINA 32
#define ALTO_BARRA_ESTAMINA 155
#define COMIENZO_LINEA_BARRA_ESTAMINA 38
#define COMIENZO_COLUMNA_BARRA_ESTAMINA 20

#define MOVIMIENTOS_NIV1 18
#define MOVIMIENTOS_NIV2 19
#define MOVIMIENTOS_NIV3 23
#define MOVIMIENTOS_NIV4 21
#define MOVIMIENTOS_NIV5 24

//--FUNCIONES--
void ConfigurarInterrupciones();
void TeclasJugador();
void ActualizarTemporizador();
void MoverEnemigo(int direccion, int enemigoActual);
void MoverObstaculo(int direccion);
void ConsultarSistemaDialogo();
void HabilitarBotonesDialogo();
void GenerarNivel(u16 mapa[],unsigned int imagen[], int numeroMovimientosMax);
void ActualizarBarraMovimientos();
void CrearMenu();
void ActualizarAnimacion();
void InicializarTeselas();

bool CrearDialogo(unsigned int imagen[], int opcionCorrecta);

int ComprobarSuelo(int posicion,u16 mapa[]);
int ElegirFondoJugador(int indiceTesela,int posicion);

//--ESTRUCTURAS--
//Puntos para formar los botones de interaccion en la pantalla
struct PuntoPantalla{
	int x;
	int y;
};

//Posicion del enemigo en la patalla
struct PosicionEnemigo{
	int x;
	int y;
	int vivo;
};



//--VARIABLES--
bool esPartidaAcabada = true;
bool jugadorVivo = false;
bool puedeJugadorMoverse = true;
bool esActivoBotonReinicio = false;
bool esActivoBotonesDialogos = true;
bool esAtrasBotonActivo = false;
bool esJuegoComenzado = false;
bool esJuegoReiniciado = false;
bool esFotograma1Activo = true;

unsigned short *fb;

int posJugFila = 22, posJugColumna = 16;
int posNpcFila = 17, posNpcColumna = 27;
int movimientosJugador= 20;
int maximoMovimientosJugador=20;
int tiempo = 0;
int pos_mapMemory;
int pos_mapData;
int record;
int opcionElegida = -1; //-1: ninguna, 0: primera, 1: segunda
int nivelActual=0;
int numeroDeEnemigos=0;

u8 sound_id = 0;
static u8*  tileMemory;

u16 newpress = 0;
static u16* mapMemory;
//Para que ComprobarSuelo tenga copia del mapa actual sin modificar
static u16* mapaAcutal;

struct PuntoPantalla puntosBoton1 [2]={
	{50,138},
	{213,161}
};
struct PuntoPantalla puntosBoton2 [2]={
	{50,165},
	{213,189}
};
struct PuntoPantalla puntosComenzaPartidaBoton [2]={
	{66,70},
	{184,93}
};
struct PuntoPantalla puntosCreditosBoton [2]={
	{66,134},
	{184,158}
};
struct PuntoPantalla puntosAtrasBoton [2]={
	{65,162},
	{184,187}
};
struct PuntoPantalla puntosReinicioNivelBoton [2]={
	{117,100},
	{236,125}
};

struct PosicionEnemigo posicionesEnemigo [10];

touchPosition posicionXY;


/**
 * Funcion main, aqui se inicializa el hardware, las variables, se configura las interrupciones y se reserva en memoria las teselas y los colores.
 * Además gestiona también cuando el jugador pulsa el boton de reiniciar el nivel(si dicho boton esta habilitado). 
 */
int main( void )
{

	//Inicializa el hardware
	REG_POWERCNT = POWER_ALL_2D;

	REG_DISPCNT_SUB  = MODE_0_2D   | DISPLAY_BG0_ACTIVE ;
	VRAM_C_CR    = VRAM_ENABLE | VRAM_C_SUB_BG ;

	//Pantalla para el menu
	VRAM_A_CR = VRAM_ENABLE | VRAM_A_LCD;
	VRAM_B_CR = VRAM_ENABLE | VRAM_B_LCD;
	fb = VRAM_A;

	tileMemory = (u8*)  BG_TILE_RAM_SUB(1);
	mapMemory  = (u16*) BG_MAP_RAM_SUB(0);

	//Inicia la paleta de colores
	BG_PALETTE_SUB[0]= RGB15(0, 0, 0);
	BG_PALETTE_SUB[1]= RGB15(15, 4, 6);
	BG_PALETTE_SUB[2]= RGB15(18, 8, 5);
	BG_PALETTE_SUB[3]= RGB15(9, 20, 9);
	BG_PALETTE_SUB[4]= RGB15(31, 25, 20);
	BG_PALETTE_SUB[5]= RGB15(28, 23, 18);
	BG_PALETTE_SUB[6]= RGB15(12, 6, 11);
	BG_PALETTE_SUB[7]= RGB15(20, 5, 10);
	BG_PALETTE_SUB[8]= RGB15(6, 8, 10);
	BG_PALETTE_SUB[9]= RGB15(31, 25, 8);
	BG_PALETTE_SUB[10]= RGB15(11, 17, 21);
	BG_PALETTE_SUB[11]= RGB15(9, 14, 17);
	BG_PALETTE_SUB[12]= RGB15(8, 5, 7);
	BG_PALETTE_SUB[13]= RGB15(5, 5, 10);
	BG_PALETTE_SUB[14]= RGB15(9, 8, 13);
	BG_PALETTE_SUB[15]= RGB15(15, 6, 6);
	BG_PALETTE_SUB[16]= RGB15(11, 5, 6);
	BG_PALETTE_SUB[17]= RGB15(18, 17, 16);
	BG_PALETTE_SUB[18]= RGB15(7, 5, 4);
	BG_PALETTE_SUB[19]= RGB15(15, 11, 7);
	BG_PALETTE_SUB[20]= RGB15(11, 8, 5);
	BG_PALETTE_SUB[21]= RGB15(18, 13, 9);
	BG_PALETTE_SUB[22]= RGB15(13, 9, 5);
	BG_PALETTE_SUB[23]= RGB15(15, 13, 12);
	BG_PALETTE_SUB[24]= RGB15(3, 2, 3);
	BG_PALETTE_SUB[25]= RGB15(10, 7, 8);
	BG_PALETTE_SUB[26]= RGB15(31, 31, 31);

	//Inicializa las teselas
	InicializarTeselas();

	//Configura las interrupciones
	ConfigurarInterrupciones();
	
	tiempo = 0;
	record = 1000;

	//Crea el menu principal
	CrearMenu(menuTitulo,menuPrincipalBitmap,menuCreditosBitmap);

	//Detecta si el jugador pulsa el boton de reiniciar el nivel
	u32 keys;
	while(1)
	{	
		//si ha comenzado el juego que revise si se ha pulsado la pantalla para reiniciar el nivel
		if(esActivoBotonReinicio == true){
			scanKeys();
			keys = keysCurrent();
			if(keys & KEY_TOUCH  && esActivoBotonesDialogos == true){
				touchRead(&posicionXY);
				//Si esta en el rango del boton
				if((posicionXY.px >= puntosReinicioNivelBoton[0].x && posicionXY.px <= puntosReinicioNivelBoton[1].x) && (posicionXY.py >= puntosReinicioNivelBoton[0].y && posicionXY.py <= puntosReinicioNivelBoton[1].y)){
					//Reinicia el nivel
					esJuegoReiniciado = true;
					ConsultarSistemaDialogo();
				}
			}
			swiWaitForVBlank();
		}
	}
}


/**
 * Configura las interrupciones de teclado y dos temporizadores: uno para alternar entre sprites del jugador y los npc's para así animarlos
 * y otra es para dar pausar entre dialogos y así no pasarlos con solo tocar una vez la pantalla táctil.
 */
void ConfigurarInterrupciones(){
 	//Habilita las interrupciones de teclado
  	irqSet(IRQ_KEYS,TeclasJugador);
  	irqEnable(IRQ_KEYS);
  	REG_KEYCNT = 0x7FFF;

  	//Temporizador de dialogos
  	irqEnable(IRQ_TIMER0);
  	irqSet(IRQ_TIMER0,HabilitarBotonesDialogo);
  	TIMER_DATA(0)=32768; 
  	TIMER_CR(0) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ ;
  	timerPause(0);
  
  	//Temporizador para actualizar la animacion
  	irqEnable(IRQ_TIMER1);
  	irqSet(IRQ_TIMER1,ActualizarAnimacion);
  	TIMER_DATA(1)=32768; 
  	TIMER_CR(1) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ ;
}


/**
 * Función llamada cada vez que se detecte una interrupción via input(con la pantalla táctil no salta). Se encarga de mover al jugador a la
 * siguiente cuadrícula y además detecta si hay una una caja y un enemigo en la posición que ocupará el jugador para moverlo, eliminarlo o no
 * mover al propio jugador.
 * <p>
 * Nota: Solo se comprueba 2 teselas diagonales si no estan tocando vacio ya que los mapas se crearan siguiendo una cuadricula 2x2
 * </p>
 */
void TeclasJugador(){
	// Comprueba si el jugador tiene movimientos, qué tecla ha pulsado, si no esta en el borde de la pantalla, si la partida no esta acabada(para evitar que se mueva en el menu principal
	// o evitar que se mueva cuando elimine a un enemigo), tambien comprueba si su tesela contigua no es vacio(el fondo negro) y comprueba si su tesela contigua no es un muro.
	if (movimientosJugador > 0 && REG_KEYINPUT == 0x03BF && posJugFila > 0 && esPartidaAcabada==false && mapMemory[(posJugFila-1)*32+posJugColumna] != 18 && mapMemory[(posJugFila-1)*32+posJugColumna] != 28){//Boton arriba
		// Si se ha topado con una caja en su cuadricula contigua
		if(mapMemory[(posJugFila-2)*32+posJugColumna] == 19){
			MoverObstaculo(0);
		}else 
		// Si por el contrario se ha encontrado con un enemigo(tanto en su tesela 1 o 2 de la animacion)
		if(mapMemory[(posJugFila-2)*32+posJugColumna] == 5 || mapMemory[(posJugFila-2)*32+posJugColumna] == 42){
			int enemActual=-1;
			// Busca el enemigo el cual va a mover para más tarde a la funcion MoverEnemigo actualizar su posicion y usarla para seguir actualzando la animacion
			// y así no se dupliquen n veces los enemigos.
			for (int i=0;i<numeroDeEnemigos && enemActual == -1;i++){
				if(posJugFila-2==posicionesEnemigo[i].y && posJugColumna==posicionesEnemigo[i].x){
					enemActual = i;
				}
			}
			MoverEnemigo(0,enemActual);
		}

		// Si ha pesar de haber llamado a la función MoverObstaculo o MoverEnemigo siguen ahí eso implica que o hay otro obstaculo o están pegados
		// a un muro. Por lo tanto el jugador no podrá moverse
		if(mapMemory[(posJugFila-2)*32+posJugColumna] != 19 && mapMemory[(posJugFila-2)*32+posJugColumna] != 5 && puedeJugadorMoverse == true){
			//Comprueba el suelo que tiene el mapa para dejar el original
			mapMemory[(posJugFila)*32+posJugColumna] = ComprobarSuelo((posJugFila)*32+posJugColumna, mapaAcutal);
			mapMemory[(posJugFila)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila)*32+(posJugColumna+1), mapaAcutal);
			mapMemory[(posJugFila+1)*32+posJugColumna] = ComprobarSuelo((posJugFila+1)*32+posJugColumna, mapaAcutal);
			mapMemory[(posJugFila+1)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila+1)*32+(posJugColumna+1), mapaAcutal);

			posJugFila              -=2;

			//Desplaza el jugador en base a el suelo que va ha ocupar
			pos_mapMemory            = posJugFila*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(0,pos_mapMemory);

			pos_mapMemory            = posJugFila*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(1,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(2,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(3,pos_mapMemory);
		}
		//Se resta uno de estamina
		movimientosJugador--;
		//Puede volver a moverse
		puedeJugadorMoverse = true;
		//Actualiza la barra de movimiento
		ActualizarBarraMovimientos();

    }else if (movimientosJugador > 0 && REG_KEYINPUT == 0x037F && posJugFila+1 < 23 && esPartidaAcabada==false && mapMemory[(posJugFila+2)*32+(posJugColumna+1)] != 18 && mapMemory[(posJugFila+2)*32+(posJugColumna+1)] != 27){//Boton abajo
 		if(mapMemory[(posJugFila+2)*32+posJugColumna] == 19){
			MoverObstaculo(1);
		}else if(mapMemory[(posJugFila+2)*32+posJugColumna] == 5  || mapMemory[(posJugFila+2)*32+posJugColumna] == 42){
			int enemActual=-1;
			for (int i=0;i<numeroDeEnemigos && enemActual == -1;i++){
				if(posJugFila + 2==posicionesEnemigo[i].y && posJugColumna==posicionesEnemigo[i].x){
					enemActual = i;
				}
			}
			MoverEnemigo(1,enemActual);
		}

		if(mapMemory[(posJugFila+2)*32+posJugColumna] != 19 && mapMemory[(posJugFila+2)*32+posJugColumna] != 5 && puedeJugadorMoverse == true){
	    	mapMemory[(posJugFila+1)*32+posJugColumna] = ComprobarSuelo((posJugFila)*32+posJugColumna, mapaAcutal);
			mapMemory[(posJugFila+1)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila)*32+(posJugColumna+1), mapaAcutal);
			mapMemory[(posJugFila)*32+posJugColumna] = ComprobarSuelo((posJugFila)*32+posJugColumna, mapaAcutal);
			mapMemory[(posJugFila)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila)*32+(posJugColumna+1), mapaAcutal);

			posJugFila              +=2;
			
			pos_mapMemory            = posJugFila*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(0,pos_mapMemory);

			pos_mapMemory            = posJugFila*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(1,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(2,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(3,pos_mapMemory);
		}
		movimientosJugador--;
		puedeJugadorMoverse = true;
		ActualizarBarraMovimientos();

    }else 
    // Adicionalmente al moverse hacia la izquierda o derecha se comprueba que no se está chocando contra los muros orientados en estas posiciones(teselas 24 y 17) 
    if (movimientosJugador > 0 && REG_KEYINPUT == 0x03DF && posJugColumna > 0 && esPartidaAcabada==false && mapMemory[(posJugFila)*32+(posJugColumna-1)] != 18 && mapMemory[(posJugFila)*32+(posJugColumna-1)] != 24 && mapMemory[(posJugFila)*32+(posJugColumna-1)] != 27){//Boton izquierda
    	if(mapMemory[(posJugFila)*32+(posJugColumna-2)] == 19){
			MoverObstaculo(2);
		}else if(mapMemory[(posJugFila)*32+(posJugColumna-2)] == 5  || mapMemory[(posJugFila)*32+(posJugColumna-2)] == 42){
			int enemActual=-1;
			for (int i=0;i<numeroDeEnemigos && enemActual == -1;i++){
				if(posJugFila==posicionesEnemigo[i].y && posJugColumna-2==posicionesEnemigo[i].x){
					enemActual = i;
				}
			}
			MoverEnemigo(2,enemActual);
		}

		if(mapMemory[(posJugFila)*32+(posJugColumna-2)] != 19 && mapMemory[(posJugFila)*32+(posJugColumna-2)] != 5 && puedeJugadorMoverse == true){
	    	mapMemory[(posJugFila)*32+(posJugColumna)] = ComprobarSuelo((posJugFila)*32+(posJugColumna), mapaAcutal);
			mapMemory[(posJugFila+1)*32+(posJugColumna)] = ComprobarSuelo((posJugFila+1)*32+(posJugColumna), mapaAcutal);
			mapMemory[(posJugFila)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila)*32+(posJugColumna+1), mapaAcutal);
			mapMemory[(posJugFila+1)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila+1)*32+(posJugColumna+1), mapaAcutal);
			
			posJugColumna           -=2;
			
			pos_mapMemory            = posJugFila*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(0,pos_mapMemory);

			pos_mapMemory            = posJugFila*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(1,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(2,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(3,pos_mapMemory);
		}
		movimientosJugador--;
		puedeJugadorMoverse = true;
		ActualizarBarraMovimientos();

    }else if (movimientosJugador > 0 && REG_KEYINPUT == 0x03EF && posJugColumna+1 < 31 && esPartidaAcabada==false && mapMemory[(posJugFila+1)*32+(posJugColumna+2)] != 18  && mapMemory[(posJugFila+1)*32+(posJugColumna+2)] != 17 && mapMemory[(posJugFila)*32+(posJugColumna+2)] != 26){//Boton derecha
		if(mapMemory[(posJugFila)*32+(posJugColumna+2)] == 19){
			MoverObstaculo(3);
		}else if(mapMemory[(posJugFila)*32+(posJugColumna+2)] == 5  || mapMemory[(posJugFila)*32+(posJugColumna+2)] == 42){
			int enemActual=-1;
			for (int i=0;i<numeroDeEnemigos && enemActual == -1;i++){
				if(posJugFila==posicionesEnemigo[i].y && posJugColumna+2==posicionesEnemigo[i].x){
					enemActual = i;
				}
			}
			MoverEnemigo(3,enemActual);
		}


		if(mapMemory[(posJugFila)*32+(posJugColumna+2)] != 19 && mapMemory[(posJugFila)*32+(posJugColumna+2)] != 5 && puedeJugadorMoverse == true){
	    	mapMemory[(posJugFila)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila)*32+(posJugColumna+1), mapaAcutal);
			mapMemory[(posJugFila+1)*32+(posJugColumna+1)] = ComprobarSuelo((posJugFila+1)*32+(posJugColumna+1), mapaAcutal);
			mapMemory[(posJugFila)*32+posJugColumna] = ComprobarSuelo((posJugFila)*32+posJugColumna, mapaAcutal);
			mapMemory[(posJugFila+1)*32+(posJugColumna)] = ComprobarSuelo((posJugFila+1)*32+(posJugColumna), mapaAcutal);

			posJugColumna           +=2;
			
			pos_mapMemory            = posJugFila*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(0,pos_mapMemory);

			pos_mapMemory            = posJugFila*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(1,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+posJugColumna;
			mapMemory[pos_mapMemory] = ElegirFondoJugador(2,pos_mapMemory);

			pos_mapMemory            = (posJugFila+1)*32+(posJugColumna+1);
			mapMemory[pos_mapMemory] = ElegirFondoJugador(3,pos_mapMemory);
		}
		movimientosJugador--;
		puedeJugadorMoverse = true;
		ActualizarBarraMovimientos();
    }
}


/**
 * Comprueba que suelo habia originalmente en el mapa
 * @param posicion Posicion del mapa oiginal.
 * @param mapa Mapa original actual
 * @return Tesela del mapa original
 */
int ComprobarSuelo(int posicion,u16 mapa[]){
	switch(mapa[posicion]){
		case 0: return 1;
		case 1: return 1;
		case 2: return 2;
		case 3: return 3;
		case 5: return 3;
		default: return 1; 
	}
}


/**
 * Comprueba el fondo que tendra el jugador al desplazarse
 * @param indiceTesela La tesela que se va ha actualizar
 * @param posicion Posicion del mapa actual
 * @return Retorna la tesela correspondiente
 */
int ElegirFondoJugador(int indiceTesela,int posicion){
	if(indiceTesela==0){
		//Comprueba que tipo de suelo ocupará el jugador
		switch(mapMemory[posicion]){
			// Comprueba que tesela de la animacion tiene que elegir para sincronizarlo al ritmo del temporizador y no de los movimientos
			case 1: return esFotograma1Activo==true ? 0 : 34;
			case 3: return esFotograma1Activo==true ? 6 : 34;
			case 0:
			case 8:
			case 9:
			case 10: return esFotograma1Activo==true ? 0 : 34;
			// Si llega al final del mapa consulta el sistema de diálogos.
			case 23: ConsultarSistemaDialogo(); return 6;
			default: return esFotograma1Activo==true ? 6 : 34;
		}
	}else if(indiceTesela==1){
		switch(mapMemory[posicion]){
			case 1: return esFotograma1Activo==true ? 8 : 35;
			case 3: return esFotograma1Activo==true ? 11 : 35;
			case 0:
			case 8:
			case 9:
			case 10: return esFotograma1Activo==true ? 8 : 35; 
			default: return esFotograma1Activo==true ? 11 : 35; 
		}
	}else if(indiceTesela==2){
		switch(mapMemory[posicion]){
			case 1: return esFotograma1Activo==true ? 9 : 36;
			case 3: return esFotograma1Activo==true ? 12 : 36;
			case 0:
			case 8:
			case 9:
			case 10: return esFotograma1Activo==true ? 9 : 36;
			default: return esFotograma1Activo==true ? 12 : 36; 
		}
	}else{
		switch(mapMemory[posicion]){
			case 1: return esFotograma1Activo==true ? 10 : 37;
			case 3: return esFotograma1Activo==true ? 13 : 37;
			case 0:
			case 8:
			case 9:
			case 10: return esFotograma1Activo==true ? 10 : 37;
			default: return esFotograma1Activo==true ? 13 : 37; 
		}
	}
}


/**
 * Mueve el enemigo dependiendo de si tiene su casilla contigua libre de no ser así es eliminado y se evita que el jugador se pueda mover
 * @param deireccion Direccion en la que se moverá el enemigo
 * @enemigoActual Identificador del enemigo que se movera y se comprobará si es eliminado o no (para así no actualizar su animacion)
 */
void MoverEnemigo(int direccion, int enemigoActual){
	if(direccion == 0){
		// Comprueba si en su tesela contigua no hay vacio, una caja o un muro. De ser así se puede mover y actualiza la posicion
		if(mapMemory[(posJugFila-4)*32+posJugColumna] != 18 && mapMemory[(posJugFila-4)*32+posJugColumna] != 19 && mapMemory[(posJugFila-4)*32+posJugColumna] != 5 && mapMemory[(posJugFila-4)*32+posJugColumna] != 26){
			mapMemory[(posJugFila-4)*32+posJugColumna] = 5;
			mapMemory[(posJugFila-4)*32+(posJugColumna+1)] = 14;
			mapMemory[(posJugFila-3)*32+posJugColumna] = 15;
			mapMemory[(posJugFila-3)*32+(posJugColumna+1)] = 16;

			posicionesEnemigo[enemigoActual].y = posJugFila-4;
			posicionesEnemigo[enemigoActual].x = posJugColumna;

		}else{ 
			//De no ser asi se establece que es eliminado y el jugador no se podrá mover(para asi hacerle gastar otro de estamina en moverse)
			posicionesEnemigo[enemigoActual].vivo = false;
			puedeJugadorMoverse = false;
		}
		// Se borra la posicion actual del enemigo por suelo
		mapMemory[(posJugFila-2)*32+posJugColumna] = 3;
		mapMemory[(posJugFila-2)*32+(posJugColumna+1)] = 3;
		mapMemory[(posJugFila-1)*32+posJugColumna] = 3;
		mapMemory[(posJugFila-1)*32+(posJugColumna+1)] = 3;

	}else if(direccion == 1){
		if(mapMemory[(posJugFila+4)*32+posJugColumna] != 18 && mapMemory[(posJugFila+4)*32+posJugColumna] != 19 && mapMemory[(posJugFila+4)*32+posJugColumna] != 5 && mapMemory[(posJugFila+4)*32+posJugColumna] != 26){
			mapMemory[(posJugFila+5)*32+posJugColumna] = 15;
			mapMemory[(posJugFila+5)*32+(posJugColumna+1)] = 16;
			mapMemory[(posJugFila+4)*32+posJugColumna] = 5;
			mapMemory[(posJugFila+4)*32+(posJugColumna+1)] = 14;

			posicionesEnemigo[enemigoActual].y = posJugFila+4;
			posicionesEnemigo[enemigoActual].x = posJugColumna;
		}else{
			posicionesEnemigo[enemigoActual].vivo = false;
			puedeJugadorMoverse = false;
		}

		mapMemory[(posJugFila+3)*32+posJugColumna] = 3;
		mapMemory[(posJugFila+3)*32+(posJugColumna+1)] = 3;
		mapMemory[(posJugFila+2)*32+posJugColumna] = 3;
		mapMemory[(posJugFila+2)*32+(posJugColumna+1)] = 3;

	}else if(direccion == 2){
		if(mapMemory[(posJugFila)*32+(posJugColumna-4)] != 18 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 19 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 26 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 5){
			mapMemory[(posJugFila)*32+(posJugColumna-4)] = 5;
			mapMemory[(posJugFila)*32+(posJugColumna-3)] = 14;
			mapMemory[(posJugFila+1)*32+(posJugColumna-4)] = 15;
			mapMemory[(posJugFila+1)*32+(posJugColumna-3)] = 16;

			posicionesEnemigo[enemigoActual].y = posJugFila;
			posicionesEnemigo[enemigoActual].x = posJugColumna-4;
		}else{
			posicionesEnemigo[enemigoActual].vivo = false;
			puedeJugadorMoverse = false;
		}

		mapMemory[(posJugFila)*32+(posJugColumna-2)] = 3;
		mapMemory[(posJugFila)*32+(posJugColumna-1)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna-2)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna-1)] = 3;

	}else if(direccion == 3){
		if(mapMemory[(posJugFila)*32+(posJugColumna+4)] != 18 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 19 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 26 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 5){
			mapMemory[(posJugFila)*32+(posJugColumna+4)] = 5;
			mapMemory[(posJugFila)*32+(posJugColumna+5)] = 14;
			mapMemory[(posJugFila+1)*32+(posJugColumna+4)] = 15;
			mapMemory[(posJugFila+1)*32+(posJugColumna+5)] = 16;

			posicionesEnemigo[enemigoActual].y = posJugFila;
			posicionesEnemigo[enemigoActual].x = posJugColumna+4;
		}else{
			posicionesEnemigo[enemigoActual].vivo = false;
			puedeJugadorMoverse = false;
		}

		mapMemory[(posJugFila)*32+(posJugColumna+2)] = 3;
		mapMemory[(posJugFila)*32+(posJugColumna+3)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna+2)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna+3)] = 3;
	}
}


/**
 * Mueve los obstaculos dependiendo si no hay vacio o un enemigo obstaculizando.
 * @param direccion La direccion en la que se mueve el obstaculo
 */
void MoverObstaculo(int direccion){
	//Comprueba si no hay vacio, otra caja, un enemigo o muros. De ser así puede moverse
	if(direccion == 0 && mapMemory[(posJugFila-4)*32+posJugColumna] != 18 && mapMemory[(posJugFila-4)*32+posJugColumna] != 19 && mapMemory[(posJugFila-4)*32+posJugColumna] != 26 && mapMemory[(posJugFila-4)*32+posJugColumna] != 5 && mapMemory[(posJugFila-4)*32+posJugColumna] != 42){
		mapMemory[(posJugFila-4)*32+posJugColumna] = 19;
		mapMemory[(posJugFila-4)*32+(posJugColumna+1)] = 20;
		mapMemory[(posJugFila-3)*32+posJugColumna] = 21;
		mapMemory[(posJugFila-3)*32+(posJugColumna+1)] = 22;

		mapMemory[(posJugFila-2)*32+posJugColumna] = 3;
		mapMemory[(posJugFila-2)*32+(posJugColumna+1)] = 3;
		mapMemory[(posJugFila-1)*32+posJugColumna] = 3;
		mapMemory[(posJugFila-1)*32+(posJugColumna+1)] = 3;

	}else if(direccion == 1 && mapMemory[(posJugFila+4)*32+posJugColumna] != 18 && mapMemory[(posJugFila+4)*32+posJugColumna] != 19 && mapMemory[(posJugFila+4)*32+posJugColumna] != 26 && mapMemory[(posJugFila+4)*32+posJugColumna] != 5 && mapMemory[(posJugFila+4)*32+posJugColumna] != 42){
		mapMemory[(posJugFila+5)*32+posJugColumna] = 21;
		mapMemory[(posJugFila+5)*32+(posJugColumna+1)] = 22;
		mapMemory[(posJugFila+4)*32+posJugColumna] = 19;
		mapMemory[(posJugFila+4)*32+(posJugColumna+1)] = 20;

		mapMemory[(posJugFila+3)*32+posJugColumna] = 3;
		mapMemory[(posJugFila+3)*32+(posJugColumna+1)] = 3;
		mapMemory[(posJugFila+2)*32+posJugColumna] = 3;
		mapMemory[(posJugFila+2)*32+(posJugColumna+1)] = 3;

	}else if(direccion == 2 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 18 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 19 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 26 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 5 && mapMemory[(posJugFila)*32+(posJugColumna-4)] != 42){
		mapMemory[(posJugFila)*32+(posJugColumna-4)] = 19;
		mapMemory[(posJugFila)*32+(posJugColumna-3)] = 20;
		mapMemory[(posJugFila+1)*32+(posJugColumna-4)] = 21;
		mapMemory[(posJugFila+1)*32+(posJugColumna-3)] = 22;

		mapMemory[(posJugFila)*32+(posJugColumna-2)] = 3;
		mapMemory[(posJugFila)*32+(posJugColumna-1)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna-2)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna-1)] = 3;

	}else if(direccion == 3 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 18 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 19 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 26 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 5 && mapMemory[(posJugFila)*32+(posJugColumna+4)] != 42){
		mapMemory[(posJugFila)*32+(posJugColumna+4)] = 19;
		mapMemory[(posJugFila)*32+(posJugColumna+5)] = 20;
		mapMemory[(posJugFila+1)*32+(posJugColumna+4)] = 21;
		mapMemory[(posJugFila+1)*32+(posJugColumna+5)] = 22;

		mapMemory[(posJugFila)*32+(posJugColumna+2)] = 3;
		mapMemory[(posJugFila)*32+(posJugColumna+3)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna+2)] = 3;
		mapMemory[(posJugFila+1)*32+(posJugColumna+3)] = 3;
	}
}


/**
 * Gestiona si ha llegado al final del nivel se llama a la funcion CrearDialogo para hacer las preguntas y si falla se reinicia el nivel.
 * Tambien gestiona si el nivel es reiniciado para así volver a generarlo
 */
void ConsultarSistemaDialogo(){
	REG_KEYCNT = 0x3FFF;
	switch(nivelActual){
		//Nivel 1
		case 0:
			// Crea los dialogos y comprueba si estan siendo respondidos correctamente
			if(esJuegoReiniciado == false && CrearDialogo(Pregunta1_1Bitmap,1) && CrearDialogo(Pregunta1_2Bitmap,0) && CrearDialogo(Pregunta1_3Bitmap,0)){
				nivelActual++;
				//Crea dialogo para indicarle que ha acertado y pasa de nivel(aqui da igual la respuesta)
				//Pasa al siguiente nivel
				CrearDialogo(PreguntasAcertadasBitmap,2);
				mapaAcutal = nivel2;
				GenerarNivel(nivel2,HUDBitmap,MOVIMIENTOS_NIV2);
			}else{
				//Crea dialogo para indicarle que ha fallado y tiene que reiniciar(aqui da igual la respuesta)
				//Reinicia nivel
				if(esJuegoReiniciado == false){
					CrearDialogo(PreguntaFallidaBitmap,2);
				}
				esJuegoReiniciado = false;
				GenerarNivel(nivel1,HUDBitmap,MOVIMIENTOS_NIV1);	
			}
			break;

		//Nivel 2
		case 1:
			if((esJuegoReiniciado == false && CrearDialogo(Pregunta2_1Bitmap,0)) && CrearDialogo(Pregunta2_2Bitmap,0)){
				nivelActual++;
				//Crea dialogo para indicarle que ha acertado y pasa de nivel(aqui da igual la respuesta)
				//Pasa al siguiente nivel
				CrearDialogo(PreguntasAcertadasBitmap,2);
				mapaAcutal = nivel3;
				GenerarNivel(nivel3,HUDBitmap,MOVIMIENTOS_NIV3);
			}else{
				//Crea dialogo para indicarle que ha fallado y tiene que reiniciar(aqui da igual la respuesta)
				//Reinicia nivel
				if(esJuegoReiniciado == false){
					CrearDialogo(PreguntaFallidaBitmap,2);
				}
				esJuegoReiniciado = false;
				GenerarNivel(nivel2,HUDBitmap,MOVIMIENTOS_NIV2);
			}
			break;

		//Nivel 3
		case 2:
			if((esJuegoReiniciado == false && CrearDialogo(Pregunta3_1Bitmap,0)) && CrearDialogo(Pregunta3_2Bitmap,0)){
				nivelActual++;
				//Crea dialogo para indicarle que ha acertado y pasa de nivel(aqui da igual la respuesta)
				//Pasa al siguiente nivel
				CrearDialogo(PreguntasAcertadasBitmap,2);
				mapaAcutal = nivel4;
				GenerarNivel(nivel4,HUDBitmap,MOVIMIENTOS_NIV4);
			}else{
				//Crea dialogo para indicarle que ha fallado y tiene que reiniciar(aqui da igual la respuesta)
				//Reinicia nivel
				if(esJuegoReiniciado == false){
					CrearDialogo(PreguntaFallidaBitmap,2);
				}
				esJuegoReiniciado = false;
				GenerarNivel(nivel3,HUDBitmap,MOVIMIENTOS_NIV3);
			}
			break;

		//Nivel 4
		case 3:
			if((esJuegoReiniciado == false && CrearDialogo(Pregunta4Bitmap,0))){
				nivelActual++;
				//Crea dialogo para indicarle que ha acertado y pasa de nivel(aqui da igual la respuesta)
				//Pasa al siguiente nivel
				CrearDialogo(PreguntasAcertadasBitmap,2);
				mapaAcutal = nivel5;
				GenerarNivel(nivel5,HUDBitmap,MOVIMIENTOS_NIV5);
			}else{
				//Crea dialogo para indicarle que ha fallado y tiene que reiniciar(aqui da igual la respuesta)
				//Reinicia nivel
				if(esJuegoReiniciado == false){
					CrearDialogo(PreguntaFallidaBitmap,2);
				}
				esJuegoReiniciado = false;
				GenerarNivel(nivel4,HUDBitmap,MOVIMIENTOS_NIV4);
			}
			break;

		//Nivel 5
		case 4:
			if((esJuegoReiniciado == false && CrearDialogo(Pregunta5_1Bitmap,0)) && CrearDialogo(Pregunta5_2Bitmap,1)){
				nivelActual++;
				//Crea dialogo para indicarle que ha acertado y pasa de nivel(aqui da igual la respuesta)
				//Pasa al siguiente nivel
				CrearDialogo(PreguntasAcertadasFinalBitmap,2);

				//Cinamatica final
				CrearDialogo(CinematicaFinalF1Bitmap,2);
				CrearDialogo(CinematicaFinalF2Bitmap,2);
				CrearDialogo(CinematicaFinalF3Bitmap,2);
				CrearMenu(menuTitulo,menuPrincipalBitmap,menuCreditosBitmap);
			}else{
				//Crea dialogo para indicarle que ha fallado y tiene que reiniciar(aqui da igual la respuesta)
				//Reinicia nivel
				if(esJuegoReiniciado == false){
					CrearDialogo(PreguntaFallidaBitmap,2);
				}
				esJuegoReiniciado = false;
				GenerarNivel(nivel5,HUDBitmap,MOVIMIENTOS_NIV5);
			}
			break;

		// Cinematica final por si surge algún error o sucede algo.
		default:
			CrearDialogo(CinematicaFinalF1Bitmap,2);
			CrearDialogo(CinematicaFinalF2Bitmap,2);
			CrearDialogo(CinematicaFinalF3Bitmap,2);
			CrearMenu(menuTitulo,menuPrincipalBitmap,menuCreditosBitmap);
			break;
	}
}


/**
 * Vuelve a habilitar el boton de los dialogos para así evitar que se pasen todos con tocar una vez la pantalla
 */
void HabilitarBotonesDialogo(){
	esActivoBotonesDialogos = true;
	timerPause(0);
}


/**
 * Genera un nivel.
 * @param mapa Mapa que generará
 * @param imagen Imagen que colocará en la pantalla inferior (normalmente es el HUD)
 * @param numeroMovimientosMax Numero de movimientos maximos para pasarse el nivel
 */
void GenerarNivel(u16 mapa[],unsigned int imagen[], int numeroMovimientosMax){
	//Vuelve a habilitar la actualizacion de animaciones
	timerUnpause(1);

	REG_KEYCNT = 0x7FFF;
	esActivoBotonReinicio=true;
	esPartidaAcabada = false;
	jugadorVivo=true;

	dmaCopy(imagen, VRAM_A, 256*192*2);
	REG_DISPCNT = MODE_FB0;

	movimientosJugador = numeroMovimientosMax;
	maximoMovimientosJugador = numeroMovimientosMax;

	//Crea la barra de estamina
	if(numeroMovimientosMax > 0){
		for(int lin =COMIENZO_LINEA_BARRA_ESTAMINA; lin<ALTO_BARRA_ESTAMINA; lin++){
			for(int col=COMIENZO_COLUMNA_BARRA_ESTAMINA; col<ANCHO_BARRA_ESTAMINA;col++){
				fb[lin*256+col] = RGB15(0,30,0);
			}
		}
	}

	int fila;
	int columna;
	int contEnemigos=0;
	
	numeroDeEnemigos=0;
	pos_mapData = 0;
	// Genera el nivel
	for(fila=0;fila<24;fila++)
		for(columna=0;columna<32;columna++)
		{
			pos_mapMemory            = fila*32+columna;
			mapMemory[pos_mapMemory] = mapa[pos_mapData];
			// Actualiza la posicion del jugador para las animaciones y el movimiento
			if(mapMemory[pos_mapMemory] == 0){
				posJugColumna = columna;
				posJugFila = fila;
			}else 
			// Actualiza la posicion del NPC para las animaciones
			if(mapMemory[pos_mapMemory] == 38){
				posNpcColumna = columna;
				posNpcFila = fila;
			}else 
			// Guarda la posicion del enemigo para actualizar sus animaciones
			if(mapMemory[pos_mapMemory] == 5 || mapMemory[pos_mapMemory] == 42){
				posicionesEnemigo[contEnemigos].x = columna;
				posicionesEnemigo[contEnemigos].y = fila;
				posicionesEnemigo[contEnemigos].vivo = true;
				contEnemigos++;
				numeroDeEnemigos++;
			}
			pos_mapData ++;
	    }
}

/**
 * Actualiza la barra de estamina
 */
void ActualizarBarraMovimientos(){
	int pixelesSegmento = ALTO_BARRA_ESTAMINA/maximoMovimientosJugador;

	// Recorre el registro en base al comienzo y final del segmento que es eliminado de la barra de estamina
	for(int lin = COMIENZO_LINEA_BARRA_ESTAMINA; lin < ALTO_BARRA_ESTAMINA - (ALTO_BARRA_ESTAMINA - (pixelesSegmento * (maximoMovimientosJugador-movimientosJugador)+1)); lin++){
		for(int col=COMIENZO_COLUMNA_BARRA_ESTAMINA; col<ANCHO_BARRA_ESTAMINA;col++){
			fb[lin*256+col] = RGB15(0,0,0);
		}
	}
}


/**
 * Crea el menu principal
 * @param imagenSuperior Un mapa de teselas que hará de titulo
 * @param imagenMenu Imagen que tendrá en la pantalla inferior
 * @param imagenCreditos Imagen que aparecera cuando se pulse el boton de creditos
 */
void CrearMenu(u16 imagenSuperior[],unsigned int imagenMenu[], unsigned int imagenCreditos[]){

	esPartidaAcabada = true;
 	jugadorVivo = false;
	puedeJugadorMoverse = true;
	esActivoBotonReinicio = false;
	esActivoBotonesDialogos = true;
	esAtrasBotonActivo = false;
	esJuegoComenzado = false;
	esJuegoReiniciado = false;
	esFotograma1Activo = true;

	//Pantalla para el juego
	BGCTRL_SUB [0]   = BG_32x32    | BG_COLOR_256 | BG_MAP_BASE(0) | BG_TILE_BASE(1);

	//Genera la imagen en la parte superior e inferior
	GenerarNivel(imagenSuperior,imagenMenu,0);


	u32 keys;
	while(esJuegoComenzado == false){
			//??¿?¿?Eliminar¿
			if(REG_DISPCNT == MODE_FB0){
				//lcdMainOnTop();
				//REG_BG2CNT_SUB = MODE_FB1;
			}else{
				//lcdMainOnBottom();
				REG_DISPCNT = MODE_FB0;
			}

		scanKeys();
		keys = keysCurrent();
		if(keys & KEY_TOUCH  && esActivoBotonesDialogos == true){
			touchRead(&posicionXY);
			//Si esta en el rango del boton
			if(esAtrasBotonActivo == false && (posicionXY.px >= puntosComenzaPartidaBoton[0].x && posicionXY.px <= puntosComenzaPartidaBoton[1].x) && (posicionXY.py >= puntosComenzaPartidaBoton[0].y && posicionXY.py <= puntosComenzaPartidaBoton[1].y)){			
				//Inicia cinematica
				CrearDialogo(CinematicaInicioF1Bitmap,2);
				CrearDialogo(CinematicaInicioF2Bitmap,2);
				CrearDialogo(CinematicaInicioF3Bitmap,2);
				//Genera el nivel 1
				mapaAcutal = nivel1;
				GenerarNivel(nivel1,HUDBitmap,MOVIMIENTOS_NIV1);
				esJuegoComenzado = true;
			}else if((posicionXY.px >= puntosCreditosBoton[0].x && posicionXY.px <= puntosCreditosBoton[1].x) && (posicionXY.py >= puntosCreditosBoton[0].y && posicionXY.py <= puntosCreditosBoton[1].y)){
				//Carga la imagen
				dmaCopy(imagenCreditos, VRAM_A, 256*192*2);
				REG_DISPCNT = MODE_FB0;
				esAtrasBotonActivo = true;
			}else if(esAtrasBotonActivo == true && (posicionXY.px >= puntosAtrasBoton[0].x && posicionXY.px <= puntosAtrasBoton[1].x) && (posicionXY.py >= puntosAtrasBoton[0].y && posicionXY.py <= puntosAtrasBoton[1].y)){
				//Carga la imagen
				dmaCopy(imagenMenu, VRAM_A, 256*192*2);
				REG_DISPCNT = MODE_FB0;
				esAtrasBotonActivo = false;
			}
		}
		swiWaitForVBlank();

	}
}


/**
 * Crea un dialogo con una posible respuesta correcta
 * @param imagen Imagen del dialogos con sus respectivas respuestas
 * @param opcionCorrecta La opcion correcta
 * @return Si ha coincidido la opcionCorrecta con la respuesta dada (true si ha coincidido)
 */
bool CrearDialogo(unsigned int imagen[], int opcionCorrecta){
	timerPause(1);
	esActivoBotonReinicio=false;

	dmaCopy(imagen, VRAM_A, 256*192*2);
	REG_DISPCNT = MODE_FB0;

	opcionElegida=-1;
	
	u32 keys;
	// No sale del bucle hasta que se de una respuesta
	while(opcionElegida == -1){
		scanKeys();
		keys = keysCurrent();
		if(keys & KEY_TOUCH  && esActivoBotonesDialogos == true){
			touchRead(&posicionXY);
			//Si esta en el rango del boton
			if((posicionXY.px >= puntosBoton1[0].x && posicionXY.px <= puntosBoton1[1].x) && (posicionXY.py >= puntosBoton1[0].y && posicionXY.py <= puntosBoton1[1].y)){
				opcionElegida = 0;
			}else if((posicionXY.px >= puntosBoton2[0].x && posicionXY.px <= puntosBoton2[1].x) && (posicionXY.py >= puntosBoton2[0].y && posicionXY.py <= puntosBoton2[1].y)){
				opcionElegida = 1;
			}
		}
		swiWaitForVBlank();
	}
	//Activa el timer pero no es llamado instantaneamente
	timerUnpause(0);
	// Desactiva los botones de los dialogos para evitar si hay varias llamadas consecutivas de esta funcion, no sean respondidas con un toque
	// de la pantalla tactil
	esActivoBotonesDialogos = false;

	if(opcionElegida == opcionCorrecta){
		return true;
	}else{
		return false;
	}
}


/**
 * Actualiza las animaciones de los NPC's y del jugador
 */
void ActualizarAnimacion(){
	if(esFotograma1Activo==true){
		//Animacion del NPC
		pos_mapMemory            = posNpcFila*32+posNpcColumna;
		mapMemory[pos_mapMemory] = 38;

		pos_mapMemory            = posNpcFila*32+(posNpcColumna+1);
		mapMemory[pos_mapMemory] = 39;

		pos_mapMemory            = (posNpcFila+1)*32+posNpcColumna;
		mapMemory[pos_mapMemory] = 40;

		pos_mapMemory            = (posNpcFila+1)*32+(posNpcColumna+1);
		mapMemory[pos_mapMemory] = 41;

		// Pasa por todos los enemigos que hay en el mapa y comprueba si estan eliminado o no.
		// Si no lo estan actualizan su animacion
		for(int i =0;i<numeroDeEnemigos;i++){
			if(posicionesEnemigo[i].vivo==true){
				pos_mapMemory            = (posicionesEnemigo[i].y)*32+(posicionesEnemigo[i].x);
				mapMemory[pos_mapMemory] = 5;

				pos_mapMemory            = (posicionesEnemigo[i].y)*32+(posicionesEnemigo[i].x+1);
				mapMemory[pos_mapMemory] = 14;

				pos_mapMemory            = (posicionesEnemigo[i].y+1)*32+(posicionesEnemigo[i].x);
				mapMemory[pos_mapMemory] = 15;
				
				pos_mapMemory            = (posicionesEnemigo[i].y+1)*32+(posicionesEnemigo[i].x+1);
				mapMemory[pos_mapMemory] = 16;
			}
		}
		esFotograma1Activo = false;
	}else{
		// Animacion del NPC
		pos_mapMemory            = posNpcFila*32+posNpcColumna;
		mapMemory[pos_mapMemory] = 30;

		pos_mapMemory            = posNpcFila*32+(posNpcColumna+1);
		mapMemory[pos_mapMemory] = 31;

		pos_mapMemory            = (posNpcFila+1)*32+posNpcColumna;
		mapMemory[pos_mapMemory] = 32;

		pos_mapMemory            = (posNpcFila+1)*32+(posNpcColumna+1);
		mapMemory[pos_mapMemory] = 33;

		for(int i =0;i<numeroDeEnemigos;i++){
			if(posicionesEnemigo[i].vivo==true){
				pos_mapMemory            = (posicionesEnemigo[i].y)*32+(posicionesEnemigo[i].x);
				mapMemory[pos_mapMemory] = 42;

				pos_mapMemory            = (posicionesEnemigo[i].y)*32+(posicionesEnemigo[i].x+1);
				mapMemory[pos_mapMemory] = 43;

				pos_mapMemory            = (posicionesEnemigo[i].y+1)*32+(posicionesEnemigo[i].x);
				mapMemory[pos_mapMemory] = 44;
				
				pos_mapMemory            = (posicionesEnemigo[i].y+1)*32+(posicionesEnemigo[i].x+1);
				mapMemory[pos_mapMemory] = 45;
			}
		}

		esFotograma1Activo = true;
	}

	// Animacion Jugador
	pos_mapMemory            = posJugFila*32+posJugColumna;
	mapMemory[pos_mapMemory] = ElegirFondoJugador(0,pos_mapMemory);

	pos_mapMemory            = posJugFila*32+(posJugColumna+1);
	mapMemory[pos_mapMemory] = ElegirFondoJugador(1,pos_mapMemory);

	pos_mapMemory            = (posJugFila+1)*32+posJugColumna;
	mapMemory[pos_mapMemory] = ElegirFondoJugador(2,pos_mapMemory);

	pos_mapMemory            = (posJugFila+1)*32+(posJugColumna+1);
	mapMemory[pos_mapMemory] = ElegirFondoJugador(3,pos_mapMemory);
}


/**
 * Inicializa las teselas
 */
void InicializarTeselas(){
	//Teselas
	dmaCopy(t_jugadorF1Parte1,      		tileMemory ,      			sizeof(t_jugadorF1Parte1));//0
	dmaCopy(t_jugadorF1Parte2,      		tileMemory+(64*8) ,     	sizeof(t_jugadorF1Parte2));//8
	dmaCopy(t_jugadorF1Parte3,      		tileMemory+(64*9) ,      	sizeof(t_jugadorF1Parte3));//9
	dmaCopy(t_jugadorF1Parte4,      		tileMemory+(64*10),      	sizeof(t_jugadorF1Parte4));//10

	dmaCopy(t_jugadorFondoVerdeParte1,      tileMemory+(64*6) ,      	sizeof(t_jugadorFondoVerdeParte1));//6
	dmaCopy(t_jugadorFondoVerdeParte2,      tileMemory+(64*11) ,     	sizeof(t_jugadorFondoVerdeParte2));//11
	dmaCopy(t_jugadorFondoVerdeParte3,      tileMemory+(64*12) ,      	sizeof(t_jugadorFondoVerdeParte3));//12
	dmaCopy(t_jugadorFondoVerdeParte4,      tileMemory+(64*13) ,      	sizeof(t_jugadorFondoVerdeParte4));//13

	dmaCopy(t_muroIzqParte1,      			tileMemory+(64*7) ,      	sizeof(t_muroIzqParte1));//7
	dmaCopy(t_muroIzqParte2,      			tileMemory+(64*17),      	sizeof(t_muroIzqParte2));//17

	dmaCopy(t_muroDerParte1,     			tileMemory + (64*24), 		sizeof(t_muroDerParte1));//24
	dmaCopy(t_muroDerParte2,     			tileMemory + (64*25), 		sizeof(t_muroDerParte2));//25


	dmaCopy(t_muroParte1,     				tileMemory + (64*26), 		sizeof(t_muroParte1));//26
	dmaCopy(t_muroParte2,     				tileMemory + (64*27), 		sizeof(t_muroParte2));//27
	dmaCopy(t_muroParte3,     				tileMemory + (64*28), 		sizeof(t_muroParte3));//28
	dmaCopy(t_muroParte4,     				tileMemory + (64*29), 		sizeof(t_muroParte4));//29

	dmaCopy(t_salida,    					tileMemory + (64*1),  		sizeof(t_salida));//1
	dmaCopy(t_meta,      					tileMemory + (64*2), 		sizeof(t_meta));//2
	dmaCopy(t_hierba, 						tileMemory + (64*3), 		sizeof(t_hierba));//3
	dmaCopy(t_mitad,     					tileMemory + (64*4), 		sizeof(t_mitad));//4

	dmaCopy(t_enemigoF1Parte1,     			tileMemory + (64*5), 		sizeof(t_enemigoF1Parte1));//5
	dmaCopy(t_enemigoF1Parte2,      		tileMemory+(64*14) ,      	sizeof(t_enemigoF1Parte2));//14
	dmaCopy(t_enemigoF1Parte3,      		tileMemory+(64*15) ,      	sizeof(t_enemigoF1Parte3));//15
	dmaCopy(t_enemigoF1Parte4,      		tileMemory+(64*16) ,      	sizeof(t_enemigoF1Parte4));//16

	dmaCopy(t_vacio,     					tileMemory + (64*18), 		sizeof(t_vacio));//18

	dmaCopy(t_cajaParte1,     				tileMemory + (64*19), 		sizeof(t_cajaParte1));//19
	dmaCopy(t_cajaParte2,     				tileMemory + (64*20), 		sizeof(t_cajaParte2));//20
	dmaCopy(t_cajaParte3,     				tileMemory + (64*21), 		sizeof(t_cajaParte3));//21
	dmaCopy(t_cajaParte4,     				tileMemory + (64*22), 		sizeof(t_cajaParte4));//22

	dmaCopy(t_dialogoColision,     			tileMemory + (64*23), 		sizeof(t_dialogoColision));//23

	dmaCopy(t_npcParte1,     				tileMemory + (64*30), 		sizeof(t_npcParte1));//30
	dmaCopy(t_npcParte2,     				tileMemory + (64*31), 		sizeof(t_npcParte2));//31
	dmaCopy(t_npcParte3,     				tileMemory + (64*32), 		sizeof(t_npcParte3));//32
	dmaCopy(t_npcParte4,     				tileMemory + (64*33), 		sizeof(t_npcParte4));//33

	dmaCopy(t_jugadorF2Parte1,     			tileMemory + (64*34), 		sizeof(t_jugadorF2Parte1));//34
	dmaCopy(t_jugadorF2Parte2,    			tileMemory + (64*35), 		sizeof(t_jugadorF2Parte2));//35
	dmaCopy(t_jugadorF2Parte3,     			tileMemory + (64*36), 		sizeof(t_jugadorF2Parte3));//36
	dmaCopy(t_jugadorF2Parte4,     			tileMemory + (64*37), 		sizeof(t_jugadorF2Parte4));//37
	
	dmaCopy(t_npcF2Parte1,     				tileMemory + (64*38), 		sizeof(t_npcF2Parte1));//38
	dmaCopy(t_npcF2Parte2,     				tileMemory + (64*39), 		sizeof(t_npcF2Parte2));//39
	dmaCopy(t_npcF2Parte3,     				tileMemory + (64*40), 		sizeof(t_npcF2Parte3));//40
	dmaCopy(t_npcF2Parte4,     				tileMemory + (64*41), 		sizeof(t_npcF2Parte4));//41

	dmaCopy(t_enemigoF2Parte1,     			tileMemory + (64*42), 		sizeof(t_enemigoF2Parte1));//42
	dmaCopy(t_enemigoF2Parte2,     			tileMemory + (64*43), 		sizeof(t_enemigoF2Parte2));//43
	dmaCopy(t_enemigoF2Parte3,     			tileMemory + (64*44), 		sizeof(t_enemigoF2Parte3));//44
	dmaCopy(t_enemigoF2Parte4,     			tileMemory + (64*45), 		sizeof(t_enemigoF2Parte4));//45



	//Titulo del menu en la pantalla superior-----------------------------------
	dmaCopy(t_menuTitulo0,     				tileMemory + (64*46), 		sizeof(t_menuTitulo0));//46
	dmaCopy(t_menuTitulo1,     				tileMemory + (64*47), 		sizeof(t_menuTitulo1));//47
	dmaCopy(t_menuTitulo2,     				tileMemory + (64*48), 		sizeof(t_menuTitulo2));//48
	dmaCopy(t_menuTitulo3,     				tileMemory + (64*49), 		sizeof(t_menuTitulo3));//49
	dmaCopy(t_menuTitulo4,     				tileMemory + (64*50), 		sizeof(t_menuTitulo4));//50
	dmaCopy(t_menuTitulo5,     				tileMemory + (64*51), 		sizeof(t_menuTitulo5));//51
	dmaCopy(t_menuTitulo6,     				tileMemory + (64*52), 		sizeof(t_menuTitulo6));//52

	dmaCopy(t_menuTitulo7,     				tileMemory + (64*53), 		sizeof(t_menuTitulo7));//53
	dmaCopy(t_menuTitulo8,     				tileMemory + (64*54), 		sizeof(t_menuTitulo8));//54
	dmaCopy(t_menuTitulo9,     				tileMemory + (64*55), 		sizeof(t_menuTitulo9));//55
	dmaCopy(t_menuTitulo10,     			tileMemory + (64*56), 		sizeof(t_menuTitulo10));//56
	dmaCopy(t_menuTitulo11,     			tileMemory + (64*57), 		sizeof(t_menuTitulo11));//57
	dmaCopy(t_menuTitulo12,     			tileMemory + (64*58), 		sizeof(t_menuTitulo12));//58
	dmaCopy(t_menuTitulo13,     			tileMemory + (64*59), 		sizeof(t_menuTitulo13));//56
	dmaCopy(t_menuTitulo14,     			tileMemory + (64*60), 		sizeof(t_menuTitulo14));//60

	dmaCopy(t_menuTitulo15,     			tileMemory + (64*61), 		sizeof(t_menuTitulo15));//61
	dmaCopy(t_menuTitulo16,     			tileMemory + (64*62), 		sizeof(t_menuTitulo16));//62
	dmaCopy(t_menuTitulo17,     			tileMemory + (64*63), 		sizeof(t_menuTitulo17));//63
	dmaCopy(t_menuTitulo18,     			tileMemory + (64*64), 		sizeof(t_menuTitulo18));//64
	dmaCopy(t_menuTitulo19,     			tileMemory + (64*65), 		sizeof(t_menuTitulo19));//65
	dmaCopy(t_menuTitulo20,     			tileMemory + (64*66), 		sizeof(t_menuTitulo20));//66
	dmaCopy(t_menuTitulo21,     			tileMemory + (64*67), 		sizeof(t_menuTitulo21));//67

	dmaCopy(t_menuTitulo22,     			tileMemory + (64*68), 		sizeof(t_menuTitulo22));//68
	dmaCopy(t_menuTitulo23,     			tileMemory + (64*69), 		sizeof(t_menuTitulo23));//69
	dmaCopy(t_menuTitulo24,     			tileMemory + (64*70), 		sizeof(t_menuTitulo24));//70
	dmaCopy(t_menuTitulo25,     			tileMemory + (64*71),		sizeof(t_menuTitulo25));//71
	dmaCopy(t_menuTitulo26,     			tileMemory + (64*72), 		sizeof(t_menuTitulo26));//72
	dmaCopy(t_menuTitulo27,     			tileMemory + (64*73), 		sizeof(t_menuTitulo27));//73
	dmaCopy(t_menuTitulo28,     			tileMemory + (64*74), 		sizeof(t_menuTitulo28));//74

	dmaCopy(t_menuTitulo29,     			tileMemory + (64*75), 		sizeof(t_menuTitulo29));//75
	dmaCopy(t_menuTitulo30,     			tileMemory + (64*76), 		sizeof(t_menuTitulo30));//76
	dmaCopy(t_menuTitulo31,     			tileMemory + (64*77), 		sizeof(t_menuTitulo31));//77
	dmaCopy(t_menuTitulo32,     			tileMemory + (64*78), 		sizeof(t_menuTitulo32));//78
	dmaCopy(t_menuTitulo33,     			tileMemory + (64*79), 		sizeof(t_menuTitulo33));//79
	dmaCopy(t_menuTitulo34,     			tileMemory + (64*80), 		sizeof(t_menuTitulo34));//80
	dmaCopy(t_menuTitulo35,     			tileMemory + (64*81), 		sizeof(t_menuTitulo35));//81

	dmaCopy(t_menuTitulo36,     			tileMemory + (64*82), 		sizeof(t_menuTitulo36));//82
	dmaCopy(t_menuTitulo37,     			tileMemory + (64*83), 		sizeof(t_menuTitulo37));//83
	dmaCopy(t_menuTitulo38,     			tileMemory + (64*84), 		sizeof(t_menuTitulo38));//84
	dmaCopy(t_menuTitulo39,     			tileMemory + (64*85), 		sizeof(t_menuTitulo39));//85
	dmaCopy(t_menuTitulo40,     			tileMemory + (64*86), 		sizeof(t_menuTitulo40));//86
	dmaCopy(t_menuTitulo41,     			tileMemory + (64*87), 		sizeof(t_menuTitulo41));//87
	dmaCopy(t_menuTitulo42,     			tileMemory + (64*88), 		sizeof(t_menuTitulo42));//88
	dmaCopy(t_menuTitulo43,     			tileMemory + (64*89), 		sizeof(t_menuTitulo43));//89

	dmaCopy(t_menuTitulo44,    				tileMemory + (64*90), 		sizeof(t_menuTitulo44));//90
	dmaCopy(t_menuTitulo45,     			tileMemory + (64*91), 		sizeof(t_menuTitulo45));//91
	dmaCopy(t_menuTitulo46,     			tileMemory + (64*92), 		sizeof(t_menuTitulo46));//92
	dmaCopy(t_menuTitulo47,     			tileMemory + (64*93), 		sizeof(t_menuTitulo47));//93
}