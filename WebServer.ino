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
//#define MODULO "Modulo " + String(direccion) + " Habitacion= " + nombres[direccion] + "<BR>"
#define IDENTIFICACION "Version " + String(VERSION) + "." + "<BR>"

#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <FS.h>

ESP8266WebServer server(PUERTO_WEBSERVER);

/*
String cabeceraHTML="<!DOCTYPE html><HTML><HEAD><TITLE>" + nombre_dispositivo + " </TITLE></HEAD><BODY><h1><a href=\"../\" target=\"_self\">" + nombre_dispositivo + "</a><br></h1>";
String enlaces="<TABLE>\n<CAPTION>Enlaces</CAPTION>\n<TR><TD><a href=\"info\" target=\"_self\">Info</a></TD></TR>\n<TR><TD><a href=\"test\" target=\"_self\">Test</a></TD></TR>\n<TR><TD><a href=\"restart\" target=\"_self\">Restart</a></TD></TR>\n<TR><TD><a href=\"listaFicheros\" target=\"_self\">Lista ficheros</a></TD></TR>\n</TABLE>"; 
String pieHTML="</BODY></HTML>";
*/

const String cabeceraHTMLlight = "<!DOCTYPE html>\n<head>\n<meta charset=\"UTF-8\" />\n<TITLE>Domoticae</TITLE><link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\"></HEAD><html lang=\"es\">\n<BODY>\n"; 
const String pieHTMLlight="</body>\n</HTML>\n";


