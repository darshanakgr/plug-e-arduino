#include <ACS712.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const String device_id = "ASD001";
const char* ssid = "DROX_LTE";
const char* password = "tran$4mers";
const char* mqtt_server = "192.168.8.105";
char current[50];
boolean switchedOn = false;

WiFiClient espClient;
PubSubClient client(espClient);
ACS712 sensor(ACS712_20A, A0);

void setup_wifi() {

  delay(10);
  //switeched off the device first
  digitalWrite(5, HIGH);
  //calibrate the current sensor
  sensor.calibrate();
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("Connected to WIFI");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if (message == "00") {
    Serial.println("Switch Off");
    switchedOn = false;
    digitalWrite(5, HIGH);
  } else if (message == "01") {
    Serial.println("Switch On");
    switchedOn = true;
    digitalWrite(5, LOW);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-" + device_id;
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("loggers/ASD001");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(5, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (switchedOn) {
    String temp_str = "11" + String(sensor.getCurrentAC());
    temp_str.toCharArray(current, temp_str.length() + 1);
    Serial.println(current);
    client.publish("loggers/ASD001", current);
    delay(1000);
  }
}
