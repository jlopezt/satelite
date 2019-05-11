/**********************************************/
/*                                            */
/*  Gestion de la conexion MQTT               */
/*  Incluye la conexion al bus y la           */
/*  definicion del callback de suscripcion    */
/*                                            */
/* Librria de sooprte del protocolo MQTT      */
/* para arduino/ESP8266/ESP32                 */
/*                                            */
/* https://pubsubclient.knolleary.net/api.html*/
/**********************************************/
//Includes MQTT
//#define MQTT_KEEPALIVE 60
#include <PubSubClient.h>
#define CLEAN_SESSION TRUE

//definicion de los comodines del MQTT
#define WILDCARD_ALL      "#"
#define WILDCARD_ONELEVEL "+"

//definicion de constantes para WILL
#define WILL_TOPIC  "will"
#define WILL_QOS    1
#define WILL_RETAIN false
#define WILL_MSG    String("¡"+ID_MQTT+" caido!").c_str()

//Definicion de variables globales
IPAddress IPBroker; //IP del bus MQTT
uint16_t puertoBroker; //Puerto del bus MQTT
uint16_t timeReconnectMQTT; //Tiempo de espera en la reconexion al bus
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String ID_MQTT; //ID del modulo en su conexion al broker
int8_t publicarEstado; //Flag para determinar si se envia el json con los valores de las salidas

WiFiClient espClient;
PubSubClient clienteMQTT(espClient);

/************************************************/
/* Inicializa valiables y estado del bus MQTT   */
/************************************************/
void inicializaMQTT(void)
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosMQTT(false)) Serial.printf("error al recuperar config MQTT.\nConfiguracion por defecto.\n");

  //Si va bien inicializo con los valores correstoc, si no con valores por defecto
  //confituro el servidor y el puerto
  clienteMQTT.setServer(IPBroker, puertoBroker);
  //configuro el callback, si lo hay
  clienteMQTT.setCallback(callbackMQTT);

  if (conectaMQTT()) Serial.println("connectado al broker");  
  else Serial.printf("error al conectar al broker con estado %i\n",clienteMQTT.state());
  }

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de MQTT                          */
/************************************************/
boolean recuperaDatosMQTT(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  IPBroker.fromString("0.0.0.0");
  puertoBroker=0;
  timeReconnectMQTT=100;
  ID_MQTT="Modulo_" + String(direccion); //Este valor no se lee del fichero, ID del modulo en su conexion al broker
  usuarioMQTT="";
  passwordMQTT="";
  topicRoot="";
  publicarEstado=1;//por defecto publico

  if(leeFichero(MQTT_CONFIG_FILE, cad))
    if(parseaConfiguracionMQTT(cad)) 
      return true;

  //Algo salio mal, confgiguracion por defecto
  Serial.printf("No existe fichero de configuracion MQTT o esta corrupto\n");
  cad="{\"IPBroker\": \"10.68.1.100\", \"puerto\": 1883, \"timeReconnectMQTT\": 500, \"usuarioMQTT\": \"usuario\", \"passwordMQTT\": \"password\", \"topicRoot\": \"casa\", \"publicarEstado\": 1}";
  salvaFichero(MQTT_CONFIG_FILE, MQTT_CONFIG_BAK_FILE, cad);
  Serial.printf("Fichero de configuracion MQTT creado por defecto\n");
  parseaConfiguracionMQTT(cad);
    
  return false;
  }  

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio MQTT                         */
/*********************************************/
boolean parseaConfiguracionMQTT(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    IPBroker.fromString(json.get<String>("IPBroker"));
    puertoBroker=json.get<uint16_t>("puerto");
    timeReconnectMQTT=json.get<uint16_t>("timeReconnectMQTT");
    usuarioMQTT=json.get<String>("usuarioMQTT");
    passwordMQTT=json.get<String>("passwordMQTT");
    topicRoot=json.get<String>("topicRoot");
    publicarEstado=json.get<int8_t>("publicarEstado");

    Serial.printf("Configuracion leida:\nIP broker: %s\nIP Puerto del broker: %i\ntimeReconnectMQTT: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nPublicar estado: %i\n",IPBroker.toString().c_str(),puertoBroker,timeReconnectMQTT, usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),publicarEstado);
//************************************************************************************************
    return true;
    }
  return false;
  }

/***********************************************Funciones de gestion de mensajes MQTT**************************************************************/
/***************************************************/
/* Funcion que recibe el mensaje cuando se publica */
/* en el bus un topic al que esta subscrito        */
/***************************************************/
void callbackMQTT(char* topic, byte* payload, unsigned int length)
  {
  //AQUI NO HAY CALLBACK!!! No recibo nada
  if(debugGlobal) Serial.printf("Entrando en callback: \n Topic: %s\nPayload %s\nLongitud %i\n", topic, payload, length);
  
  /**********compruebo el topic*****************/
  //Identifica si el topic del mensaje es uno de los suscritos (deberia ser siempre que si)
  //Compara el topic recibido con los que tiene suscritos para redirigir a la funcion de gestion correspondiente  
  String cad=String(topic);
/*
  //Para cada topic suscrito...
  if(cad.equalsIgnoreCase(topicRoot + <topicSuscrito>)) <funcion de gestion>(topic,payload,length);  
  //elseif(cad.equalsIgnoreCase(topicRoot + <topicSuscrito>)) <funcion de gestion>(topic,payload,length);  
  //Si no machea el topic recibido con los sucritos lo tira (no deberia ocurrir)
  else Serial.printf("topic no reconocido: \ntopic: %s\nroot: %s\n", cad.c_str(),cad.substring(0,cad.indexOf("/")).c_str());   
*/  
  }

