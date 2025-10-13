// -------------------- Módulo L298D --------------------
#define PINO_IN1 27  // Sentido horário
#define PINO_IN2 26  // Sentido anti-horário

// -------------------- Sensor Ultrassônico --------------------
#define trigPin 5
#define echoPin 18
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

void setup() { 
  // Motor
  pinMode(PINO_IN1, OUTPUT);
  pinMode(PINO_IN2, OUTPUT);
  
  // Sensor ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(115200);
}

void loop() {
  // -------------------- Leitura do Ultrassônico --------------------
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;
  distanceInch = distanceCm * CM_TO_INCH;

  Serial.print("Distância (cm): ");
  Serial.println(distanceCm);

  // -------------------- Controle do Motor --------------------
  if (distanceCm > 10) {
    int valor_pwm;

    // Roda infinito pra fretnte
      analogWrite(PINO_IN1, 255);
      analogWrite(PINO_IN2, 0);

  } else {
    // Se tiver perto ele começa a dar ré
    analogWrite(PINO_IN1, 0);
    analogWrite(PINO_IN2, 40);
    Serial.println("Objeto próximo — motor parado");
    delay(500);
  }

  delay(500);
}
