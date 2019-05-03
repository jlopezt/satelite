/************************************************************************************************
Servicio                                    URL                            Formato entrada Formato salida                       Comentario                                                                                       Ejemplo peticion                    Ejemplo respuesta
Servicio de informacion                     http://IPSatelite              N/A             N/A                                  Nombre del modulo y version                                                  
Servicio de consulta de valores temperatura http://IPSatelite/temperatura  id=<id>         <temperatura>,<id2>                  Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/temperatura?id=59 23.5|43682
Servicio de consulta de valores humedad     http://IPSatelite/humedad      id=<id>         <humedad>,<id2>                      Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/humedad?id=59     46.2|43682
Servicio de consulta de valores luz         http://IPSatelite/luz          id=<id>         <luz>,<id2>                          Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/luz?id=59         97.3|43682
Servicio de consulta de valores medidos     http://IPSatelite/medidas      id=<id>         <temperatura>|<humedad>|<luz>|<id2>  Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/medidas?id=59     23.5|46.2|97.3|43682
Web de valores medidos                      http://IPSatelite/web          N/A             HTML                                 Devuelve una pagina web para la consulta desde navegador                                         http://IPSatelite/web  
Servicio de test                            http://IPSatelite/test         N/A             HTML                                 Devuelve un texto html para verificar conectividad desde navegador                               http://IPSatelite/test 
************************************************************************************************/
//enum HTTPMethod { HTTP_ANY, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS };
#define IDENTIFICACION "Modulo " + String(direccion) + " Habitacion= " + nombres[direccion] + "<BR>"

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(PUERTO_WEBSERVER);

String cabeceraHTML="<HTML><HEAD><TITLE>Control " + nombre_dispoisitivo + " </TITLE></HEAD><BODY><h1>Control " + nombre_dispoisitivo + "<br></h1>";
String pieHTML="</BODY></HTML>";

void handleRoot() 
  {
  String cad="";

  //genero la respuesta por defecto
  cad="Modulo satelite. Version ";
  cad += VERSION;
  cad += "<BR>";
  cad += IDENTIFICACION;
  cad += "vueltas= " + String(vuelta) + " / " + String(UINT16_MAX);
  server.send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*  Servicio de consulta de valores medidos  */
/*                                           */
/*********************************************/
void handleMedidas() 
  {
  String cad="";
  
  //genero la respuesta OK
  cad = String(getTemperatura(),1);
  cad += SEPARADOR;
  cad += String(getHumedad(),1);
  cad += SEPARADOR;
  cad += String(getLuz(),1);
  cad += SEPARADOR;

  server.send(200, "text/plain", cad);  
   
  //Serial.println("Medidas requeridas ok");
  
  registraPolling();//Apunto la hora del ultimo polling
  }

/*********************************************/
/*                                           */
/*  Medidas                                  */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Temperatura\":23.2,\"Humedad\":45.5,\"Luz\":14}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Temperatura"]; // 23.2
 *  float Humedad = root["Humedad"]; // 45.5
 *  int Luz = root["Luz"]; // 14
 */
/*********************************************/
void handleMedida() 
  {
  String cad=generaJson();
  
  server.send(200, "application/json", cad);   
   
  //Serial.println("Medidas requeridas ok");
    
  registraPolling();//Apunto la hora del ultimo polling
  }

/*********************************************/
/*                                           */
/*  Medida de temperatura                    */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Temperatura\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Temperatura"]; // 23.1
*/    
/*********************************************/
void handleTemperatura() 
  {
  String cad="";

  //genero la respuesta OK --> {"Temperatura": 22.5}
  cad  = "{\"Temperatura\": ";
  cad += String(getTemperatura(),1);
  cad += "}";
  
  server.send(200, "application/json", cad);    
  }

/*********************************************/
/*                                           */
/*  Medida de humedad                    */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *   const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Humedad\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Humedad"]; // 23.1
*/  
/*********************************************/
void handleHumedad() 
  {
  String cad="";
  
  //genero la respuesta OK --> {"Humedad": 22.5}
  cad  = "{\"Humedad\": ";
  cad += String(getHumedad(),1);
  cad += "}";
  
  server.send(200, "application/json", cad);    
  }