void inicializaWebServer(void)
  {
  //decalra las URIs a las que va a responder
  server.on("/", HTTP_ANY, handleMain); //layout principal
  server.on("/estado", HTTP_ANY, handleEstado); //Servicio de temperatura con respuesta Json
  server.on("/nombre", handleNombre); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos  
  server.on("/root", HTTP_ANY, handleRoot); //devuleve el frame con la informacion principal

  server.on("/temperatura", HTTP_ANY, handleTemperatura); //Servicio de temperatura
  server.on("/humedad", HTTP_ANY, handleHumedad); //Servicio de humedad
  server.on("/presion", HTTP_ANY, handlePresion); //Servicio de presion
  server.on("/altitud", HTTP_ANY, handleAltitud); //Servicio de altitud
  server.on("/luz", HTTP_ANY, handleLuz); //Servicio de luz
   
  server.on("/restart", HTTP_ANY, handleRestart);  //URI de test
  server.on("/info", HTTP_ANY, handleInfo);  //URI de test
  
  server.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero
  server.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero
  server.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero  
  server.on("/infoFS", HTTP_ANY, handleInfoFS);  //URI de info del FS

 //load editor
  server.on("/upload", HTTP_GET, []() {
    if (!handleFileRead("/upload.html")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  
  //first callback is called after the request has ended with all parsed arguments, second callback handles file uploads at that location  
  server.on("/upload",  HTTP_POST, []() {  // If a POST request is sent to the /upload.html address,
    server.send(200, "text/plain", "Subiendo..."); 
  }, handleFileUpload);                       // go to 'handleFileUpload'
  
  
  
  /*
  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", ""); 
  }, handleFileUpload);                       // go to 'handleFileUpload'
  */
  server.onNotFound(handleNotFound);//pagina no encontrada

  server.begin();
  Serial.println("HTTP server started");
  }

void webServer(int debug)
  {
  server.handleClient();
  }

void handleMain() 
  {
  server.sendHeader("Location", "main.html",true); //Redirect to our html web page 
  server.send(302, "text/html","");    
  }

void handleRoot() 
  {
  server.sendHeader("Location", "root.html", true); //Redirect to our html web page */
  server.send(302, "text/html","");    
  }

void handleNombre()
  {
  const size_t capacity = JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  root["nombre"] = nombre_dispositivo;
  root["version"] = VERSION;
  
  String cad="";
  root.printTo(cad);
  server.send(200,"text/json",cad);
  }
  
/*********************************************/
/*                                           */
/*  Estado                                   */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*********************************************/
void handleEstado() 
  {
  String cad=generaJson();
  
  server.send(200, "application/json", cad);   
   
  //Serial.println("Medidas requeridas ok");
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
/*  Medida de presion                        */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *   const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"presion\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Presion"]; // 23.1
*/  
/*********************************************/
void handlePresion(){
  String cad="";

  //genero la respuesta OK --> {"Presion": 22.5}
  cad  = "{\"presion\": ";
  cad += String(getPresion(),1);
  cad += "}";
  
  server.send(200, "application/json", cad);      
}

/*********************************************/
/*                                           */
/*  Medida de altitud                        */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *   const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"altitud\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["altitud"]; // 23.1
*/  
/*********************************************/
void handleAltitud(){
  String cad="";

  //genero la respuesta OK --> {"Presion": 22.5}
  cad  = "{\"altitud\": ";
  cad += String(getAltitud(),1);
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
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(void)
  {
  String cad="";

  cad += cabeceraHTMLlight;
  cad += IDENTIFICACION;
  
  cad += "Reiniciando...<br>";
  cad += pieHTMLlight;
    
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
  String cad="";

  cad += cabeceraHTMLlight;
  cad += IDENTIFICACION;

  cad += "<BR>----------------- uptime --------------------<BR>";
  unsigned long milis=millis();

  cad += "uptime (ms): " + String(milis);
  cad += "<BR>";

  int8_t segundos=(milis/1000)%60;
  int8_t minutos=(milis/60000)%60;
  int8_t horas=milis/3600000;

  char horaText[9]="";
  sprintf(horaText,"%02i:%02i:%02i",horas,minutos,segundos);
  cad += String(horaText);
  cad += "<BR>---------------------------------------------<BR>";

  cad += "<BR>-----------------info logica-----------------<BR>";
  cad += "IP: " + String(getIP(debugGlobal));
  cad += "<BR>";  

  cad += "id: ";
  cad += String(direccion);
  cad += "<BR>";   
  
  cad += "Temperatura: ";
  cad += String(getTemperatura(),1);
  cad += "<BR>";     
  cad += "Humedad: ";
  cad += String(getHumedad(),1);
  cad += "<BR>";     
  cad += "Luz: ";
  cad += String(getLuz(),1);
  cad += "<BR>";     
  cad += "-----------------------------------------------<BR>"; 
  
  cad += "<BR>-----------------Contadores info-----------------<BR>";
  cad += "multiplicadorAnchoIntervalo: ";
  cad += String(multiplicadorAnchoIntervalo);
  cad += "<BR>";     
  cad += "anchoIntervalo: ";
  cad += String(anchoIntervalo);
  cad += "<BR>";   
  cad += "frecuenciaOTA: ";
  cad += String(frecuenciaOTA);
  cad += "<BR>";   
  cad += "frecuenciaLeeSensores: ";
  cad += String(frecuenciaLeeSensores);
  cad += "<BR>";   
  cad += "frecuenciaServidorWeb: ";
  cad += String(frecuenciaServidorWeb);
  cad += "<BR>";   
  cad += "frecuenciaOrdenes: ";
  cad += String(frecuenciaOrdenes);
  cad += "<BR>"; 
  cad += "frecuenciaMQTT: ";
  cad += String(frecuenciaMQTT);
  cad += "<BR>";
  cad += "frecuenciaWifiWatchdog: ";
  cad += String(frecuenciaWifiWatchdog); 
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>"; 
  
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
  
  cad += pieHTMLlight;
  server.send(200, "text/html", cad);     
  }
/************************* FICHEROS *********************************************/
/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(void)
  {
  String mensaje="";
  String nombreFichero="";
  String contenidoFichero="";
  
  boolean salvado=false;

  if(server.hasArg("nombre") && server.hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    contenidoFichero=server.arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) 
      {
      handleListaFicheros();
      return;
      }
    else mensaje = "No se pudo salvar el fichero"; 
    }
  else mensaje = "Falta el argumento <nombre de fichero>"; 

  server.sendHeader("Location", String("mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");  
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(void)
  {
  String nombreFichero="";
  String mensaje="";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(!borraFichero(nombreFichero)) mensaje = "No se pudo borrar el fichero " + nombreFichero + ".\n";   
    else
      {
      handleListaFicheros();
      return;
      }
    }
  else  mensaje = "Falta el argumento <nombre de fichero>"; 
  
  server.sendHeader("Location", String("mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");        
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(void)
  {
  String cad=cabeceraHTMLlight;
  String nombreFichero="";
  String contenido="";
  
  cad += "<h1>" + String(NOMBRE_FAMILIA) + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";           
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";   
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTMLlight;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado del        */
/*  fichero indicado, a traves de una        */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
void handleManageFichero(void)
  {
  String nombreFichero="";
  String contenido="";
  String cad=cabeceraHTMLlight;
   
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(leeFichero(nombreFichero, contenido))
      {            
      cad += "<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 5px; text-align: left;}</style>";

    cad += "<form id=\"borrarFichero\" action=\"/borraFichero\">\n";
    cad += "  <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">\n";
    cad += "</form>\n";

    cad += "<form action=\"creaFichero\" target=\"_self\">";
      
      cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";      
      cad += "<caption>Fichero: " + nombreFichero + "(" + contenido.length() + ")</caption>";
      cad += "<tr><td colspan=\"2\">";      

      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    <textarea cols=75 rows=20 name=\"contenido\">" + contenido + "</textarea>";
      cad += "    </td>";
      cad += "    </tr>"; 
           
      cad += "    <tr>";
      cad += "    <td>"; 
      cad += "    <input type=\"submit\" value=\"salvar\">";
      cad += "    </td>";

      cad += "    <td align=\"right\">\n";  
      cad += "    <button type=\"button\" onclick=\"document.getElementById('borrarFichero').submit();\">Borrar</button>\n";
      cad += "    </td>\n";
                  
      cad += "    </tr>"; 
      
      cad += "</table>";
      cad += "</form>";     
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTMLlight;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros(void)
  {
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;
  String cad=cabeceraHTMLlight;

  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad += "<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 5px; text-align: left;}</style>";
    cad += "<br>\n";

    cad += "<table style=\"border: 0px; border-color: #FFFFFF;\">\n";
    cad += "<tr style=\"border: 0px; border-color: #FFFFFF;\">";
    cad += "<td style=\"vertical-align: top; border: 0px; border-color: #FFFFFF;\">";
    
    cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
    cad += "<caption>Lista de ficheros</caption>\n";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR>";
      cad += "<TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editar\">";
      cad += "</form>";
      cad += "</TD>";
      cad += "<TD>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "</form>";
      cad += "</TD>";
      cad += "</TR>";
      }
    cad += "</TABLE>\n";
    cad += "</td>";
    
    //Para crear un fichero nuevo
    cad += "<td style=\"vertical-align: top; border: 0px; border-color: #FFFFFF;\">";    
    //Para crear un fichero nuevo
    cad += "<table>";
    cad += "<caption>Crear un fichero nuevo:</caption>";
    cad += "<tr><td>";    
    cad += "<form action=\"creaFichero\" target=\"_self\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</form>";
    cad += "</td></tr>";
    cad += "</table>";  

    cad += "</td>";
    cad += "</tr></table>";        
    }
  else cad += "<H2>No se pudo recuperar la lista de ficheros</H2>"; 

  cad += pieHTMLlight;
  server.send(200, "text/html", cad); 
  }
/**********************************************************************/

/*********************************************/
/*                                           */
/*  Lee info del FS                          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfoFS(void)
  {
  String cad=cabeceraHTMLlight;
  
  cad += IDENTIFICACION;
  
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

  cad += pieHTMLlight;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound()
  {
  if(handleFileRead(server.uri()))return;
    
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

/**********************************************************************/
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) 
  { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  
  if (!path.startsWith("/")) path += "/";
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  //if (!path.endsWith("/")) path += "/";
  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) 
    { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
    }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
  }

 void handleFileUpload()
  {
  String path = "";
  static File fsUploadFile;
  HTTPUpload& upload = server.upload();

  Serial.printf("Vamos a subir un fichero...");
  if(upload.status == UPLOAD_FILE_START)
    {
    if(server.hasArg("directorio"))path=server.arg("directorio");
    if(!path.startsWith("/")) path = "/" + path;
    if(!upload.filename.startsWith("/")) path = path + "/";
    path += upload.filename;    
    
    Serial.printf("handleFileUpload Name: %s",path.c_str());
    fsUploadFile = SPIFFS.open(path.c_str(), "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    if(!fsUploadFile) Serial.printf("Error al crear el fichero\n");
    }
  else if(upload.status == UPLOAD_FILE_WRITE)
    {
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    else Serial.printf("Error al escribir en el fichero\n");
    } 
  else if(upload.status == UPLOAD_FILE_END)
    {
    String mensaje="";  

    if(fsUploadFile) // If the file was successfully created
      {                                    
      fsUploadFile.close();                               // Close the file again
      Serial.printf("handleFileUpload Size: %i", upload.totalSize);
      mensaje="Fichero subido con exito (" + String(upload.totalSize) + "bytes)";  
      }
    else mensaje="Se produjo un error al subir el fichero [" + path + "]";  

    server.sendHeader("Location","resultadoUpload.html?mensaje=" + mensaje, true);      // Redirect the client to the success page
    server.send(302);  
    }
  }
