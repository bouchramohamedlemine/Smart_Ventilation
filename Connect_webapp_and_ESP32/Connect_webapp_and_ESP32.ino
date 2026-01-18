/**
 * This program sends indoor temperature and humidity values to the webapp through MQTT.
 * It receives an action message from the webapp for controlling the buzzer.
 *  
*/

#include "Arduino.h"
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

// Turns the 'PRG' button into the power button, long press is off 
#define HELTEC_POWER_BUTTON   // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>

// Pause between transmited packets in seconds.
// Set to zero to only transmit a packet when pressing the user button
// Will not exceed 1% duty cycle, even if you set a lower value.
#define PAUSE               100

// Frequency in MHz. Keep the decimal point to designate float.
// Check your own rules and regulations to see what is legal where you are.
#define FREQUENCY           866.3       // for Europe
// #define FREQUENCY           905.2       // for US

// LoRa bandwidth. Keep the decimal point to designate float.
// Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define BANDWIDTH           250.0

// Number from 5 to 12. Higher means slower but higher "processor gain",
// meaning (in nutshell) longer range and more robust against interference. 
#define SPREADING_FACTOR    9

// Transmit power in dBm. 0 dBm = 1 mW, enough for tabletop-testing. This value can be
// set anywhere between -9 dBm (0.125 mW) to 22 dBm (158 mW). Note that the maximum ERP
// (which is what your antenna maximally radiates) on the EU ISM band is 25 mW, and that
// transmissting without an antenna can damage your hardware.
#define TRANSMIT_POWER      0

String rxdata;
volatile bool rxFlag = false;
long counter = 0;
uint64_t last_tx = 0;
uint64_t tx_time;
uint64_t minimum_pause;

const char* ssid = "" // Wifi name;
const char* pass = "" // Wifi password;
const char* mqtt_server = "broker.hivemq.com";

const int buzzerPin = 48; // the pin to which the buzzer is connected

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];


void setupWifi(){
  // Setup wifi connection
  delay(100);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  // when a message is received from the webapp, turn the buzzer based on the content of this message 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // if the message = ON, turn buzzer on for 10 seconds 
  if (length == 2 && payload[0] == 'O' && payload[1] == 'N') {
    digitalWrite(buzzerPin, HIGH); 
    delay(10000);
    digitalWrite(buzzerPin, LOW);
    Serial.println("Buzzer ON");
  } // if the message = OFF, turn buzzer on for 2 seconds, turn it off for 1 second and then on for 2 seconds.
  else if (length == 3 && payload[0] == 'O' && payload[1] == 'F' && payload[2] == 'F') {
    digitalWrite(buzzerPin, HIGH);  
    delay(2000);
    digitalWrite(buzzerPin, LOW);
    delay(1000);
    digitalWrite(buzzerPin, HIGH);   
    delay(2000);
    digitalWrite(buzzerPin, LOW);
    Serial.println("Buzzer OFF");
  }

}


void reconnect() {
  // Loop until we are reconnected
  while (!client.connected()) {
    Serial.print("\nAttempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("topic/buzzer");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      // Wait for 3 seconds before retrying
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin();

  setupWifi();

  // Listen for messages from the webapp and control the buzzer according to the content of these messages
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 

  pinMode(buzzerPin, OUTPUT);

  heltec_setup();
  both.println("Radio init");
  RADIOLIB_OR_HALT(radio.begin());
  // Set the callback function for received packets
  radio.setDio1Action(rx);
  // Set radio parameters
  both.printf("Frequency: %.2f MHz\n", FREQUENCY);
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  both.printf("Bandwidth: %.1f kHz\n", BANDWIDTH);
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  both.printf("Spreading Factor: %i\n", SPREADING_FACTOR);
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  both.printf("TX power: %i dBm\n", TRANSMIT_POWER);
  RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
  // Start receiving
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  heltec_loop();
  
  // If a packet was received, display it and the RSSI and SNR
  if (rxFlag) {
 
    rxFlag = false;
    radio.readData(rxdata);
    if (_radiolib_status == RADIOLIB_ERR_NONE) {
      both.printf("RX [%s]\n", rxdata.c_str());
    }
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));


    // ************** Send the received data to the webapp through MQTT 
    int tempStart = rxdata.indexOf("T:") + 2;  // Start after "T:"
    int tempEnd = rxdata.indexOf(" C");        // End before " C"
    String temperatureStr = rxdata.substring(tempStart, tempEnd); // Extract substring
    float temperature = temperatureStr.toFloat();   // Convert to float

    // Extract humidity value
    int humStart = rxdata.indexOf("H:") + 2;   // Start after "H:"
    int humEnd = rxdata.indexOf(" %");         // End before " %"
    String humidityStr = rxdata.substring(humStart, humEnd); // Extract substring
    float humidity = humidityStr.toFloat();         // Convert to float

    // Publish temperature and humidity as JSON
    String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    
    client.publish("topic/temp_hum", payload.c_str(), true);

  }
  
}


// Can't do Serial or display things here, takes too much time for the interrupt
void rx() {
  rxFlag = true;
}