/*********************************************/
/*                                           */
/*  Medida de Luz                            */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *   const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Luz\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Luz"]; // 23.1
*/  
/*********************************************/
void handleLuz() 
  {
  String cad="";
  
  //genero la respuesta OK --> {"Luz": 22.5}
  cad  = "{\"Luz\": ";
  cad += String(getLuz(),1);
  cad += "}";
  
  server.send(200, "application/json", cad);    
  }

/*********************************************/
/*                                           */
/*    Web de consulta de valores medidos     */
/*                                           */
/*********************************************/
void handleWeb() 
  {
  String cad="";

  cad  = cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];
  cad += "<h1>Temperatura: ";
  cad += String(getTemperatura(),1);
  cad += "&deg; </h1>";
  cad += "<h1>Humedad: ";
  cad += String(getHumedad(),1);
  cad += "% </h1>";
  cad += "<h1>Luz: ";
  cad += String(getLuz(),1);
  cad += "</h1>";
  
  cad += pieHTML;
  
  server.send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*  Servicio de test                         */
/*                                           */
/*********************************************/  
void handleTest(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];
  
  cad += "Test OK<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Resetea el dispositivo mediante          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleReset(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];
  
  cad += "Reseteando...<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad);
  delay(100);     
  ESP.reset();
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION;
  
  cad += "Reiniciando...<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad);     
  delay(100);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];

  cad += "<BR>-----------------WiFi info-----------------<BR>";
  cad += "SSID: " + nombreSSID();
  cad += "<BR>";    
  cad += "IP: " + WiFi.localIP().toString();
  cad += "<BR>";    
  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";  
