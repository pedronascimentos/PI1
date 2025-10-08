int sensorPin = 19;
volatile int pulsos = 0;
unsigned long tempoAnterior = 0;
float diametroRoda = 0.065; // 6,5 cm de diâmetro
int furos = 20; // número de furos no disco

void IRAM_ATTR contarPulso() {
  pulsos++;
}

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), contarPulso, RISING);
}

void loop() {
  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnterior >= 1000) { // a cada 1 segundo
    noInterrupts();
    int contagem = pulsos;
    pulsos = 0;
    interrupts();

    // Calcular rotações por segundo
    float rps = contagem / (float)furos;  
    float rpm = rps * 60.0;

    // Circunferência da roda
    float circunferencia = 3.1416 * diametroRoda;

    // Velocidade em m/s = rps * circunferência
    float velocidade_ms = rps * circunferencia;


    Serial.print("Velocidade: ");
    Serial.print(velocidade_ms, 2);
    Serial.println(" m/s");

    tempoAnterior = tempoAtual;
  }
}
