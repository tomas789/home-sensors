#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN D2
#define DHTTYPE DHT22
#define BUFFER_SIZE 2

const char* ssid = "bibi";
const char* password = "11223344";
const char* mqtt_server = "10.0.1.10";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];
char f_to_str_buff[30];
int value = 0;

int measure_idx = 0;
long measurement_time[BUFFER_SIZE];
float temperatures[BUFFER_SIZE];
float humidities[BUFFER_SIZE];
int air_qualities[BUFFER_SIZE];

DHT dht(DHTPIN, DHTTYPE);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
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
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  long now = millis();
  if ((now - lastMsg) < 2000) {
    delay(500);
    return; 
  }

  if (measure_idx == BUFFER_SIZE) {
    // Send data
    Serial.println("Buffer is full -> sending data");

    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    snprintf(msg, 100, "time: %d", now);
    client.publish("humidity", msg);
    client.publish("temperature", msg);
    client.publish("air_quality", msg);

    for (int i = 0; i < BUFFER_SIZE; ++i) {
      dtostrf(humidities[i], 1, 5, f_to_str_buff);
      snprintf (msg, 100, "%d,%s", measurement_time[i], f_to_str_buff);
      client.publish("humidity", msg);

      dtostrf(temperatures[i], 1, 5, f_to_str_buff);
      snprintf (msg, 100, "%d,%s", measurement_time[i], f_to_str_buff);
      client.publish("temperature", msg);

      snprintf (msg, 100, "%d,%d", measurement_time[i], air_qualities[i]);
      client.publish("air_quality", msg);
    }

    // Reset buffer
    measure_idx = 0;
  }

  // Measure data
  measurement_time[measure_idx] = now;
  humidities[measure_idx] = dht.readHumidity();
  temperatures[measure_idx] = dht.readTemperature();
  air_qualities[measure_idx] = analogRead(0);

  measure_idx += 1;
  lastMsg = now;
}


