#include <Servo.h>

#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#define MSG_BUFFER_SIZE(50)

IPAddress mqtt_server(192, 100, 247, 3);
WiFiClient espClient;

PubSubClient client(espClient);

const int VD1 = 5;
const int VD2 = 4;
const int VD3 = 0;
const int VD4 = 2;
const int VD5 = 14;

int SODistCm = 0;
const int SOTrigPin = VD1;
const int SOEchoPin = VD2;

int STDistCm = 0;
const int STTrigPin = VD3;
const int STEchoPin = VD4;

Servo servo;
int servoPosition = 0;
const int servoPin = VD5;
const int minimunAngle = 500;
const int maximunAngle = 2500;
const int angleToRotate = 90;
const int delayToRotateInMs = 5;

const int distToOpenCm = 10;
const int delayToReturnCloseInMs = 5000;

const int mqttPort = 11883;
const char * ssid = "AULA_IOT";
const char * password = "unismg123!";
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
    if (client.connect("grupo04", "alunos", "$3nh@321!")) {
      Serial.println(" Conectado");
      // client.publish("grupo04", "teste");
      // client.subscribe(topic);
    } else {
      Serial.print(" Falha na Conexão, rc=");
      Serial.print(client.state());
      Serial.println(" tente novamente em 3 segundos");
      delay(3000);
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
  client.setServer(mqtt_server, mqttPort);
  client.setCallback(callback);
}

void loop() {
  // if (!client.connected()) {
  //   reconnect();
  // }
  // client.loop();

  SODistCm = 0.01723 * readUltrasonicDistance(SOTrigPin, SOEchoPin);
  Serial.print("sensor one distance in cm: ");
  Serial.println(SODistCm);

  STDistCm = 0.01723 * readUltrasonicDistance(STTrigPin, STEchoPin);
  Serial.print("sensor two distance in cm: ");
  Serial.println(STDistCm);

  if (SODistCm <= distToOpenCm) {
    Serial.println("servo triggered !");
    rotateClockwise();
    delay(delayToReturnCloseInMs);
    rotateCounterClockwise();
  }

  if (STDistCm <= 15) {
    Serial.println("enviar mensagem pro MQTT");
    // client.publish(topic, "lixeira cheia");
  }

  delay(500);
}