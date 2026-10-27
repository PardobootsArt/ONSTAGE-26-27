# python "C:\Users\Instrutor\Documents\GitHub\ONSTAGE-26-27\NACIONAL\CAPSULA\capsula.nova.py"     

import sounddevice as sd
import numpy as np
import serial
import time

PORTA_SERIAL = "COM7"

duracao = 0.2
limite = 0.4
ultimo_grito = 0
intervalo_gritos = 2.0

print(sd.query_devices())

try:
    with serial.Serial(PORTA_SERIAL, 9600, timeout=1) as arduino:
        print("A ligar ao Arduino...")
        time.sleep(2)
        print("Arduino ligado!")

        while True:
            audio = sd.rec(
                int(duracao * 44100),
                samplerate=44100,
                channels=1
            )
            sd.wait()

            rms = np.sqrt(np.mean(audio ** 2))
            print("RMS:", rms)

            agora = time.time()

            if rms >= limite and agora - ultimo_grito >= intervalo_gritos:
                print("GRITO DETECTADO")
                arduino.write(b'GRITO')
                ultimo_grito = agora

except KeyboardInterrupt:
    print("\nPrograma terminado.")

except Exception as erro:
    print("Erro:", erro)