/********************************************/
/* Funcion que gestiona la conexion al bus  */
/* MQTT del broker                          */
/********************************************/
boolean conectaMQTT(void)  
  {
  int8_t intentos=0;
  String topic;
  
  while (!clienteMQTT.connected()) 
    {    
    if(debugGlobal) Serial.println("No conectado, intentando conectar.");
  
    // Attempt to connect
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);    
    if (clienteMQTT.connect(ID_MQTT.c_str(), usuarioMQTT.c_str(), passwordMQTT.c_str(), (topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN, String(WILL_MSG).c_str(), CLEAN_SESSION))
      {
      if(debugGlobal) Serial.println("conectado");
/*
      //Suscripcion a todos los topics que aplican a este componente
      //topicOrdenes: topic en el que el controlador publica las ordenes
      topic=topicRoot + <topic suscrito>;
      if (clienteMQTT.subscribe(topic.c_str())) Serial.printf("Subscrito al topic %s\n", topic.c_str());
      else Serial.printf("Error al subscribirse al topic %s\n", topic.c_str());       
*/
      return(true);
      }

    if(debugGlobal) Serial.printf("Error al conectar al broker. Estado: %s\n",stateTexto().c_str());
    if(intentos++>3) return (false);
    delay(timeReconnectMQTT);      
    }
  }

/********************************************/
/* Funcion que envia un mensaje al bus      */
/* MQTT del broker                          */
/* Eliminado limite del buffer de envio     */
/********************************************/
boolean enviarMQTT(String topic, String payload)
  {
  //si no esta conectado, conecto
  if (!clienteMQTT.connected()) conectaMQTT();

  //si y esta conectado envio, sino salgo con error
  if (clienteMQTT.connected()) 
    {
    if(!topic.startsWith("/")) topic = "/" + topic;  
    topic=topicRoot + topic;

	//Serial.printf("Enviando:\ntopic:  %s | payload: (%i) %s\n",topic.c_str(),payload.length(),payload.c_str());
  
    if(clienteMQTT.beginPublish(topic.c_str(), payload.length(), false))//boolean beginPublish(const char* topic, unsigned int plength, boolean retained)
      {
      for(uint16_t i=0;i<payload.length();i++) clienteMQTT.write((uint8_t)payload.charAt(i));//virtual size_t write(uint8_t);
      return(clienteMQTT.endPublish()); //int endPublish();
      }
    }
  else return (false);
  }

/********************************************/
/* Funcion que revisa el estado del bus y   */
/* si se ha recibido un mensaje             */
/********************************************/
void atiendeMQTT(void)
  {
  clienteMQTT.loop();
  }

/********************************************/
/*                                          */
/* Funcion que envia datos de estado del    */
/* controlador al broker                    */
/*                                          */
/********************************************/
void enviaDatos(boolean debug)
  {
  String topic= "/" + nombres[direccion] + "/medidas";
  String payload;

  if(publicarEstado==1)
    {
    payload=generaJsonEstado();//genero el json de estado
    
    //Lo envio al bus    
    if(enviarMQTT(topic, payload)) if(debug)Serial.println("Enviado json al broker con exito.");
    else if(debug)Serial.println("¡¡Error al enviar json al broker!!");
    }
  else if(debugGlobal) Serial.printf("No publico estado. Publicar estado es %i\n",publicarEstado);  
  }
  
/******************************* UTILIDADES *************************************/
/********************************************/
/* Funcion que devuleve el estado           */
/* de conexion MQTT al bus                  */
/********************************************/
String stateTexto(void)  
  {
  int r = clienteMQTT.state();

  String cad=String(r) + " : ";
  
  switch (r)
    {
    case -4:
      cad += "MQTT_CONNECTION_TIMEOUT";
      break;
    case -3:
      cad += "MQTT_CONNECTION_LOST";
      break;
    case -2:
      cad += "MQTT_CONNECT_FAILED";
      break;
    case -1:
      cad += "MQTT_DISCONNECTED";
      break;
    case  0:
      cad += "MQTT_CONNECTED";
      break;
    case  1:
      cad += "MQTT_CONNECT_BAD_PROTOCOL";
      break;
    case  2:
      cad += "MQTT_CONNECT_BAD_CLIENT_ID";
      break;
    case  3:
      cad += "MQTT_CONNECT_UNAVAILABLE";
      break;
    case  4:
      cad += "MQTT_CONNECT_BAD_CREDENTIALS";
      break;     
    case  5:
      cad += "MQTT_CONNECT_UNAUTHORIZED";
      break;
    default:
      cad += "????";
    }
      
  return (cad);
  }
