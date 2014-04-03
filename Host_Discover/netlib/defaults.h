/**
  @file config.h
  @brief Definiciones Generales

  @author Alvaro Parres
  @date Feb/2013

*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

#define BUFFERSIZE					1048576

/**
  * @defgroup Configs Valores de Configuracion
  * @{
*/

#define	CONFIG_LISENT_IFACE			"0.0.0.0" /*< Interfaz para escuchar */
#define CONFIG_DEST_IFACE			"0.0.0.0" /*< Interfaz para destino */
#define CONFIG_BDCAST_IFACE			"255.255.255.255" /*< Interfaz para bdcast */
#define CONFIG_MAX_CLIENT			5		/*< Maximo de Conexiones Simultaneas */
#define CONFIG_DEFAULT_TCPPORT		8080	/*< Puerto de trabajo TCP */
#define CONFIG_DEFAULT_UDPPORT		8081	/*< Puerto de trabajo UDP */

#define	CONFIG_DEFAULT_COUNT		5		/*< Numero de Mensajes a enviar */
#define CONFIG_DEFAULT_MSGSIZE		64		/*< Tamaño de Mensaje en KB  */

#define CONFIG_DEFAULT_VERBOSE		1		/*< Nivel de Verbosity  */

/** @}*/


/**
  * @defgroup Constante Constantes
  * @{
*/

#define MINPORT						1025	/*< Menor puerto a Utilizar */
#define MAXPORT						USHRT_MAX /*< Mayor puerto a Utilizar */
/** @}*/

#define TCPLISTENER					0
#define TCPSENDER					1
#define UDPLISTENER					2
#define UDPSENDER					3

#endif
