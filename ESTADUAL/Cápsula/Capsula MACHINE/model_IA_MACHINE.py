import os

# Desativa avisos e logs informativos do TensorFlow/oneDNN no terminal
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'

import numpy as np
import tensorflow as tf
import sounddevice as sd
import time
import serial


# =====================================================
# COMUNICAÇÃO COM O ARDUINO
# =====================================================

# COLOQUE AQUI A PORTA DO SEU ARDUINO
PORTA_ARDUINO = "COM7"

arduino = serial.Serial(
    PORTA_ARDUINO,
    9600,
    timeout=1
)

# Aguarda o Arduino reiniciar
time.sleep(2)

print("Arduino conectado!")


# =====================================================
# 1. CARREGA O MODELO TFLITE
# =====================================================

interpreter = tf.lite.Interpreter(
    model_path="soundclassifier_with_metadata.tflite"
)

interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()


# =====================================================
# CARREGA AS CLASSES
# =====================================================

with open("labels.txt", "r", encoding="utf-8") as f:
    labels = [line.strip() for line in f.readlines()]


# =====================================================
# CONFIGURAÇÕES DO ÁUDIO
# =====================================================

SAMPLE_RATE = 44100
DURATION = 1  # Janela de escuta de 1 segundo


# =====================================================
# ESTADO DO LED
# =====================================================

led_aceso = False


def mostrar_status_led(estado):

    if estado:
        print("\n [  LED: 🟡 LIGADO (PERMANECE ACESO)  ]\n")

    else:
        print("\n [  LED: ⚫ DESLIGADO  ]\n")


# =====================================================
# INFORMAÇÕES INICIAIS
# =====================================================

print("=== SISTEMA DE RECONHECIMENTO DE VOZ ESPECÍFICO ===")

print(f"Classes cadastradas no modelo: {labels}")

print("Filtro ativo: Palavra 'SUSTO' ativará o LED.")

mostrar_status_led(led_aceso)


# =====================================================
# PROCESSAMENTO DO ÁUDIO
# =====================================================

def processar_audio():

    # Captura 1 segundo do microfone
    audio = sd.rec(
        int(DURATION * SAMPLE_RATE),
        samplerate=SAMPLE_RATE,
        channels=1,
        dtype='float32'
    )

    sd.wait()


    # Formata os dados para a entrada do modelo TFLite
    input_shape = input_details[0]['shape']


    try:

        audio_input = np.reshape(
            audio,
            input_shape
        ).astype(np.float32)


    except ValueError:

        audio_input = np.array(
            audio.flatten()[:np.prod(input_shape)],
            dtype=np.float32
        ).reshape(input_shape)


    # Envia áudio para a IA
    interpreter.set_tensor(
        input_details[0]['index'],
        audio_input
    )

    interpreter.invoke()


    # Obtém resultado
    output_data = interpreter.get_tensor(
        output_details[0]['index']
    )

    index = np.argmax(output_data[0])

    return labels[index], output_data[0][index]


# =====================================================
# LOOP PRINCIPAL
# =====================================================

try:

    print("Escutando continuamente...\n")


    while True:

        classe, confianca = processar_audio()


        # Exibe no terminal apenas quando houver
        # detecção relevante acima de 60%
        if confianca > 0.60:

            print(
                f"Ouvido: [{classe}] | "
                f"Confiança: {confianca * 100:.1f}%"
            )


        # =================================================
        # DETECÇÃO DA PALAVRA SUSTO
        # =================================================

        if (
            "susto" in classe.lower()
            and confianca > 0.60
        ):

            # Só envia uma vez
            if not led_aceso:

                led_aceso = True


                print(
                    f"\n🟢 SUSTO DETECTADO! "
                    f"Confiança: {confianca * 100:.1f}%"
                )


                # =========================================
                # ENVIA O COMANDO PARA O ARDUINO
                # =========================================

                arduino.write(b"SUSTO\n")

                arduino.flush()


                print(
                    "📡 Comando 'SUSTO' enviado "
                    "para o Arduino!"
                )


                mostrar_status_led(led_aceso)


        # O LED permanece aceso.
        # Não existe comando para desligá-lo aqui.


        time.sleep(0.05)


# =====================================================
# ENCERRAMENTO
# =====================================================

except KeyboardInterrupt:

    print("\nEncerrando o programa...")


finally:

    if arduino.is_open:

        arduino.close()

    print("Comunicação com Arduino encerrada.")

