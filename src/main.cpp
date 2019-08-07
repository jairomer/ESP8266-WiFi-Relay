#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/*
 * Each different deployment requires an ID to identify the messages 
 * sent to the device as a MQTT topic. 
 * 
 *  'wifi-rele/<ID>/r'
 *  'wifi-rele/<ID>/w'
 *  
 * Once the device has connected successfully to the wifi network, it 
 * will connect to the Eclipse Mosquitto MQTT broker and subscribe to 
 * this topic. 
 * 
 * After each operation message, the device will send the operation 
 * status as a message published to this same topic. S
*/

/* MQTT parameters */
const char* DEV_NAME = "wifi-relay-0";

/* Topics */
const char* R_TOPIC = "wifi-relay/0/r";
const char* W_TOPIC = "wifi-relay/0/w";

/* User and password defined on MQTT broker. */ 
const char* USER = "wifi-relay-0"; 
const char* PASS = "UserPasswordDefinedOnTheBroker"; 
const char* MQTT_SERVER = "MqttBrokerIpOrDNS";
const int TLS_PORT = 8883;

/* TLS Wifi */
BearSSL::WiFiClientSecure tlsClient; 
PubSubClient mqtts_client(tlsClient);

/* Time and Date */
const char* ntpServerName = "pool.ntp.org";
const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServerName, utcOffsetInSeconds);

/* WIFI Authentication*/
const char* ssid = "YourWifiNetwork"; 
const char* password = "YourWifiNetworkPassword";

/* Messages */
const char* RELAY_OPEN  = "OPEN";
const char* RELAY_CLOSE = "CLOSE";
const char* RELAY_READY = "READY";

/* TLS Certificate */
const char CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIHcAgEBBEIATXmkkoaxsd7d6QvaLYOFBpVWIKkpZiIVifjWyEvG7KORzlGXuWzA
67CkiTbUMscnzM7kn/YrwmITRDaYQ2eF0jagBwYFK4EEACOhgYkDgYYABAFzgTPk
co/CM1hNYyRm8Tnlq0l+rnFSst74VHqoj2wD9XOz7W8iFX1C0J4KsQy2N6FAccym
72tTstwCruZmuc91mgC+RyRm9TxcwvztEOFDkWeKpVCrheILGH03zBqb93p9nTIa
bUMscnzM7kn/YrwmITRDaYQ2eF0jagBwYFK4EEACOhgYkDgYYABAFzgTPkco/CM1
Rm8Tnlq0l+rnFSst74VHqoj2wD9XOz7W8iFX1C0J4KsQy2N6FAccymFSst74VHqF
zBqb93p9nTIa72tTstwCruZmuc91mgC+RyRm9TxcwvztEOFDkWeKpVCrheILGH03
zM7kn/YrwmITRDaYQ2eF0jag67CkiTbUMscnBwYFK4EEACOhgYkDgYYABAFzgTPk
qoj2wD9XOzco/CM1hNYJ4KsQy2N6FAccymyRm8Tnlq0l+7W8iFX1C0rnFSst74VH
rheILGH03zBqb93p9nTIa72tTc91mgC+RyRm9TxcwvztEOFDkWeKpVCstwCruZmu
qoj2wD9XOzco/CM1hGbPfS2UEKITVxTth9OZ+4rplg==
-----END CERTIFICATE-----
)EOF";

/* MQTT server fingerprint, used for verification. */
const uint8_t mqttCertFingerprint[] = 
  {0x8E, 0xE1, 0x27, 0x9C, 0x31, 0x33, 0x0D, 0xB0, 0x6B, 0xBC, 0x38, 0x52, 0x8F, 0xEA, 0x0E, 0x14, 0x8D, 0x06, 0xA5, 0x04};

BearSSL::X509List x509(CA_CERT); 


/* 
 *  GPIO Ids change depending on the board variant. 
 *    - GPIO 0 is used to control the relay, LED 0 is located on the 
 *      relay board. 
 *    - On ESP8266EX, BUILTIN led is GPIO 1. 
 *    - On ESP8266S,  BUILTIN led is GPIO 2. 
*/ 
enum Variant {ESP8266S, ESP8266EX};
Variant variant = ESP8266S;  // To change according with the board.

int builtinLed(Variant v) {
  switch (v) {
    case ESP8266S:
      return 2;
    case ESP8266EX:
      return 1; 
    default: 
      break; 
  }
  return 3; // Undefined board variant.
}

