// Definición de pines
const int PIN_CARGA = 7;     // Carga el capacitor a través de las resistencias y LEDs
const int PIN_DESCARGA = 8;  // Descarga el capacitor hacia tierra (Columna 30)
const int PIN_LECTURA = A0;  // Mide el voltaje del capacitor (Columna 30)

// Tiempos de muestreo
const unsigned long INTERVALO_MUESTREO = 2000;   // 2 ms (500 muestras por segundo)
const unsigned long TIEMPO_FASE = 1000000;       // 1 segundo para carga, 1 segundo para descarga

unsigned long tiempoInicioFase = 0;
unsigned long ultimoMuestreo = 0;
int estadoActual = 1; // 1 = Carga, 2 = Descarga

void setup() {
  Serial.begin(115200);
  
  // Descarga inicial para empezar desde 0 voltios
  pinMode(PIN_CARGA, OUTPUT);
  digitalWrite(PIN_CARGA, LOW);
  pinMode(PIN_DESCARGA, OUTPUT);
  digitalWrite(PIN_DESCARGA, LOW);
  delay(1000);

  // Encabezado CSV para Python
  Serial.println("tiempo_us,fase,adc_valor");
  
  // Arrancamos en CARGA: Pin 8 en INPUT (alta impedancia) para que no robe corriente
  pinMode(PIN_DESCARGA, INPUT);
  digitalWrite(PIN_CARGA, HIGH);
  tiempoInicioFase = micros();
}

void loop() {
  unsigned long tiempoActual = micros();

  // Alterna entre CARGA y DESCARGA cada 1 segundo
  if (tiempoActual - tiempoInicioFase >= TIEMPO_FASE) {
    if (estadoActual == 1) {
      // Cambia a DESCARGA: apaga Pin 7 y activa Pin 8 como salida a 0V (LOW)
      estadoActual = 2;
      digitalWrite(PIN_CARGA, LOW);
      pinMode(PIN_DESCARGA, OUTPUT);
      digitalWrite(PIN_DESCARGA, LOW);
      tiempoInicioFase = tiempoActual;
    } else {
      // Cambia a CARGA: libera Pin 8 (INPUT) y enciende Pin 7 a 5V (HIGH)
      estadoActual = 1;
      pinMode(PIN_DESCARGA, INPUT);
      digitalWrite(PIN_CARGA, HIGH);
      tiempoInicioFase = tiempoActual;
    }
  }

  // Captura de los datos cada 2 milisegundos
  if (tiempoActual - ultimoMuestreo >= INTERVALO_MUESTREO) {
    ultimoMuestreo = tiempoActual;
    int lecturaADC = analogRead(PIN_LECTURA);
    unsigned long tiempoRelativo = tiempoActual - tiempoInicioFase;

    Serial.print(tiempoRelativo);
    Serial.print(",");
    Serial.print(estadoActual == 1 ? "CARGA" : "DESCARGA");
    Serial.print(",");
    Serial.println(lecturaADC);
  }
}
