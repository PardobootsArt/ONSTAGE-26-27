import os
# Desativa avisos e logs informativos do TensorFlow/oneDNN no terminal
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'

import numpy as np
import tensorflow as tf
import sounddevice as sd
import time

# 1. Carrega o modelo TFLite
interpreter = tf.lite.Interpreter(model_path="soundclassifier_with_metadata.tflite")
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Carrega as classes do arquivo labels.txt
with open("labels.txt", "r") as f:
    labels = [line.strip() for line in f.readlines()]

SAMPLE_RATE = 44100
DURATION = 1  # Janela de escuta de 1 segundo

# Estado do LED (inicia desligado)
led_aceso = False

def mostrar_status_led(estado):
    if estado:
        print("\n [  LED: 🟡 LIGADO (PERMANECE ACESO)  ]\n")
    else:
        print("\n [  LED: ⚫ DESLIGADO  ]\n")

print("=== SISTEMA DE RECONHECIMENTO DE VOZ ESPECÍFICO ===")
print(f"Classes cadastradas no modelo: {labels}")
print("Filtro ativo: Apenas a voz da 'Maria Cecília' ativará o LED.")
mostrar_status_led(led_aceso)

def processar_audio():
    # Captura 1 segundo do microfone
    audio = sd.rec(int(DURATION * SAMPLE_RATE), samplerate=SAMPLE_RATE, channels=1, dtype='float32')
    sd.wait()

    # Formata os dados para a entrada do modelo TFLite
    input_shape = input_details[0]['shape']
    
    try:
        audio_input = np.reshape(audio, input_shape).astype(np.float32)
    except ValueError:
        audio_input = np.array(audio.flatten()[:np.prod(input_shape)], dtype=np.float32).reshape(input_shape)

    interpreter.set_tensor(input_details[0]['index'], audio_input)
    interpreter.invoke()

    output_data = interpreter.get_tensor(output_details[0]['index'])
    index = np.argmax(output_data[0])
    
    return labels[index], output_data[0][index]

# Loop Principal
try:
    print("Escutando continuamente...\n")
    while True:
        classe, confianca = processar_audio()
        
        # Exibe no terminal apenas quando houver detecção relevante acima de 60%
        if confianca > 0.60:
            print(f"Ouvido: [{classe}] | Confiança: {confianca * 100:.1f}%")

        # Verifica se o som reconhecido veio especificamente da label "Maria Cecília"
        # (Ajuste o texto abaixo caso o nome exato na sua label no arquivo labels.txt seja um pouco diferente)
        if "susto" in classe.lower() and confianca > 0.70:

            if not led_aceso:

                led_aceso = True

                print(f"\n🟢 SUSTO DETECTADO! Confiança: {confianca * 100:.1f}%")

                mostrar_status_led(led_aceso)
        
        # Nota: Removemos o bloco 'else' que desligava o LED.
        # Ele permanecerá aceso indefinidamente após a primeira ativação.

        time.sleep(0.05)

except KeyboardInterrupt:
    print("\nEncerrando o programa...")