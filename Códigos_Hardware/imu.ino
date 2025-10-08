#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Variáveis para o eixo Z
float anguloZ = 0;
float anguloZAnterior = 0;
unsigned long tempoAnterior = 0;

// Variáveis de calibração
float offsetGyroZ = 0;
float driftGyroZ = 0;
bool calibrado = false;

// Configurações
const int NUM_LEITURAS_CALIBRACAO = 1000;
const float VARIACAO_MINIMA = 0.5; // 🔥 Alterado para 0.5 grau mínimo
const unsigned long INTERVALO_CORRECAO_DRIFT = 3000; // 3 segundos

// Controle de atualização
bool primeiraLeitura = true;
unsigned long ultimaCorrecaoDrift = 0;
unsigned long ultimaAtualizacao = 0;

void setup(void) {
  Serial.begin(115200);
  
  // Tenta inicializar o MPU6050
  if (!mpu.begin()) {
    Serial.println("Falha ao encontrar o chip MPU6050");
    while (1) {
      delay(10);
    }
  }
  
  Serial.println("MPU6050 encontrado!");
  
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  // Realiza calibração específica para o eixo Z
  calibrarEixoZ();
  
  tempoAnterior = micros();
  ultimaCorrecaoDrift = millis();
  ultimaAtualizacao = millis();
}

void calibrarEixoZ() {
  Serial.println();
  Serial.println("=== CALIBRAÇÃO DO EIXO Z ===");
  Serial.println("Mantenha o sensor COMPLETAMENTE PARADO!");
  Serial.println("Calibrando em 3...");
  delay(1000);
  Serial.println("2...");
  delay(1000);
  Serial.println("1...");
  delay(1000);
  Serial.println("Calibrando eixo Z...");
  
  float somaGyroZ = 0;
  
  for (int i = 0; i < NUM_LEITURAS_CALIBRACAO; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    somaGyroZ += g.gyro.z;
    
    if (i % (NUM_LEITURAS_CALIBRACAO / 10) == 0) {
      Serial.print("Progresso: ");
      Serial.print((i * 100) / NUM_LEITURAS_CALIBRACAO);
      Serial.println("%");
    }
    
    delay(10);
  }
  
  // Calcula offset e drift inicial
  offsetGyroZ = somaGyroZ / NUM_LEITURAS_CALIBRACAO;
  driftGyroZ = offsetGyroZ; // Drift inicial é o offset
  
  Serial.println();
  Serial.println("=== CALIBRAÇÃO CONCLUÍDA ===");
  Serial.print("Offset Gyro Z: ");
  Serial.print(offsetGyroZ, 6);
  Serial.print(" rad/s (");
  Serial.print(offsetGyroZ * 180/PI, 2);
  Serial.println(" °/s)");
  Serial.print("Filtro ativo: ");
  Serial.print(VARIACAO_MINIMA, 1);
  Serial.println("°");
  
  calibrado = true;
  Serial.println("Pronto para medir rotação no eixo Z!");
  Serial.println();
  delay(1000);
}

void corrigirDriftEixoZ() {
  // Correção de drift baseada na detecção de movimento
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  float gyroZ_calibrado = g.gyro.z - offsetGyroZ;
  
  // Se a leitura do giroscópio é muito baixa, provavelmente está parado
  if (abs(gyroZ_calibrado) < 0.03) { // 0.03 rad/s ≈ 1.72 °/s
    // Atualiza estimativa de drift suavemente
    driftGyroZ = driftGyroZ * 0.98 + gyroZ_calibrado * 0.02;
  }
}

float aplicarFiltroVariacao(float novoAngulo, float anguloAnterior) {
  // Só atualiza se a variação for maior que 0.5°
  if (abs(novoAngulo - anguloAnterior) >= VARIACAO_MINIMA) {
    return novoAngulo;
  }
  return anguloAnterior;
}

