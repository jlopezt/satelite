var titulo = document.getElementById('Titulo');
var id = document.getElementById("Id");
var temp = document.getElementById("Temperatura");
var hum = document.getElementById("Humedad");
var luz = document.getElementById("Luz");

var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);

function updateData(event)
{
  document.getElementById('valoresLeidos').innerHTML = event.data; 
		
	var msg = JSON.parse(event.data);
	
	console.log('titulo=' + msg.titulo);
	console.log('id=' + msg.id);
	console.log('temperatura=' + msg.Temperatura);
	console.log('humedad=' + msg.Humedad);
	console.log('luz=' + msg.Luz);

  document.getElementById('Titulo').innerHTML = msg.titulo; 
  document.getElementById("Id").innerHTML = msg.id; 
  document.getElementById("Temperatura").innerHTML = msg.Temperatura; 
  document.getElementById("Humedad").innerHTML = msg.Humedad; 
  document.getElementById("Luz").innerHTML = msg.Luz; 

}

function scheduleRequest() {
   connection.send("Datos");
 }
 
connection.onopen = function () {
    //connection.send('Connect ' + new Date());
    console.log('Connect ' + new Date());
    connection.send("Datos");
    console.log('peticion inicial enviada');
    setTimeout(scheduleRequest, 100);
   // Ejemplo 1, peticion desde cliente
   //(function scheduleRequest() {
   //   connection.send("");
   //   setTimeout(scheduleRequest, 100);
   //})();    
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};

connection.onclose = function(){
    console.log('WebSocket connection closed');
};

connection.onmessage = function (event) {
  console.log('Server: ', event.data);
  updateData(event);
};

