/*
 * Código para ESP32 - Medidor de Tensão de Bateria
 * * Este código lê a tensão de uma bateria (ex: LiPo 4.2V) usando um 
 * divisor de tensão e o ADC (Conversor Analógico-Digital) do ESP32.
 * * --- Hardware ---
 * * Bateria (+) --- [R1] --+-- [R2] --- GND
 * |
 * GPIO 34
 * * R1 e R2 formam um divisor de tensão.
 * Use valores iguais (ex: R1=100k, R2=100k) para dividir a tensão por 2.
 * Isso garante que a tensão máxima da bateria (4.2V) se torne 2.1V,
 * que é seguro para o ESP32 (que só aceita até 3.3V).
 * */

// --- Configurações do Hardware ---

// Defina o pino do ADC que você está usando.
// Pinos do ADC1 (GPIO 32-39) são recomendados para usar com Wi-Fi.
#define BATTERY_PIN 34

// Defina os valores dos resistores do seu divisor de tensão (em Ohms)
// Use .0 para forçar o cálculo com números decimais (float)
#define R1 470000.0 // 470k Ohms
#define R2 100000.0 // 100k Ohms

// O LED "Onboard" da maioria das placas ESP32 é no pino 2
#define LED_PIN 2

// --- Variáveis Globais ---
float batteryVoltage = 0.0;


void setup() {
  // Inicia a comunicação serial (115200 é uma velocidade padrão para o ESP32)
  Serial.begin(115200);

  // Configura o pino do LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Acende o LED para indicar que a placa ligou

  // Configura a "Atenuação" do ADC.
  // ISSO É ESSENCIAL! Por padrão, o ADC só lê até 1.1V.
  // ADC_ATTEN_DB_11 faz com que ele consiga ler até aprox. 3.3V.
  analogSetAttenuation(ADC_ATTEN_DB_11);
  
  // Define a resolução do ADC para 12 bits (0-4095).
  // Isso melhora a precisão da leitura.
  analogSetWidth(12);

  // É uma boa prática definir o pino como entrada
  pinMode(BATTERY_PIN, INPUT);
}


void loop() {
  // Chama a função que lê a tensão
  readBatteryVoltage();

  // Imprime o valor no Monitor Serial
  Serial.print("Tensão da Bateria: ");
  Serial.print(batteryVoltage); // Imprime o valor calculado
  Serial.println(" V");

  // Espera 1 segundo antes de medir novamente
  delay(1000);
}


/**
 * @brief Lê o ADC e calcula a tensão real da bateria.
 */
void readBatteryVoltage() {
  // "analogReadMilliVolts" é uma função específica do ESP32 que é
  // mais precisa, pois usa a calibração de fábrica do chip.
  // Ela retorna o valor lido pelo pino em milivolts (mV).
  uint32_t adc_mv = analogReadMilliVolts(BATTERY_PIN);

  // Converte o valor lido (Vout do divisor) de milivolts para volts
  float v_out = adc_mv / 1000.0;

  // --- A Mágica do Divisor de Tensão ---
  // A fórmula para "descobrir" a tensão original (Vin) é:
  // Vin = Vout * (R1 + R2) / R2
  //
  // Se R1 e R2 são iguais (100k), a fórmula vira:
  // Vin = Vout * (100k + 100k) / 100k
  // Vin = Vout * (200k) / 100k
  // Vin = Vout * 2
  //
  batteryVoltage = v_out * (R1 + R2) / R2;

  // Se você usou R1=100k e R2=100k, você pode simplificar para:
  // batteryVoltage = v_out * 2.0;

  // Se você quiser mais precisão, pode precisar de um fator de calibração.
  // Meça a tensão real da bateria com um multímetro e ajuste.
  // Ex: Se o multímetro diz 4.1V e o serial diz 4.0V, 
  //    o fator é 4.1 / 4.0 = 1.025
  // batteryVoltage = batteryVoltage * 1.025;
}