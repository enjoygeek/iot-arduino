#include <ESP8266WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

#define DHTPIN 5 // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11 // DHT 11
#define DHTTYPE DHT22 // DHT 22

const char* ssid = "LaloLanda"; // Your ssid
const char* password = "departamento3d"; // Your Password
const char* mqttServer = "mqtt.thingspeak.com";

WiFiServer server(80);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

DHT dht(DHTPIN, DHTTYPE);

// track the last connection time
unsigned long lastConnectionTime = 0;
// post data every 20 seconds
const unsigned long postingInterval = 20L * 1000L;

void setup() {
Serial.begin(115200);
delay(10);
Serial.println();
WiFi.mode(WIFI_STA);
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
server.begin();
Serial.println("Server started");
Serial.println(WiFi.localIP());
mqttClient.setServer(mqttServer, 1883);
}

void loop() {
if (!mqttClient.connected()) {
  reconnect();
}
mqttClient.loop();

 delay(2000);
 float h = dht.readHumidity();
 float t = dht.readTemperature();
 float f = dht.readTemperature(true);
 if (isnan(h) || isnan(t) || isnan(f)) {
  Serial.println("Err reading DHT sensor data");
 return;
 } else {
 float hif = dht.computeHeatIndex(f, h);
 float hic = dht.computeHeatIndex(t, h, false);
 Serial.print("Humidity: ");
 Serial.print(h);
 Serial.print(" %\t");
 Serial.print("Temperature: ");
 Serial.print(t);
 Serial.print(" *C ");
 Serial.print(f);
 Serial.print(" *F\t");
 Serial.print("Heat index: ");
 Serial.print(hic);
 Serial.print(" *C ");
 Serial.print(hif);
 Serial.println(" *F");
 }
 /* MQTT */
 // If interval time has passed since the last connection, Publish data to ThingSpeak
  if (millis() - lastConnectionTime > postingInterval)
  {
    mqttpublish(t,h);
  }
 /* Fin MQTT */
WiFiClient client = server.available();
client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println("Connection: close");
client.println("Refresh: 60");
client.println();
client.println("<!DOCTYPE html>");
client.println("<html xmlns='http://www.w3.org/1999/xhtml'>");
client.println("<head>\n<meta charset='UTF-8'>");
client.println("<title>Temperature & Humidity</title>");
client.println("</head>\n<body bgcolor='#000318'>");
client.println("<div style='margin: auto; padding: 20px; text-align: center; font-family: Impact, Charcoal, sans-serif; color: #fff'>");
client.println("<H1>ESP8266 & DHT11/22 Sensor</H1>");
client.println("<div style='font-size: 8em;'>");
client.print("Temperature<br /><b>");
client.print((float)t, 2);
client.println("°C</b><br />");
client.print("Humidity<br /><b>");
client.print((float)h, 2);
client.println("%</b><br />");
client.println("</div>");
client.println("");
client.println("</div>");
client.print("</body>\n</html>");
}

void mqttpublish(float t, float h) {
  // Create data string to send to ThingSpeak
  String data = String("field1=" + String(t, DEC) + "&field2=" + String(h, DEC));
  // Get the data string length
  int length = data.length();
  char msgBuffer[length];
  // Convert data string to character buffer
  data.toCharArray(msgBuffer,length+1);
  Serial.println(msgBuffer);
  // Publish data to ThingSpeak. Replace <YOUR-CHANNEL-ID> with your channel ID and <YOUR-CHANNEL-WRITEAPIKEY> with your write API key
  mqttClient.publish("channels/223497/publish/ZUX7AMHWFFDINU6M",msgBuffer);
  // note the last connection time
  lastConnectionTime = millis();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "esp8266-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //mqttClient.publish("esp-c0794b/outTopic", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
