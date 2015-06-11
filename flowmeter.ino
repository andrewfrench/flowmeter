#define TI_BOARD_NUMBER 0
#define TICKS_PER_OZ 170

#include <SPI.h>
#include <WiFi.h>
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
// #include <WiFiCredentials.h>

// WiFiCredentials credentials;
#define WIFI_SSID "SWB"
#define WIFI_PASSWORD "370samples"

WifiIPStack ipstack;
MQTT::Client<WifiIPStack, Countdown> client = MQTT::Client<WifiIPStack, Countdown>(ipstack);

const char* topic = "ceyjppuqf51jmon/flowmeter/ounce";
char printbuf[100];
char mqttbuf[100];
volatile int meter_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void blinkGreen(int times) {
  for(int i = 0; i < times; i++) {
    digitalWrite(GREEN_LED, HIGH);
    delay(50);
    digitalWrite(GREEN_LED, LOW);
    delay(50);
  }
}

void blinkRed() {
  digitalWrite(RED_LED, HIGH);
  delay(50);
  digitalWrite(RED_LED, LOW);
}

void blinkRedForever() {
  for(;;) {
    digitalWrite(RED_LED, HIGH);
    delay(50);
    digitalWrite(RED_LED, LOW);
    delay(50);
  }
}

void blinkYellow() {
  for(int i = 0; i < 2; i++) {
    digitalWrite(YELLOW_LED, HIGH);
    delay(10);
    digitalWrite(YELLOW_LED, LOW);
    delay(40);
  }
}

void connect() {
  char hostname[] = "q.thingfabric.com";
  int port = 1883;
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  int rc = ipstack.connect(hostname, port);
  if (rc != 1) {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }

  Serial.println("MQTT connecting...");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"flowmeter";
  data.username.cstring = (char*)"8a857b0b-6307-4dc9-ac50-d0895f92f976";
  data.password.cstring = (char*)"80de655641d43752cf26161a9cfb075e";
  rc = client.connect(data);
  if (rc != 0) {
    sprintf(printbuf, "rc from MQTT connect is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT connection complete.");
}

void checkMeter(int meter) {
  // Blink the red LED to give some feedback
  blinkRed();
  if(meter_count[meter] >= TICKS_PER_OZ) {
    meter_count[meter] = 0;
    sendMessage(meter);
  }
}

void meter0() {
  meter_count[0]++;
  checkMeter(0);
}
void meter1() {
  meter_count[1]++;
  checkMeter(1);
}

void sendMessage(int meter) {
  Serial.println("Sending MQTT Message...");

  MQTT::Message message;
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.dup = false;

  sprintf(mqttbuf, "{'board': %d, 'meter': %d}", TI_BOARD_NUMBER, meter);
  message.payload = (void*)mqttbuf;
  message.payloadlen = strlen(mqttbuf);
  client.publish(topic, message);

  Serial.println("Message send complete.");
}

void pinSetup() {
  Serial.println("Setting up pins...");

  // Button 1
  pinMode(PUSH1, INPUT_PULLUP);
  attachInterrupt(PUSH1, meter0, RISING);
  Serial.println("PUSH1 interrupt created.");

  // Button 2
  pinMode(PUSH2, INPUT_PULLUP);
  attachInterrupt(PUSH2, meter1, RISING);
  Serial.println("PUSH2 interrupt created.");

  Serial.println("Pin setup complete.");
}

void initWifi() {
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");

  // print the network name (SSID);
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int loops = 0;
  while ( WiFi.status() != WL_CONNECTED && loops < 30) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
    blinkYellow();
    loops++;
  }

  // check if timeout threshold met
  if(loops == 30) {
    blinkRedForever();
  }

  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");

  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  Serial.println(WiFi.localIP());

  blinkGreen(10);

}

void setup() {
  // Set up pin input and output
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Init LEDs as off
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  Serial.begin(115200);

  initWifi();

  connect();

  pinSetup();
}

void loop() {
  if (!client.isConnected()) {
    connect();
  }
}
