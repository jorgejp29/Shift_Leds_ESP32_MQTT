
//******************************************************************************
//    Este programa envía realiza el corrimiento de un bit en estado alto (HIGH)
// en un grupo de cuatro leds virtuales en un dashboard por protocolo MQTT 
//  utilizando conexión a unbroker y node-red.
// 
//    Como base de conexión al servidor, se utiliza un ejemplo de muestra que 
// proporcionan en el diplomado CodiogoIoT por parte del profesor Hugo Escalpelo
// 
//    La adaptación para enviar cada corrimiento esta desarrolado por:
//
//    Jorge Miguel Jaimes Ponce
//    Estudiante del Diplomado IoT  G3
//
//    Secuencia:
//               0001, 0010,  0100, 1000, 0001, 0010,  0100........
//
//*****************************************************************************


#define CAMERA_MODEL_AI_THINKER // Has PSRAM

//   Variables utilizada en la selección de la secuencia enviada al dashboard

unsigned long t_0 = 0;
unsigned long t_1 = 0;
byte  corr = 0;

//******************************************************************************

/*
 * Conexión básica por MQTT del NodeMCU
 * por: Hugo Escalpelo
 * Fecha: 28 de julio de 2021
 * 
 * Este programa envía datos  por Internet a través del protocolo MQTT. Para poder
 * comprobar el funcionamiento de este programa, es necesario conectarse a un broker
 * y usar NodeRed para visualzar que la información se está recibiendo correctamente.
 * Este programa no requiere componentes adicionales.
 * 
 * Componente     PinESP32CAM     Estados lógicos
 * ledStatus------GPIO 33---------On=>LOW, Off=>HIGH
 * ledFlash-------GPIO 4----------On=>HIGH, Off=>LOW
 */

//Bibliotecas
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT

//Datos de WiFi
const char* ssid = "*********";  // Aquí debes poner el nombre de tu red
const char* password = "************";  // Aquí debes poner la contraseña de tu red

//Datos del broker MQTT
const char* mqtt_server = "3.121.120.155"; // Si estas en una red local, coloca la IP asignada, en caso contrario, coloca la IP publica
IPAddress server(3,121,120,155);


// Objetos
WiFiClient espClient; // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient); // Este objeto maneja los datos de conexion al broker



// Variables
int flashLedPin = 4;  // Para indicar el estatus de conexión
int statusLedPin = 33; // Para ser controlado por MQTT
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
float data = 0; // Contador
int wait = 5000;  // Indica la espera cada 5 segundos para envío de mensajes MQTT

// Inicialización del programa
void setup() {
  // Iniciar comunicación serial
  Serial.begin (115200);
  pinMode (flashLedPin, OUTPUT);
  pinMode (statusLedPin, OUTPUT);
  digitalWrite (flashLedPin, LOW);
  digitalWrite (statusLedPin, HIGH);

  Serial.println();
  Serial.println();
  Serial.print("Conectar a ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password); // Esta es la función que realiz la conexión a WiFi
 
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión
    digitalWrite (statusLedPin, HIGH);
    delay(500); //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
    digitalWrite (statusLedPin, LOW);
    Serial.print(".");  // Indicador de progreso
    delay (5);
  }
  
  // Cuando se haya logrado la conexión, el programa avanzará, por lo tanto, puede informarse lo siguiente
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Si se logro la conexión, encender led
  if (WiFi.status () > 0){
  digitalWrite (statusLedPin, LOW);
  }
  
  delay (1000); // Esta espera es solo una formalidad antes de iniciar la comunicación con el broker

  Serial.end();

  // Conexión con el broker MQTT
  client.setServer(server, 1883); // Conectarse a la IP del broker en el puerto indicado
  client.setCallback(callback); // Activar función de CallBack, permite recibir mensajes MQTT y ejecutar funciones a partir de ellos
  //delay(1523);  // Esta espera es preventiva, espera a la conexión para no perder información
  delay(323);  // Esta espera es preventiva, espera a la conexión para no perder información

  
}// fin del void setup ()

