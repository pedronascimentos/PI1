// Pino conectado ao D0 do FC-03
int sensorPin = 19;   
int estadoSensor = 0;
int a=0, furos = 20;

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
}

void loop() {
  estadoSensor = digitalRead(sensorPin);
  
  if (estadoSensor == HIGH) {
    a++;
    Serial.println("Vibração detectada!");
    Serial.println(a);
  } else {
    Serial.println("Sem vibração.");
  }

  delay(300);
}
