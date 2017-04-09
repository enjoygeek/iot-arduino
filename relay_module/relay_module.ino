/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.

const char* ssid = "LaloLanda";
const char* password = "departamento3d";
const char* mqtt_server = "cuys-srv.cloudapp.net";
const int mqtt_port = 8080;
const char* mqtt_username = "guest"; // Your ssid
const char* mqtt_password = "password"; // Your ssid

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int IN1 = 14; // GPIO14 (D5)
int IN2 = 12; // GPIO12 (D6)
int IN3 = 13; // GPIO13 (D7)
int IN4 = 5; // GPIO5 (D1)

int inputs[] = {IN1,IN2,IN3,IN4};
int states[] = {false,false,false,false};
int pinCount = 4;

const bool inverted = false;

struct ModuleData {
  String module;
  String operation;
  String port;
  String value;
};

//current sensor
const int analogIn = A0;
int mVperAmp = 66; // use 100 for 20A Module and 66 for 30A Module
int RawValue= 0;
int ACSoffset = 2500; 
double Voltage = 0;
double Amps = 0;

void setup() {
  for(int i = 0; i < pinCount; i++){
    pinMode(inputs[i], OUTPUT);
    if(inverted){
      digitalWrite(inputs[i], HIGH);
    }
  }
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool deserialize(ModuleData& data, char* json)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);

    const char* jModule = root["module"];
    const char* jOperation = root["operation"];
    const char* jPort = root["data"]["port"]; 
    const char* jValue = root["data"]["value"];
    data.module = String(jModule);
    data.operation = String(jOperation);
    data.port = String(jPort);
    data.value = String(jValue);
    
    return root.success();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


  //Fetch values
  ModuleData data = {"","",""};
  
  if(!deserialize(data,(char*)payload)){
    Serial.println("parseObject failed");
    return;
  }

 //JsonObject& data = jsonBuffer.parse(dataJson);

  //Print values
  Serial.println(data.module);
  Serial.println(data.port);
  Serial.println(data.value);

  if(client.connected()){
    if(data.operation.equals("GET")){
      if (data.module.equals("relay")) {
        String msg = "{ \"module\" : \"relay\", \"data\" : { \"port\" : \""+data.port+"\", \"value\": \""+ states[data.port.toInt()-1]+"\" } }";
        Serial.println(msg);
        client.publish("esp-c0794b/outTopic", msg.c_str() );  
      }
    }
  }

  if(data.operation.equals("POST")){ // OHHH SUPER IFFF !!!! WTF!!!!!!! 
    // Switch on the LED if an 1 was received as first character
    if (data.module.equals("relay")) {
      int gpio = getGPIO(data.port);
      if(gpio > -1){
        if(data.value.equals("1")){ // TURN ON. TODO: Aqui va el control del puerto y arriba debe ir el control del modulo/sensor
          if(inverted){
            digitalWrite(gpio, LOW);   // Turn the LED on (Note that LOW is the voltage level
          }
          else{
            digitalWrite(gpio, HIGH);   // Turn the LED on (Note that HIGH is the voltage level
          }
          states[data.port.toInt()-1] = true;
          // but actually the LED is on; this is because
          // it is acive low on the ESP-01)
        }else { //TURN OFF
          if(inverted){
            digitalWrite(gpio, HIGH);   // Turn the LED off (Note that HIGH is the voltage level
          }
          else{
            digitalWrite(gpio, LOW);   // Turn the LED off (Note that LOW is the voltage level
          }
          states[data.port.toInt()-1] = false;
        }
      }
      
    }
  }
}


int getGPIO(String input){
  if(input.equals("1")) return IN1; // GPIO 14 (D5)
  if(input.equals("2")) return IN2; // GPIO 12 (D6)
  if(input.equals("3")) return IN3; // GPIO 13 (D7)
  if(input.equals("4")) return IN4; // GPIO 15 (D8)
  return -1;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "esp8266-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_username,mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("esp-c0794b/outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("esp-c0794b/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  senseCurrent();

  //Timed output
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    ++value; //Cicle count

    //Hacer algo
  }
  
}


void senseCurrent() {
  float avgAcc = 0.0;
  for(int i = 0; i < 200; i++){
    avgAcc = avgAcc + analogRead(analogIn);
    delay(3);
  }
  RawValue = avgAcc/200;
  Voltage = (RawValue / 1024.0) * 5000; // Gets you mV
  Amps = ((Voltage - ACSoffset) / mVperAmp);
  Serial.print("Raw Value = " ); // shows pre-scaled value 
  Serial.print(RawValue); 
  Serial.print("\t mV = "); // shows the voltage measured 
  Serial.print(Voltage,3); // the '3' after voltage allows you to display 3 digits after decimal point
  Serial.print("\t Amps = "); // shows the voltage measured 
  Serial.println(Amps,3); // the '3' after voltage allows you to display 3 digits after decimal point

  snprintf (msg, 75, "RawValue %ld", RawValue);
  //client.publish("outTopic", msg);
  delay(100);
}

// Common functions 

String getTime() {
  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
    Serial.println("connection failed, retrying...");
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");
 
  while(!!!client.available()) {
     yield();
  }

  while(client.available()){
    if (client.read() == '\n') {    
      if (client.read() == 'D') {    
        if (client.read() == 'a') {    
          if (client.read() == 't') {    
            if (client.read() == 'e') {    
              if (client.read() == ':') {    
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
                return theDate;
              }
            }
          }
        }
      }
    }
  }
}
