/**********************************************/
/*                                            */
/*  Gestion del Websockets                    */
/*  Dimaniza la interfaz Web                  */
/*                                            */
/**********************************************/

#define PUERTO_WEBSOCKETS 81

//#include <WebSocketsServer.h>

WebSocketsServer webSocket= WebSocketsServer(PUERTO_WEBSOCKETS);    // create a websocket server on port 81

void inicializaWebSockets()
  {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
    
  Serial.println("WebSocket server started");
  }

void atiendeWebSocket(int debug)
  {
  webSocket.loop();
  } 

void webSocketEvent(uint8_t num, WStype_t msgType, uint8_t * payload, size_t length) 
  {
  switch (msgType) 
    {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: 
      {
      IPAddress wsIp = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, wsIp[0], wsIp[1], wsIp[2], wsIp[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: #%s#\n", num, payload);
      if (!strcmp((const char*)payload,"Datos")) 
        {
        String salida=generaJson();
        webSocket.sendTXT(num, salida.c_str());  
        }
      else webSocket.sendTXT(num, "Error");
      break;
    }
  }