/*
  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + IPBroker.toString();
  cad += "<BR>";
  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "<BR>";  
  cad += "Usuario: " + usuarioMQTT="";
  cad += "<BR>";  
  cad += "Password: " + passwordMQTT="";
  cad += "<BR>";  
  cad += "Topic root: " + topicRoot="";
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
*/    
  cad += "<BR>-----------------Hardware info-----------------<BR>";
  cad += "Vcc: " + String(ESP.getVcc());
  cad += "<BR>";  
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
  cad += "ChipId: " + String(ESP.getChipId());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CoreVersion: " + ESP.getCoreVersion();
  cad += "<BR>";  
  cad += "FullVersion: " + ESP.getFullVersion();
  cad += "<BR>";  
  cad += "BootVersion: " + String(ESP.getBootVersion());
  cad += "<BR>";  
  cad += "BootMode: " + String(ESP.getBootMode());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
  cad += "FlashChipId: " + String(ESP.getFlashChipId());
  cad += "<BR>";  
     //gets the actual chip size based on the flash id
  cad += "FlashChipRealSize: " + String(ESP.getFlashChipRealSize());
  cad += "<BR>";  
     //gets the size of the flash as set by the compiler
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";  
     //FlashMode_t ESP.getFlashChipMode());
  cad += "FlashChipSizeByChipId: " + String(ESP.getFlashChipSizeByChipId());  
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  

  cad += "<BR>-----------------info fileSystem-----------------<BR>";   
  FSInfo fs_info;
  if(SPIFFS.info(fs_info)) 
    {
    /*        
     struct FSInfo {
        size_t totalBytes;
        size_t usedBytes;
        size_t blockSize;
        size_t pageSize;
        size_t maxOpenFiles;
        size_t maxPathLength;
    };
     */
    cad += "totalBytes: ";
    cad += fs_info.totalBytes;
    cad += "<BR>usedBytes: ";
    cad += fs_info.usedBytes;
    cad += "<BR>blockSize: ";
    cad += fs_info.blockSize;
    cad += "<BR>pageSize: ";
    cad += fs_info.pageSize;    
    cad += "<BR>maxOpenFiles: ";
    cad += fs_info.maxOpenFiles;
    cad += "<BR>maxPathLength: ";
    cad += fs_info.maxPathLength;
    }
  else cad += "Error al leer info";
  cad += "<BR>-----------------------------------------------<BR>"; 
  
  cad += pieHTML;
  server.send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;

  cad += IDENTIFICACION;//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";

  if(server.hasArg("nombre") && server.hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    contenidoFichero=server.arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) cad += "Fichero salvado con exito<br>";
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  cad += IDENTIFICACION;//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(borraFichero(nombreFichero)) cad += "El fichero " + nombreFichero + " ha sido borrado.\n";
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";

  cad += IDENTIFICACION;//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    //inicializo el sistema de ficheros
    if (SPIFFS.begin()) 
      {
      Serial.println("---------------------------------------------------------------\nmounted file system");  
      //file exists, reading and loading
      if(!SPIFFS.exists(nombreFichero)) cad += "El fichero " + nombreFichero + " no existe.\n";
      else
        {
         File f = SPIFFS.open(nombreFichero, "r");
         if (f) 
           {
           Serial.println("Fichero abierto");
           size_t tamano_fichero=f.size();
           Serial.printf("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
           cad += "El fichero tiene un tamaño de ";
           cad += tamano_fichero;
           cad += " bytes.<BR>";
           char buff[tamano_fichero+1];
           f.readBytes(buff,tamano_fichero);
           buff[tamano_fichero+1]=0;
           Serial.printf("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
           cad += "El contenido del fichero es:<BR>";
           cad += buff;
           cad += "<BR>";
           f.close();
           }
         else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
        }  
      Serial.println("unmounted file system\n---------------------------------------------------------------");
      }//La de abrir el sistema de ficheros
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee info del FS                          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfoFS(void)
  {
  String cad=cabeceraHTML;

  cad += IDENTIFICACION;//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  //inicializo el sistema de ficheros
  if (SPIFFS.begin()) 
    {
    Serial.println("---------------------------------------------------------------\nmounted file system");  
    FSInfo fs_info;
    if(SPIFFS.info(fs_info)) 
      {
      /*        
       struct FSInfo {
          size_t totalBytes;
          size_t usedBytes;
          size_t blockSize;
          size_t pageSize;
          size_t maxOpenFiles;
          size_t maxPathLength;
      };
       */
      cad += "totalBytes: ";
      cad += fs_info.totalBytes;
      cad += "<BR>usedBytes: ";
      cad += fs_info.usedBytes;
      cad += "<BR>blockSize: ";
      cad += fs_info.blockSize;
      cad += "<BR>pageSize: ";
      cad += fs_info.pageSize;    
      cad += "<BR>maxOpenFiles: ";
      cad += fs_info.maxOpenFiles;
      cad += "<BR>maxPathLength: ";
      cad += fs_info.maxPathLength;
      }
    else cad += "Error al leer info";

    Serial.println("unmounted file system\n---------------------------------------------------------------");
    }//La de abrir el sistema de ficheros

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound()
  {
  String message = "";//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";

  message = "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  message += "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
    {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
  server.send(404, "text/html", message);
  }

void inicializaWebServer(void)
  {
  //decalra las URIs a las que va a responder
  server.on("/", handleRoot); //web de temperatura
  server.on("/temperatura", handleTemperatura); //Servicio de temperatura
  server.on("/humedad", handleHumedad); //Servicio de temperatura
  server.on("/luz", handleLuz); //Servicio de temperatura    
  server.on("/medidas", handleMedidas); //Servicio de temperatura
  server.on("/medida", handleMedida); //Servicio de temperatura con respuesta Json
  server.on("/web", handleWeb); //Servicio de temperatura  
  server.on("/test", handleTest);  //URI de test

  server.on("/reset", handleReset);  //URI de test  
  server.on("/restart", handleRestart);  //URI de test
  server.on("/info", handleInfo);  //URI de test
  
  server.on("/creaFichero", handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", handleLeeFichero);  //URI de leer fichero
  server.on("/infoFS", handleInfoFS);  //URI de info del FS

  server.onNotFound(handleNotFound);//pagina no encontrada

  server.begin();
  Serial.println("HTTP server started");
  }

void webServer(int debug)
  {
  server.handleClient();
  }