/*
 *  The integrated relay has two modes depending on the physical wiring:
 *    a) Normally closed.
 *    b) Normally open. 
 *   
 *  We will assume NORMALLY CLOSED or NC in this program.  
 */

const int RELAY_PIN = 0;
bool isRelayOpen = false; // Normally closed

/*
 *  Open relay logic. 
 */
bool openRelay(){
  if (!isRelayOpen) {
    digitalWrite(RELAY_PIN, LOW);
    isRelayOpen = true;
    Serial.println("Opening relay.");
    return true;
  }
  
  Serial.println("Relay already open.");
  return false;
}

/*
 *  Close relay logic. 
 */
bool closeRelay(){
  if (isRelayOpen) {
    digitalWrite(RELAY_PIN, HIGH);
    isRelayOpen = false;
    Serial.println("Closing relay.");
    return true;
  }
   
  Serial.println("Relay already closed.");
  return false;
}

/*
 * Prints to a serial terminal the connection status of the ESP8266 wifi interface. 
 * 
 *    0 : WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses
 *    1 : WL_NO_SSID_AVAIL in case configured SSID cannot be reached
 *    3 : WL_CONNECTED after successful connection is established
 *    4 : WL_CONNECT_FAILED if password is incorrect
 *    6 : WL_DISCONNECTED if module is not configured in station mode
 */
void printConnectionStatus() {
  Serial.printf("WIFI Status: ");
  switch (WiFi.status())
  {
  case 0:
    Serial.println("In process of changing between statuses.");
    break;
  case 1:
    Serial.println("Configured SSID cannot be reached.");
    break;
  case 3: 
    Serial.println("Successful connection established.");
    break;
  case 4:
    Serial.println("Incorrect network password.");
    break;
  case 6: 
    Serial.println("Disconnected.");
    break;
  default:
    Serial.println("Failed to retrieve WIFI status.");
    break;
  }
}

/*
 * Verify TLS
 *  Debug function to test WifiClientSecure connection via certificate and fingerprint. 
 */
bool verifytls() {
  bool success = false; // Assume failure. 
  success = tlsClient.connect(MQTT_SERVER, TLS_PORT);
  if (success)
    Serial.println("Connection complete, valid cert, valid fingerprint. ");
  else 
    Serial.println("Connection failed. TLS verification failed.");
  return (success);
}

/*
 * Print mqtt client state to Serial to keep insanity away.
 */
void printMqttClientStatus() {
  if (!Serial) return;
  Serial.print("Client state: ");
  switch(mqtts_client.state()){
    case MQTT_CONNECTION_TIMEOUT:
      Serial.println("Connection timeout.");
      break;
    case MQTT_CONNECT_FAILED:
      Serial.println("Connection failed.");
      break;
    case MQTT_CONNECTION_LOST:
      Serial.println("Connection lost.");
      break;
    case MQTT_DISCONNECTED:
      Serial.println("Disconnected.");
      break;
    case MQTT_CONNECTED:
      Serial.println("Connected.");
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      Serial.println("Bad protocol.");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      Serial.println("Bad credentials.");
      break;
    case MQTT_CONNECT_UNAUTHORIZED: 
      Serial.println("Unauthorized.");
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      Serial.println("Bad client ID.");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      Serial.println("Unavailable.");
      break;
    default:
      Serial.println("Unknown");   
  };
}

/* 
 *  Task to execute on message reception from R_TOPIC.
 */
void callback(char* topic, byte* payload, unsigned int lenght) { 

  if (strncmp((char*)payload, RELAY_OPEN, lenght) == 0) {
    // Rele set to OPEN
    if(openRelay())
      mqtts_client.publish(W_TOPIC, "Opening relay.");
    else 
      mqtts_client.publish(W_TOPIC, "Relay already open.");

  }
  else if (strncmp((char*)payload, RELAY_CLOSE, lenght) == 0) {
    // Rele set to CLOSED
    if(closeRelay())
      mqtts_client.publish(W_TOPIC, "Closing relay.");
    else 
      mqtts_client.publish(W_TOPIC, "Relay already closed.");

  }
  digitalWrite(builtinLed(variant), HIGH);
}

/* 
 *  Attempt (re)connection to WiFi network until success. 
 *  Blink at 5 Hzs while disconnected. 
 */ 
