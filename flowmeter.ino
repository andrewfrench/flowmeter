#define TICKS_PER_OZ 1000

#include <SPI.h>
#include <WiFi.h>
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <WiFiCredentials.h>

WiFiCredentials credentials;
WifiIPStack ipstack;
MQTT::Client<WifiIPStack, Countdown> client = MQTT::Client<WifiIPStack, Countdown>(ipstack);
MQTT:Message message;
message.qos = MQTT::QOS0;
message.retained = false;
message.dup = false;

const char* topic = "public/flowmeter/test";
char printbuf[100];
char mqttbuf[100];
int meter_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"flowmeter";
  rc = client.connect(data);
  if (rc != 0) {
    sprintf(printbuf, "rc from MQTT connect is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT connected");
}

void meter_one() {
  meter_count[1]++;

  digitalWrite(RED_LED, HIGH);
  delay(50);
  digitalWrite(RED_LED, LOW);

  if(meter_count[1] > last_count_one + PUBLISH_THRESHOLD){
    sendMessage(1, "public/flowmeter/meter-1");
    last_count_one = meter_count[1];
  }
}

void check_meter(int meter) {
  // Blink the red LED to give some feedback
  blinkRed();
  if(meter_count[meter] >= TICKS_PER_OZ) {
    meter_count[meter] = 0;
    sendMessage(meter, )
  }
}

void meter0() {
  meter_count[0]++;
  blinkRed();
  checkMeter(0);
}
void meter1() {
  meter_count[1]++;
  blinkRed();
  checkMeter(1);
}
void meter2() {
  meter_count[2]++;
  blinkRed();
  checkMeter(2);
}
void meter3() {
  meter_count[3]++;
  blinkRed();
  checkMeter(3);
}
void meter4() {
  meter_count[4]++;
  blinkRed();
  checkMeter(4);
}
void meter5() {
  meter_count[5]++;
  blinkRed();
  checkMeter(5);
}
void meter6() {
  meter_count[6]++;
  blinkRed();
  checkMeter(6);
}
void meter7() {
  meter_count[7]++;
  blinkRed();
  checkMeter(7);
}
void meter8() {
  meter_count[8]++;
  blinkRed();
  checkMeter(8);
}
void meter9() {
  meter_count[9]++;
  blinkRed();
  checkMeter(9);
}

void sendMessage(int meter) {
  Serial.println("Sending MQTT Message...");

  sprintf(mqttbuf, "{'meter': %d}", meter);
  message.payload = (void*)mqttbuf;
  message.payloadlen = strlen(mqttbuf);
  client.publish(topic, message);

  Serial.println("Message send complete.");
}

void pinSetup() {
  // GPIO_00
  pinMode(PIN_50, INPUT_PULLUP);
  attachInterrupt(PIN_50, meter0, RISING);

  // GPIO_01
  pinMode(PIN_55, INPUT_PULLUP);
  attachInterrupt(PIN_55, meter1, RISING);

  // GPIO_02
  pinMode(PIN_57, INPUT_PULLUP);
  attachInterrupt(PIN_57, meter2, RISING);

  // GPIO_03
  pinMode(PIN_58, INPUT_PULLUP);
  attachInterrupt(PIN_58, meter3, RISING);

  // GPIO_04
  pinMode(PIN_59, INPUT_PULLUP);
  attachInterrupt(PIN_59, meter4, RISING);

  // GPIO_05
  pinMode(PIN_60, INPUT_PULLUP);
  attachInterrupt(PIN_60, meter5, RISING);

  // GPIO_06
  pinMode(PIN_61, INPUT_PULLUP);
  attachInterrupt(PIN_61, meter6, RISING);

  // GPIO_07
  pinMode(PIN_62, INPUT_PULLUP);
  attachInterrupt(PIN_62, meter7, RISING);

  // GPIO_08
  pinMode(PIN_63, INPUT_PULLUP);
  attachInterrupt(PIN_63, meter8, RISING);

  // GPIO_09
  pinMode(PIN_64, INPUT_PULLUP);
  attachInterrupt(PIN_64, meter9, RISING);
}

void initWifi() {
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");

  // print the network name (SSID);
  Serial.println(credentials.get_ssid());

  WiFi.begin(credentials.get_ssid(), credentials.get_password());

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
