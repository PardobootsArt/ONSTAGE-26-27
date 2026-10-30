# python "C:\Users\Instrutor\Documents\GitHub\ONSTAGE-26-27\NACIONAL\CAPSULA\capsula.nova.py"


import time
import sounddevice as sd
import numpy as np
import serial

PORTA_SERIAL = "COM7"
DISPOSITIVO_MIC = 1

AMOSTRAGEM = 44100
DURACAO_BLOCO = 0.1

# Limite inicial para ignorar ruídos baixos.
# Vamos ajustar depois de observar os valores em silêncio.

# Detecção inicial mais estável
LIMITE_RMS = 0.20
BLOCOS_NECESSARIOS = 6
print("LIMITE ATIVO:", LIMITE_RMS)
print("BLOCOS NECESSÁRIOS:", BLOCOS_NECESSARIOS)
INTERVALO_GRITOS = 2.0

contagem = 0
ultimo_grito = 0

try:
    with serial.Serial(PORTA_SERIAL, 9600, timeout=1) as arduino:
        print("A ligar ao Arduino...")
        time.sleep(2)
        arduino.reset_input_buffer()

        print("Arduino ligado!")
        print("A ouvir o microfone...")
        print("ARQUIVO: capsula.nova.py")
        print(f"LIMITE RMS ATIVO: {LIMITE_RMS}")
        print(f"BLOCOS NECESSÁRIOS: {BLOCOS_NECESSARIOS}")

        with sd.InputStream(
            device=DISPOSITIVO_MIC,
            samplerate=AMOSTRAGEM,
            channels=1,
            dtype="float32",
            blocksize=int(DURACAO_BLOCO * AMOSTRAGEM)
        ) as mic:

            while True:
                audio, overflowed = mic.read(
                    int(DURACAO_BLOCO * AMOSTRAGEM)
                )

                rms = float(np.sqrt(np.mean(audio ** 2)))
                agora = time.time()

                print(f"RMS: {rms:.5f}")
                print("CONTAGEM ATUAL:", contagem)
                if rms >= LIMITE_RMS:
                 print("ACIMA DO LIMITE CONFIGURADO!")

                if rms >= LIMITE_RMS:
                    contagem += 1
                    print(f"Som acima do limite! Blocos: {contagem}/6")
                else:
                   contagem = 0

                if (
                    contagem >= BLOCOS_NECESSARIOS
                    and agora - ultimo_grito >= INTERVALO_GRITOS
                ):
                    print("GRITO DETECTADO!")

                    arduino.write(b"G")
                    arduino.flush()

                    resposta = arduino.readline().decode(
                        "utf-8", errors="ignore"
                    ).strip()

                    if resposta == "GRITO RECEBIDO":
                        print("Arduino confirmou a receção!")
                    else:
                        print(
                            "Sem confirmação do Arduino. "
                            f"Resposta: {resposta!r}"
                        )

                    ultimo_grito = time.time()
                    contagem = 0

except KeyboardInterrupt:
    print("\nPrograma terminado.")

except Exception as erro:
    print("Erro:", erro)
