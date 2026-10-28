
import sounddevice as sd
import numpy as np

DISPOSITIVO_MIC = 1
AMOSTRAGEM = 44100
DURACAO_BLOCO = 0.2

def analisar_audio(audio):
    audio = audio.flatten().astype(np.float32)

    # 1. Volume geral
    rms = float(np.sqrt(np.mean(audio ** 2)))

    # 2. Energia relativa em frequências altas
    espectro = np.abs(np.fft.rfft(audio))
    frequencias = np.fft.rfftfreq(len(audio), 1 / AMOSTRAGEM)

    energia_total = np.sum(espectro ** 2) + 1e-12
    energia_aguda = np.sum(espectro[frequencias >= 1500] ** 2)
    proporcao_aguda = float(energia_aguda / energia_total)

    # 3. Variação do sinal entre amostras
    variacao = float(np.mean(np.abs(np.diff(audio))))

    return rms, proporcao_aguda, variacao


print("TESTE DO DETECTOR DE GRITOS")
print("Não envia comandos ao Arduino.")
print("Faça silêncio, fale normalmente e depois grite.")
print("Pressione Ctrl+C para terminar.\n")

try:
    with sd.InputStream(
        device=DISPOSITIVO_MIC,
        samplerate=AMOSTRAGEM,
        channels=1,
        blocksize=int(AMOSTRAGEM * DURACAO_BLOCO),
        dtype="float32"
    ) as mic:
        while True:
            audio, overflowed = mic.read(
                int(AMOSTRAGEM * DURACAO_BLOCO)
            )

            rms, agudos, variacao = analisar_audio(audio)

            print(
                f"RMS={rms:.5f} | "
                f"Agudos={agudos:.3f} | "
                f"Variacao={variacao:.5f}"
            )

except KeyboardInterrupt:
    print("\nTeste terminado.")
except Exception as erro:
    print(f"\nErro ao capturar audio: {erro}")
