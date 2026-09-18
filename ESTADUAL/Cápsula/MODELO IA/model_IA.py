
import os
import time
import queue
import numpy as np
import tensorflow as tf
import sounddevice as sd

# =====================================================
# CONFIGURAÇÕES
# =====================================================

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"
os.environ["TF_ENABLE_ONEDNN_OPTS"] = "0"

SAMPLE_RATE = 44100
MODEL_SAMPLES = 44032

# Nova análise a cada 0,1 segundo
ANALYSIS_INTERVAL = 0.10

# =====================================================
# LIMITES DE DETECÇÃO
# =====================================================

# Acima disso, ativa imediatamente
CONFIDENCE_HIGH = 0.90

# Acima disso, pode entrar na confirmação
CONFIDENCE_MEDIUM = 0.75

# Quantas detecções médias são necessárias
CONFIRMATIONS_NEEDED = 2

# Tempo de bloqueio depois da ativação
TEMPO_BLOQUEIO = 1.0


# =====================================================
# CARREGAR MODELO
# =====================================================

interpreter = tf.lite.Interpreter(
    model_path="soundclassifier_with_metadata.tflite"
)

interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()


# =====================================================
# CARREGAR LABELS
# =====================================================

with open("labels.txt", "r", encoding="utf-8") as f:
    labels = [linha.strip() for linha in f.readlines()]


print("==============================================")
print("=== SISTEMA DE RECONHECIMENTO DE VOZ ===")
print("==============================================")
print("Classes:", labels)
print("Modelo carregado!")
print()


# =====================================================
# STATUS DO LED
# =====================================================

led_aceso = False


def mostrar_status_led(estado):

    if estado:
        print("💡 LED ACESO")
    else:
        print("💡 LED APAGADO")


# =====================================================
# FILA DE ÁUDIO
# =====================================================

audio_queue = queue.Queue()


def callback(indata, frames, time_info, status):

    if status:
        print("Status do microfone:", status)

    audio_queue.put(indata.copy())


# =====================================================
# CLASSIFICAÇÃO DO ÁUDIO
# =====================================================

def classificar_audio(audio):

    if len(audio) < MODEL_SAMPLES:
        return None, 0.0

    audio = audio[:MODEL_SAMPLES]

    audio_input = np.expand_dims(
        audio,
        axis=0
    ).astype(np.float32)

    interpreter.set_tensor(
        input_details[0]["index"],
        audio_input
    )

    interpreter.invoke()

    output = interpreter.get_tensor(
        output_details[0]["index"]
    )[0]

    # ================================================
    # PROTEÇÃO CONTRA NaN
    # ================================================

    if not np.all(np.isfinite(output)):
        return None, 0.0

    indice = np.argmax(output)

    confianca = float(output[indice])

    if not np.isfinite(confianca):
        return None, 0.0

    classe = labels[indice]

    return classe, confianca


# =====================================================
# INICIAR MICROFONE
# =====================================================

print("🎤 Microfone iniciado!")
print("Fale SUSTO para ativar a cápsula.")
print()
print("==============================================")
print("Detecção forte: 90% ou mais")
print("Detecção média: 75% ou mais")
print("==============================================")
print()


# =====================================================
# BUFFER DE ÁUDIO
# =====================================================

buffer_audio = np.zeros(
    MODEL_SAMPLES,
    dtype=np.float32
)


# =====================================================
# CONTROLE
# =====================================================

ultimo_tempo = 0

ultimo_susto = -TEMPO_BLOQUEIO

confirmacoes_susto = 0


# =====================================================
# LOOP PRINCIPAL
# =====================================================

try:

    with sd.InputStream(
        samplerate=SAMPLE_RATE,
        channels=1,
        dtype="float32",
        callback=callback
    ):

        while True:

            # =============================================
            # PEGAR ÁUDIO DA FILA
            # =============================================

            while not audio_queue.empty():

                novo_audio = audio_queue.get()

                novo_audio = novo_audio.flatten()

                tamanho = len(novo_audio)

                if tamanho >= MODEL_SAMPLES:

                    buffer_audio = novo_audio[-MODEL_SAMPLES:]

                else:

                    buffer_audio[:-tamanho] = (
                        buffer_audio[tamanho:]
                    )

                    buffer_audio[-tamanho:] = novo_audio


            # =============================================
            # INTERVALO DAS ANÁLISES
            # =============================================

            agora = time.monotonic()

            if agora - ultimo_tempo >= ANALYSIS_INTERVAL:

                ultimo_tempo = agora

                # =========================================
                # CLASSIFICAR
                # =========================================

                classe, confianca = classificar_audio(
                    buffer_audio
                )

                if classe is None:
                    confirmacoes_susto = 0
                    continue


                # =========================================
                # MOSTRAR RESULTADO
                # =========================================

                print(
                    f"[{classe}] "
                    f"{confianca * 100:.1f}%"
                )


                # =========================================
                # VERIFICAR SE É SUSTO
                # =========================================

                eh_susto = (
                    "susto" in classe.lower()
                )


                # =========================================
                # NÃO É SUSTO
                # =========================================

                if not eh_susto:

                    confirmacoes_susto = 0

                    continue


                # =========================================
                # SUSTO MUITO FORTE
                # =========================================

                if confianca >= CONFIDENCE_HIGH:

                    confirmacoes_susto = 0

                    if (
                        agora - ultimo_susto
                        >= TEMPO_BLOQUEIO
                        and
                        not led_aceso
                    ):

                        ultimo_susto = agora

                        print()
                        print("🟢 SUSTO DETECTADO!")
                        print(
                            f"Confiança: "
                            f"{confianca * 100:.1f}%"
                        )

                        led_aceso = True

                        mostrar_status_led(
                            led_aceso
                        )

                        print(
                            "💡 A cápsula permanecerá acesa."
                        )

                        print()

                    continue


                # =========================================
                # SUSTO MÉDIO
                # =========================================

                if confianca >= CONFIDENCE_MEDIUM:

                    confirmacoes_susto += 1

                    print(
                        f"🔎 SUSTO provável: "
                        f"{confirmacoes_susto}/"
                        f"{CONFIRMATIONS_NEEDED}"
                    )

                else:

                    # Abaixo de 75%, não consideramos
                    # uma detecção confiável.
                    confirmacoes_susto = 0

                    continue


                # =========================================
                # CONFIRMAR SUSTO MÉDIO
                # =========================================

                if (
                    confirmacoes_susto
                    >= CONFIRMATIONS_NEEDED
                    and
                    agora - ultimo_susto
                    >= TEMPO_BLOQUEIO
                    and
                    not led_aceso
                ):

                    ultimo_susto = agora

                    print()
                    print("🟢 SUSTO DETECTADO!")
                    print(
                        f"Confiança: "
                        f"{confianca * 100:.1f}%"
                    )

                    led_aceso = True

                    mostrar_status_led(
                        led_aceso
                    )

                    print(
                        "💡 A cápsula permanecerá acesa."
                    )

                    print()

                    confirmacoes_susto = 0


except KeyboardInterrupt:

    print()
    print("Programa encerrado.")


except Exception as e:

    print()
    print("ERRO:")
    print(e)