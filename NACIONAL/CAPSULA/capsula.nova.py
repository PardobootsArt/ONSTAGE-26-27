# python "C:\Users\Instrutor\Documents\GitHub\ONSTAGE-26-27\NACIONAL\CAPSULA\capsula.nova.py"

import sounddevice as sd
import numpy as np

duracao = 0.2
limite = 0.35

print(sd.query_devices())

while True:
    audio = sd.rec(
        int(duracao * 44100),
        samplerate=44100,
        channels=1
    )

    sd.wait()

    rms = np.sqrt(np.mean(audio ** 2))
    print("RMS:", rms)

    if rms >= limite:
        print("GRITO DETECTADO")
