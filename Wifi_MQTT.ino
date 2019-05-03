/***********************************************
 * Librria de sooprte del protocolo MQTT       *
 * para arduino/ESP8266/ESP32                  *
 *                                             *
 * https://pubsubclient.knolleary.net/api.html *
 ***********************************************/
#include <PubSubClient.h>

#define WILL_TOPIC  "will/" + nombres[direccion]
#define WILL_QOS    1
#define WILL_RETAIN false
#define WILL_MSG    "vueltas: " + vuelta

IPAddress IPBroker; //IP del bus MQTT
uint16_t puertoBroker; //Puerto del bus MQTT
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String ID_MQTT="Modulo_" + String(direccion) + "_" + nombres[direccion]; //ID del modulo en su conexion al broker

WiFiClient espClient;
PubSubClient clienteMQTT(espClient);

void inicializaMQTT(void)
  {
  //recupero datos del fichero de configuracion
  recuperaDatosMQTT(false);

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
  usuarioMQTT="";
  passwordMQTT="";
  topicRoot="";
    
  if(leeFichero(MQTT_CONFIG_FILE, cad)) parseaConfiguracionMQTT(cad);

  return true;
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
    IPBroker.fromString((const char *)json["IPBroker"]);
    puertoBroker = atoi(json["puerto"]); 
    usuarioMQTT=((const char *)json["usuarioMQTT"]);
    passwordMQTT=((const char *)json["passwordMQTT"]);
    topicRoot=((const char *)json["topicRoot"]);
    Serial.printf("Configuracion leida:\nIP broker: %s\nIP Puerto del broker: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\n",IPBroker.toString().c_str(),puertoBroker,usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str());
//************************************************************************************************
    }
  }


/***********************************************Funciones de gestion de mensajes MQTT**************************************************************/
/***************************************************/
/* Funcion que recibe el mensaje cuando se publica */
/* en el bus un topic al que esta subscrito        */
/***************************************************/
void callbackMQTT(char* topic, byte* payload, unsigned int length)
  {
  //AQUI NO HAY CALLBACK!!! No recibo nada
  }

/********************************************/
/* Funcion que gestiona la conexion al bus  */
/* MQTT del broker                          */
/********************************************/
boolean conectaMQTT(void)  
  {
  int8_t intentos=0;
  
  while (!clienteMQTT.connected()) 
    {    
    if(debugGlobal) Serial.println("No conectado, intentando conectar.");
  
    // Attempt to connect
    //boolean connect (clientID, willTopic, willQoS, willRetain, willMessage)
    if (clienteMQTT.connect(ID_MQTT.c_str(),String(WILL_TOPIC).c_str(),WILL_QOS,WILL_RETAIN,String(WILL_MSG).c_str()))
      {
      if(debugGlobal) Serial.println("conectado");
      return(true);
      }

    if(intentos++>3) return (false);
    
    if(debugGlobal) Serial.println("Error al conectar al broker...");
    delay(500);      
    }
  }

/********************************************/
/* Funcion que envia un mensaje al bus      */
/* MQTT del broker                          */
/********************************************/
boolean enviarMQTT(String topic, String payload)
  {
  //si no esta conectado, conecto
  if (!clienteMQTT.connected()) conectaMQTT();

  //si y esta conectado envio, sino salgo con error
  if (clienteMQTT.connected()) 
    {
    //Serial.printf("Enviando:\ntopic:  %s | payload: %s\n",topic.c_str(),payload.c_str());
    return (clienteMQTT.publish(topic.c_str(), payload.c_str()));      
    }
  else return (false);
  }

/********************************************/
/* Funcion que revisa el estado del bus y   */
/* si se ha recibido un mensaje             */
/********************************************/
void atiendeMQTT(boolean debug)
  {
  String topic=topicRoot+ "/" + nombres[direccion] + "/medidas";
  String payload=generaJson();

  if(enviarMQTT(topic, payload)) if(debug)Serial.println("Enviado json al broker con exito.");
  else if(debug)Serial.println("¡¡Error al enviar json al broker!!");

  clienteMQTT.loop();
  }  

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