void loop() {
  if (!calibrado) {
    Serial.println("Sensor não calibrado!");
    delay(1000);
    return;
  }
  
  // Verifica comandos pela serial
  verificarComandos();
  
  // Corrige drift periodicamente
  if (millis() - ultimaCorrecaoDrift > INTERVALO_CORRECAO_DRIFT) {
    corrigirDriftEixoZ();
    ultimaCorrecaoDrift = millis();
  }
  
  // Obtém dados do sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Calcula delta time
  unsigned long tempoAtual = micros();
  float dt = (tempoAtual - tempoAnterior) / 1000000.0;
  tempoAnterior = tempoAtual;
  
  // Aplica calibração no eixo Z
  float gyroZ_calibrado = g.gyro.z - offsetGyroZ - driftGyroZ;
  
  // Calcula nova posição angular
  float variacaoAngular = gyroZ_calibrado * dt * 180 / PI; // Converte para graus
  float novoAnguloZ = anguloZ + variacaoAngular;
  
  // Mantém o ângulo entre 0-360 graus
  if (novoAnguloZ >= 360) novoAnguloZ -= 360;
  if (novoAnguloZ < 0) novoAnguloZ += 360;
  
  // Aplica filtro de variação mínima (0.5°)
  anguloZAnterior = anguloZ;
  anguloZ = aplicarFiltroVariacao(novoAnguloZ, anguloZ);
  
  // Detecta se houve atualização
  bool atualizou = (anguloZ != anguloZAnterior);
  
  // Exibe resultados apenas quando houver atualização ou a cada 2 segundos
  bool deveExibir = atualizou || primeiraLeitura || (millis() - ultimaAtualizacao > 2000);
  
  if (deveExibir) {
    Serial.print("Yaw (Z): ");
    Serial.print(anguloZ, 2); // 🔥 Mais casas decimais para ver melhor os 0.5°
    Serial.print("° | Variação: ");
    Serial.print(variacaoAngular, 3); // 🔥 Mais precisão na variação
    Serial.print("° | Drift: ");
    Serial.print(driftGyroZ * 180/PI, 3);
    Serial.print("°/s");
    
    if (atualizou) {
      Serial.print(" | ✅ ATUALIZOU");
    } else {
      Serial.print(" | ⏸️  ESTÁVEL");
    }
    
    // Mostra há quanto tempo não atualiza
    if (!atualizou && !primeiraLeitura) {
      Serial.print(" | Tempo estável: ");
      Serial.print((millis() - ultimaAtualizacao) / 1000);
      Serial.print("s");
    }
    
    Serial.println();
    
    if (atualizou || primeiraLeitura) {
      ultimaAtualizacao = millis();
    }
    
    if (primeiraLeitura) {
      primeiraLeitura = false;
    }
  }
  
  delay(50); // Loop principal mais rápido
}

// Função para resetar o ângulo Z
void resetarYaw() {
  anguloZ = 0;
  anguloZAnterior = 0;
  Serial.println("Yaw resetado para 0°");
}

// Função para calibrar novamente
void recalibrarEixoZ() {
  calibrado = false;
  anguloZ = 0;
  anguloZAnterior = 0;
  calibrarEixoZ();
}

// Verifica comandos pela serial
void verificarComandos() {
  if (Serial.available()) {
    char comando = Serial.read();
    switch (comando) {
      case 'r':
        resetarYaw();
        break;
      case 'c':
        recalibrarEixoZ();
        break;
      case 'd':
        Serial.print("Drift atual: ");
        Serial.print(driftGyroZ * 180/PI, 4);
        Serial.println(" °/s");
        break;
      case 'f':
        Serial.print("Filtro atual: ");
        Serial.print(VARIACAO_MINIMA, 1);
        Serial.println("°");
        break;
      case 's':
        Serial.println("=== STATUS ===");
        Serial.print("Yaw atual: ");
        Serial.print(anguloZ, 2);
        Serial.println("°");
        Serial.print("Filtro: ");
        Serial.print(VARIACAO_MINIMA, 1);
        Serial.println("°");
        Serial.print("Drift: ");
        Serial.print(driftGyroZ * 180/PI, 4);
        Serial.println(" °/s");
        Serial.println("==============");
        break;
    }
  }
}