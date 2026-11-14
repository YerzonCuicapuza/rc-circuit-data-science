import os
import time
import serial
import pandas as pd

PUERTO = "COM5"
BAUD_RATE = 115200
SEGUNDOS_CAPTURA = 6

def capturar_datos():
    print(f"Conectando a {PUERTO}...")
    try:
        ser = serial.Serial(PUERTO, BAUD_RATE, timeout=1)
        time.sleep(2)
        ser.reset_input_buffer()
    except Exception as e:
        print(f"Error al abrir {PUERTO}. Revisa que el Arduino esté conectado y Arduino IDE cerrado.\nDetalle: {e}")
        return

    print(f"Capturando datos durante {SEGUNDOS_CAPTURA} segundos... ¡No desconectes el cable!")
    registros = []
    inicio = time.time()

    while (time.time() - inicio) < SEGUNDOS_CAPTURA:
        if ser.in_waiting > 0:
            linea = ser.readline().decode("utf-8", errors="ignore").strip()
            partes = linea.split(",")
            
            if len(partes) == 3 and partes[0].isdigit() and partes[2].isdigit():
                registros.append({
                    "tiempo_us": int(partes[0]),
                    "fase": partes[1],
                    "adc_valor": int(partes[2])
                })

    ser.close()
    print(f"¡Listo! Se recolectaron {len(registros)} muestras.")

    if len(registros) == 0:
        print("No llegaron datos. Revisa la conexión USB.")
        return

    os.makedirs("data/raw", exist_ok=True)
    os.makedirs("data/processed", exist_ok=True)

    df_raw = pd.DataFrame(registros)
    df_raw.to_csv("data/raw/rc_raw_data.csv", index=False)

    df_clean = df_raw.copy()
    df_clean["tiempo_s"] = df_clean["tiempo_us"] / 1_000_000.0
    df_clean["voltaje_v"] = df_clean["adc_valor"] * (5.0 / 1023.0)
    df_clean["ciclo"] = (df_clean["tiempo_us"].diff() < 0).cumsum() + 1

    df_clean.to_csv("data/processed/rc_clean_data.csv", index=False)
    print("\n[OK] Archivo guardado en: data/processed/rc_clean_data.csv")
    print(df_clean.head())

if __name__ == "__main__":
    capturar_datos()