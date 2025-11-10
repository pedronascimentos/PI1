// Programa: Controle de direção de dois motores DC via monitor serial (L298N)

// Define os pinos de controle do motor esquerdo
#define IN1 27  // Sentido horário (frente)
#define IN2 26  // Sentido anti-horário (ré)

// Define os pinos de controle do motor direito
#define IN3 25  // Sentido horário (frente)
#define IN4 33  // Sentido anti-horário (ré)

int velocidade = 100;  // Velocidade média (0–255)

void frente (int tempo) {
  analogWrite(IN1, velocidade);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, velocidade);

  delay(tempo);

  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, 0);
}

void re (int tempo) {
  analogWrite(IN1, 0);
  analogWrite(IN2, velocidade);
  analogWrite(IN3, velocidade);
  analogWrite(IN4, 0);

  delay(tempo);

  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, 0);
}

void girar(int dir){
  if (dir == 0){
    analogWrite(IN1, 0);
    analogWrite(IN2, velocidade-25);
    analogWrite(IN3, 0);
    analogWrite(IN4, velocidade-25);

    delay(1000);

    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
    }
  if (dir == 1){
    analogWrite(IN1, velocidade-25);
    analogWrite(IN2, 0);
    analogWrite(IN3, velocidade-25);
    analogWrite(IN4, 0);

    delay(1000);

    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
    }
}

void setup() {
  // Configura os pinos de controle como saída
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Inicializa a comunicação serial
  Serial.begin(115200);
  Serial.println("Controle do carrinho iniciado!");
  Serial.println("Comandos: D = frente, R = ré, P = parar");
}

void loop() {
  // Verifica se há dados disponíveis no monitor serial
  if (Serial.available() > 0) {
    char comando = Serial.read();  // Lê o caractere enviado

    if (comando == 'F' || comando == 'f') {
      frente(1000);
    }
    if (comando == 'R' || comando == 'r') {
      re(1000);
    }
    if (comando == 'D' || comando == 'd') {
      girar(1);
    }
    if (comando == 'E' || comando == 'e') {
      girar(0);
    }
  }
}