void connectWifi() {
  printConnectionStatus();
  if (WiFi.status() == WL_DISCONNECTED) {
    Serial.printf("Setting Wifi mode to station: %s\n", 
    WiFi.mode(WIFI_STA) ? "SUCCESS" : "FAILURE");

    while(!WiFi.begin(ssid, password)) {
      Serial.println("Connection to Wifi network...");      
      // Wait 1 second before reconnecting.
      for(int i=0; i<10; i++) {
        Serial.print("*");
        digitalWrite(builtinLed(variant), LOW);
        delay(100);
        digitalWrite(builtinLed(variant), HIGH);
        Serial.print("*");
        delay(100);  
      }
      printConnectionStatus();
    }
    Serial.println("Connected");
    Serial.print("The IP Address of ESP8266 Module is: "); 
    Serial.println(WiFi.localIP()); // Print the Ip address. 
    Serial.println("");
  }

  WiFi.printDiag(Serial); // Print snapshot of wifi status. 
}

/* 
 *  Attempt subscription to MQTT server until success. 
 *  Blink at 10 Hz while disconnected. 
 */
void connectMQTT() {
  Serial.println("Lost connection with MQTT broker.");
  printConnectionStatus();
  Serial.println("Attempting reconnection...");
  bool on = true; // Status led.

  while(!mqtts_client.connected()) {

    Serial.println("Connecting...");
    /* MQTTS session */

    if (mqtts_client.connect(DEV_NAME, USER, PASS)) {
      Serial.println("Connected.");
      if (mqtts_client.subscribe(R_TOPIC)) {
        Serial.print("Subscribed: ");
        Serial.println(R_TOPIC);
      }
      else 
        Serial.println("[ERROR] Cannot subscribe.");
    }
    else {
      Serial.println("Cannot connect to MQTT server.");
      char buff[56];
      tlsClient.getLastSSLError(buff, 56);
      Serial.print("SSL eror: ");
      Serial.println(buff);
    }    
    // Print device state.
    Serial.println(mqtts_client.state()); 
    printConnectionStatus();
    
    // Wait 5 seconds before retrying
    for (int i=0; i<=50; i++) {
      Serial.print("*");
      if (on)
        digitalWrite(builtinLed(variant), HIGH);
      else
        digitalWrite(builtinLed(variant), LOW); 
      on = !on;
      delay(100); 
    }
    Serial.println("");
  }
  // Anounce ready status. 
  if(!mqtts_client.publish(W_TOPIC, RELAY_READY))
    Serial.println("[ERROR] Cannot publish ready message!");
  else 
    Serial.println("READY message sent.");
}

void setup(void) {
  /* Setup Status LED */
  pinMode(builtinLed(variant), OUTPUT);
  
  /* Setup Relay */
  pinMode(RELAY_PIN, OUTPUT); // set as output.
  digitalWrite(RELAY_PIN, HIGH); // set as initially closed. 

  /* Setup Serial communication */
  Serial.begin(115200);
  while(!Serial) {} 

  /* Wifi connection */
  WiFi.setAutoConnect(true);
  connectWifi();

  /* Set time 
   *  We need date/time to perform the TLS handshake.
   */
  Serial.println("Seting up time.");
  timeClient.begin();
  while(!timeClient.update()) 
    timeClient.forceUpdate();
  
  tlsClient.setX509Time(timeClient.getEpochTime());
  Serial.print("Time set: ");
  Serial.println(timeClient.getFormattedTime());

  /* Configure secure client connection. */
  tlsClient.setTrustAnchors(&x509);
  tlsClient.allowSelfSignedCerts();
  tlsClient.setFingerprint(mqttCertFingerprint);
  
  /* MQTT server setup. */
  mqtts_client.setServer(MQTT_SERVER, TLS_PORT);
  mqtts_client.setCallback(callback);

  /* Set the transmission buffer size
   *    To make sure no chunk of transmitted data will overflow the 
   *    available memory.
   *    Using 512 KB buffers.
   */
  tlsClient.setBufferSizes(512,512);

  connectMQTT();
}

void loop() {

  // Do not blink on operational status. 
  if (WiFi.status() == WL_CONNECTED)
    digitalWrite(builtinLed(variant), LOW);
  else 
    connectWifi();

  // Check mqtt connectivity.
  if (!mqtts_client.connected())
    connectMQTT();
  mqtts_client.loop();
}
