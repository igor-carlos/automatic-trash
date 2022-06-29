#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define MSG_BUFFER_SIZE 50

IPAddress mqtt_server(192, 100, 247, 3);
WiFiClient espClient;
// 35.157.18.117

PubSubClient client(espClient);

// o roxo é no vu
// o marrom é no 3v

const int VD1 = 5;
const int VD2 = 4;
const int VD3 = 0;
const int VD4 = 2;
const int VD5 = 14;

int SODistCm = 0;
const int SOTrigPin = VD3;
const int SOEchoPin = VD4;

int STDistCm = 0;
const int STTrigPin = VD1;
const int STEchoPin = VD2;

Servo servo;
int servoPosition = 0;
const int servoPin = VD5;
const int minimunAngle = 500;
const int maximunAngle = 2500;
const int angleToRotate = 20;
const int delayToRotateInMs = 5;

const int distToOpenCm = 10;
const int distMaxToSentMessage = 20;
const int distMinToSentMessage = 10;
const int delayToReturnCloseInMs = 5000;

const int mqttPort = 11883;
const char * ssid = "Tone 2.4 GHz";
const char * password = "tone1970";
char msg[MSG_BUFFER_SIZE];
const char * topic = topic;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char * topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentativa MQTT para conexão...");
    if (client.connect("b547a30b-5110-4f2a-8d01-ec137257ffc6","mqttserverunisbilly","billy$#@!4321")) {
      Serial.println(" Conectado");
      // client.publish("grupo04", "teste");
      // client.subscribe(topic);
    } else {
      Serial.print(" Falha na Conexão, rc=");
      Serial.print(client.state());
      Serial.println(" tente novamente em 1 segundos");
      delay(1000);
    }
  }
}

long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH);
}

void rotateClockwise() {
  for (servoPosition = 0; servoPosition <= angleToRotate; servoPosition += 1) {
    servo.write(servoPosition);
    delay(delayToRotateInMs);
  }
}

void rotateCounterClockwise() {
  for (servoPosition = angleToRotate; servoPosition >= 0; servoPosition -= 1) {
    servo.write(servoPosition);
    delay(delayToRotateInMs);
  }
}

void setup() {
  Serial.begin(9600);

  servo.attach(servoPin, minimunAngle, maximunAngle);

  setup_wifi();
  const char *xmqtt_broker = "broker.hivemq.com";
  // client.setServer(xmqtt_broker, mqttPort);
  client.setServer(xmqtt_broker, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  SODistCm = 0.01723 * readUltrasonicDistance(SOTrigPin, SOEchoPin);
  Serial.print("sensor one distance in cm: ");
  Serial.println(SODistCm);

  STDistCm = 0.01723 * readUltrasonicDistance(STTrigPin, STEchoPin);
  Serial.print("sensor two distance in cm: ");
  Serial.println(STDistCm);

  if (SODistCm <= distToOpenCm) {
    Serial.println("servo triggered !");
    rotateCounterClockwise();
    delay(delayToReturnCloseInMs);
    rotateClockwise();
  }

  if (STDistCm <= distMinToSentMessage || STDistCm >= distMaxToSentMessage) {
    // client.subscribe("grupo04/estadoLixeira")
    client.publish("grupo04/estadoLixeira", "cheia");
    Serial.println("Enviou que está cheia");
  } else {
    client.publish("grupo04/estadoLixeira", "vazia");
    Serial.println("Enviou que está vazia");
  }

  delay(500);
}