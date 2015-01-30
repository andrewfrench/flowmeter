#define MQTTCLIENT_QOS2 1
#define PUBLISH_THRESHOLD 1000

#include <SPI.h>
#include <WiFi.h>
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>



// your network name also called SSID
char ssid[] = "2lemetry";
// your network password
char password[] = "connecteverydevice";

char printbuf[100];

int arrivedcount = 0;

int meter_count[] = {0, 0, 0};
int last_count_one = 0;
int last_count_two = 0;

void blinkGreen(int times) {
  for(int i = 0; i < times; i++) {
    digitalWrite(GREEN_LED, HIGH);
    delay(50);
    digitalWrite(GREEN_LED, LOW);
    delay(50);
  }
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
WifiIPStack ipstack;
MQTT::Client<WifiIPStack, Countdown> client = MQTT::Client<WifiIPStack, Countdown>(ipstack);

const char* topic = "public/flowmeter/test";

void connect()
{
  char hostname[] = "q.thingfabric.com";
  int port = 1883;
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  int rc = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }
 
  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"flowmeter";
  rc = client.connect(data);
  if (rc != 0)
  {
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

void meter_two() {
  meter_count[2]++;
  
  digitalWrite(YELLOW_LED, HIGH);
  delay(50); 
  digitalWrite(YELLOW_LED, LOW);
  
  if(meter_count[2] > last_count_two + PUBLISH_THRESHOLD){
    sendMessage(2, "public/flowmeter/meter-2");
    last_count_two = meter_count[2];
  }
}

void sendMessage(int meter, const char* topic) {
  MQTT::Message message;

  // Send and receive QoS 0 message
  char buf[100];
  float meter_level = meter_count[meter] / 5600.0;
  sprintf(buf, "{'liters': %.4f}", meter_level);
  Serial.println(buf);
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.dup = false;
  message.payload = (void*)buf;
  message.payloadlen = strlen(buf);
  int rc = client.publish(topic, message);
}

void setup()
{
  // Set up pin input and output
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(PUSH2, INPUT_PULLUP);
  pinMode(PUSH1, INPUT_PULLUP);
  
  // we'll save the attaching of the interrupts for
  // the end of setup(), after all connections have
  // been made.
  
  // Init LEDs as off
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  
  Serial.begin(115200);
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  
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
  
  Serial.println("MQTT Hello example");
  connect();

  attachInterrupt(PUSH1, meter_one, RISING);  
  attachInterrupt(PUSH2, meter_two, RISING);
}

void loop()
{
  if (!client.isConnected())
    connect();
}