// Cuerpo del programa, bucle principal
void loop() {


  //Verificar siempre que haya conexión al broker
  if (!client.connected()) {
    reconnect();  // En caso de que no haya conexión, ejecutar la función de reconexión, definida despues del void setup ()
  }// fin del if (!client.connected())
  client.loop(); // Esta función es muy importante, ejecuta de manera no bloqueante las funciones necesarias para la comunicación con el broker
  
  timeNow = millis(); // Control de tiempo para esperas no bloqueantes
  if (timeNow - timeLast > wait) { // Manda un mensaje por MQTT cada cinco segundos
    timeLast = timeNow; // Actualización de seguimiento de tiempo

// Uso del switch-case, para elegir la combinación enviada al broquer por protocolo MQTT

    switch (corr) {
        case 0:
          client.publish("esp32/data_Led","{\"Led0\":\"true\",\"Led1\":\"false\",\"Led2\":\"false\",\"Led3\":\"false\"}" );
          break;
        case 1:
          client.publish("esp32/data_Led","{\"Led0\":\"false\",\"Led1\":\"true\",\"Led2\":\"false\",\"Led3\":\"false\"}" );
          break;
        case 2:
          client.publish("esp32/data_Led","{\"Led0\":\"false\",\"Led1\":\"false\",\"Led2\":\"true\",\"Led3\":\"false\"}" );
          break;
        case 3:
          client.publish("esp32/data_Led","{\"Led0\":\"false\",\"Led1\":\"false\",\"Led2\":\"false\",\"Led3\":\"true\"}" );
          break;
        default:
          // statements
          break;
}
//*******************    FIN del / Uso del switch-case    ************************


//  Sección del programa que selecciona la nueva combinación de corrimiento
  t_1 = millis();
    
    if(t_1 - t_0 >= 1000){
      corr++;
      t_0 = t_1;  
      if(corr >= 4){
        corr = 0;
      }
    }
//  ***************************************************************************

    
  }// fin del if (timeNow - timeLast > wait)
}// fin del void loop ()

// Funciones de usuario

// Esta función permite tomar acciones en caso de que se reciba un mensaje correspondiente a un tema al cual se hará una suscripción
void callback(char* topic, byte* message, unsigned int length) {

  // Indicar por serial que llegó un mensaje
  Serial.print("Llegó un mensaje en el tema: ");
  Serial.print(topic);

  // Concatenar los mensajes recibidos para conformarlos como una varialbe String
  String messageTemp; // Se declara la variable en la cual se generará el mensaje completo  
  for (int i = 0; i < length; i++) {  // Se imprime y concatena el mensaje
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  // Se comprueba que el mensaje se haya concatenado correctamente
  Serial.println();
  Serial.print ("Mensaje concatenado en una sola variable: ");
  Serial.println (messageTemp);

  // En esta parte puedes agregar las funciones que requieras para actuar segun lo necesites al recibir un mensaje MQTT

  // Ejemplo, en caso de recibir el mensaje true - false, se cambiará el estado del led soldado en la placa.
  // El ESP323CAM está suscrito al tema esp/output
  if (String(topic) == "esp32/output") {  // En caso de recibirse mensaje en el tema esp32/output
    if(messageTemp == "true"){
      Serial.println("Led encendido");
      digitalWrite(flashLedPin, HIGH);
    }// fin del if (String(topic) == "esp32/output")
    else if(messageTemp == "false"){
      Serial.println("Led apagado");
      digitalWrite(flashLedPin, LOW);
    }// fin del else if(messageTemp == "false")
  }// fin del if (String(topic) == "esp32/output")
}// fin del void callback

// Función para reconectarse
void reconnect() {
  // Bucle hasta lograr conexión
  while (!client.connected()) { // Pregunta si hay conexión
    Serial.print("Tratando de contectarse...");
    // Intentar reconexión
    if (client.connect("ESP32CAMClient")) { //Pregunta por el resultado del intento de conexión
      Serial.println("Conectado");
      client.subscribe("esp32/output"); // Esta función realiza la suscripción al tema
    }// fin del  if (client.connect("ESP32CAMClient"))
    else {  //en caso de que la conexión no se logre
      Serial.print("Conexion fallida, Error rc=");
      Serial.print(client.state()); // Muestra el codigo de error
      Serial.println(" Volviendo a intentar en 5 segundos");
      // Espera de 5 segundos bloqueante
      delay(493);
      Serial.println (client.connected ()); // Muestra estatus de conexión
    }// fin del else
  }// fin del bucle while (!client.connected())
}// fin de void reconnect(
