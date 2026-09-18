
import json
import queue
import sounddevice as sd
import serial

from vosk import Model, KaldiRecognizer


# ============================================================
# CONFIGURAÇÕES
# ============================================================

MODEL_PATH = r"C:\Users\Instrutor\Downloads\vosk-model-small-pt-0.3"

SAMPLE_RATE = 16000

PORTA_SERIAL = "COM7"

BAUD_RATE = 9600


# ============================================================
# CONEXÃO COM O ARDUINO
# ============================================================

arduino = serial.Serial(
    PORTA_SERIAL,
    BAUD_RATE,
    timeout=1
)


# ============================================================
# FILA QUE RECEBE O ÁUDIO DO MICROFONE
# ============================================================

audio_queue = queue.Queue()


# ============================================================
# CAPTURA DO MICROFONE
# ============================================================

def callback(indata, frames, time, status):

    if status:
        print(f"\nStatus: {status}")

    audio_queue.put(bytes(indata))


# ============================================================
# ENVIA COMANDO PARA O ARDUINO
# ============================================================

def enviar_comando(comando):

    arduino.write(
        (comando + "\n").encode("utf-8")
    )

    print(
        f"\n🤖 Comando enviado para o robô: {comando}"
    )


# ============================================================
# RECONHECIMENTO DE VOZ
# ============================================================

def main():

    print("========================================")
    print("       RECONHECIMENTO DE VOZ")
    print("========================================")
    print()

    print("Carregando modelo...")

    # Carrega o modelo Vosk
    model = Model(MODEL_PATH)

    # Cria o reconhecedor
    recognizer = KaldiRecognizer(
        model,
        SAMPLE_RATE
    )

    print("Modelo carregado com sucesso!")
    print()

    print("🔌 Arduino conectado na", PORTA_SERIAL)
    print("🎤 Fale alguma coisa...")
    print("Pressione Ctrl+C para sair.")
    print()


    # ========================================================
    # ABRE O MICROFONE
    # ========================================================

    with sd.RawInputStream(
        samplerate=SAMPLE_RATE,
        blocksize=4000,
        dtype="int16",
        channels=1,
        callback=callback
    ):

        while True:

            # Pega o próximo bloco de áudio
            data = audio_queue.get()


            # =================================================
            # RESULTADO FINAL
            # =================================================

            if recognizer.AcceptWaveform(data):

                resultado = json.loads(
                    recognizer.Result()
                )

                texto = resultado.get(
                    "text",
                    ""
                )


                if texto:

                    # Limpa a linha anterior
                    print(
                        "\r" + " " * 100,
                        end="\r"
                    )

                    # Mostra o resultado final
                    print(
                        f"✓ Você disse: {texto}"
                    )


                    # =========================================
                    # VERIFICA A PALAVRA "SUSTO"
                    # =========================================

                    texto = texto.lower()

                    palavras_susto = {
                        "ai",
                        "aí",
                        "susto"
                    }

                    palavras = texto.split()


                    # Se alguma palavra reconhecida
                    # estiver na lista, dispara o robô

                    if any(
                        palavra in palavras_susto
                        for palavra in palavras
                    ):

                        print(
                            "⚡ AI DETECTADO!"
                        )

                        enviar_comando("SUSTO")


            # =================================================
            # RESULTADO PARCIAL
            # =================================================

            else:

                parcial = json.loads(
                    recognizer.PartialResult()
                )

                texto_parcial = parcial.get(
                    "partial",
                    ""
                )


                if texto_parcial:

                    # Mostra o que o Vosk está entendendo
                    # enquanto você fala

                    print(
                        f"\r🎤 Ouvindo: {texto_parcial}",
                        end="",
                        flush=True
                    )


# ============================================================
# EXECUÇÃO
# ============================================================

if __name__ == "__main__":

    try:

        main()

    except KeyboardInterrupt:

        print("\n\nPrograma encerrado.")

    except Exception as erro:

        print("\nERRO:")
        print(erro)

    finally:

        if arduino.is_open:

            arduino.close()

