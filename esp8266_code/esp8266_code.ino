#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DUST_PARTICLE_DEVICE 0

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

#if DUST_PARTICLE_DEVICE
int dust_particles[BUFFER_SIZE];

#define DUST_LED_POWER D3
#else
float temperatures[BUFFER_SIZE];
float humidities[BUFFER_SIZE];
int air_qualities[BUFFER_SIZE];

DHT dht(DHTPIN, DHTTYPE);
#endif

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
#if DUST_PARTICLE_DEVICE
    const char* device_name = "ESP8266ClientDust";
#else
    const char* device_name = "ESP8266Client";
#endif
    if (client.connect(device_name)) {
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
#if DUST_PARTICLE_DEVICE
  pinMode(DUST_LED_POWER, OUTPUT);
#else
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
#endif

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
#if DUST_PARTICLE_DEVICE
    client.publish("dust_particles", msg);
#else
    client.publish("humidity", msg);
    client.publish("temperature", msg);
    client.publish("air_quality", msg);
#endif

    for (int i = 0; i < BUFFER_SIZE; ++i) {
#if DUST_PARTICLE_DEVICE
      snprintf (msg, 100, "%d,%d", measurement_time[i], dust_particles[i]);
      client.publish("dust_particles", msg);
#else
      dtostrf(humidities[i], 1, 5, f_to_str_buff);
      snprintf (msg, 100, "%d,%s", measurement_time[i], f_to_str_buff);
      client.publish("humidity", msg);

      dtostrf(temperatures[i], 1, 5, f_to_str_buff);
      snprintf (msg, 100, "%d,%s", measurement_time[i], f_to_str_buff);
      client.publish("temperature", msg);

      snprintf (msg, 100, "%d,%d", measurement_time[i], air_qualities[i]);
      client.publish("air_quality", msg);
#endif
    }

    // Reset buffer
    measure_idx = 0;
  }

  // Measure data
  measure();
  
  lastMsg = now;
}

void measure() {
  long now = millis();
  measurement_time[measure_idx] = now;
  
#if DUST_PARTICLE_DEVICE
  dust_particles[measure_idx] = analogRead(0);
#else
  humidities[measure_idx] = dht.readHumidity();
  temperatures[measure_idx] = dht.readTemperature();
  air_qualities[measure_idx] = analogRead(0);
#endif

  measure_idx += 1;
}


