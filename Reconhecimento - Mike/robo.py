import cv2
import math
import numpy as np
from pyapriltags import Detector
import serial
import serial.tools.list_ports
import time

# 1. INICIALIZA O DETECTOR DE APRILTAGS DO PC
# (Especificamos a família padrão: tag36h11)
at_detector = Detector(families='tag36h11',
                       nthreads=1,
                       quad_decimate=1.0,
                       quad_sigma=0.0,
                       refine_edges=1,
                       decode_sharpening=0.25,
                       debug=0)

# -----------------------------
# Procura automaticamente o Arduino
# -----------------------------
def encontrar_arduino():
    """
    Procura automaticamente uma porta que contenha
    'Arduino' ou 'CH340' (muitos Arduinos genéricos usam CH340).
    """

    portas = serial.tools.list_ports.comports()

    for porta in portas:
        if "Arduino" in porta.description or "CH340" in porta.description:
            return porta.device

    return None


porta_arduino = encontrar_arduino()

if porta_arduino:

    arduino = serial.Serial(porta_arduino, 9600)

    # Sempre esperamos alguns segundos.
    # Quando abrimos a Serial, o Arduino reinicia.
    time.sleep(2)

    print(f"Arduino conectado em {porta_arduino}")

else:

    porta_manual = input("Arduino não encontrado. Digite a porta COM: ")

    arduino = serial.Serial(porta_manual,9600)

    time.sleep(2)

    print(f"Arduino conectado em {porta_manual}")
# 2. INICIALIZA A WEBCAM
# O argumento '0' geralmente é a webcam integrada do notebook
cap = cv2.VideoCapture(1)

print("=== Sistema de Webcam Iniciado! Mostre a Tag ID 0 ===")

while True:
    ret, frame = cap.read()
    if not ret:
        print("Erro ao acessar a webcam.")
        break

    # O detector do PC precisa da imagem em tons de cinza
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    tags = at_detector.detect(gray)
    
    # Centro da tela do notebook (vamos recalcular dinamicamente)
    altura, largura, _ = frame.shape
    centro_tela_x = largura // 2
    centro_tela_y = altura // 2
    
    tag_detectada = False

    for tag in tags:
        # Pega os cantos da tag para desenhar o retângulo
        corners = tag.corners.astype(int)
        
        # Desenha o retângulo ao redor da tag (Equivalente ao draw_rectangle)
        for i in range(4):
            cv2.line(frame, tuple(corners[i]), tuple(corners[(i+1)%4]), (0, 0, 255), 2)
            
        # Centro geométrico da tag (Equivalente ao tag.cx e tag.cy)
        cx, cy = int(tag.center[0]), int(tag.center[1])
        
        # Desenha a cruz verde no centro (Equivalente ao draw_cross)
        cv2.drawMarker(frame, (cx, cy), (0, 255, 0), cv2.MARKER_CROSS, 15, 2)

        if tag.tag_id == 0:
            tag_detectada = True

            tagWidth = np.linalg.norm(corners[0] - corners[1])
            tagHeight = np.linalg.norm(corners[1] - corners[2])
            
            # Calcula a diagonal (Pitágoras)
            tagLength = math.sqrt((tagWidth ** 2) + (tagHeight ** 2))
            
            # Desenha a linha lilás do centro da tag até o centro da tela
            cv2.line(frame, (cx, cy), (centro_tela_x, centro_tela_y), (255, 130, 171), 2)
            
            # Desenha o círculo azul no centro da tela
            cv2.circle(frame, (centro_tela_x, centro_tela_y), 6, (255, 144, 30), -1)

            # ==========================================================
            # CÁLCULO DOS ERROS

            # Erro horizontal.
            erro_x = cx - centro_tela_x

            # Distância desejada.
            DISTANCIA_DESEJADA = 180

            # Erro de distância.
            erro_distancia = DISTANCIA_DESEJADA - tagLength
            
            print("TAG 0")
            mensagem = f"{tag.tag_id},{erro_x},{erro_distancia:.2f}\n"
            arduino.write(mensagem.encode())

            print("-------------------------------------")
            print("Mensagem enviada ao Arduino:")

            print(mensagem.strip())

            print("-------------------------------------")

        elif tag.tag_id == 1:
            tag_detectada = True
            
            print("TAG 1")
            mensagem = "1,0,0\n"
            arduino.write(mensagem.encode())

        elif tag.tag_id == 2:
            tag_detectada = True
            
            print("TAG 2")
            mensagem = "2,0,0\n"
            arduino.write(mensagem.encode())

        elif tag.tag_id == 3:
            tag_detectada = True
            
            print("TAG 3")
            mensagem = "3,0,0\n"
            arduino.write(mensagem.encode())

    if not tag_detectada:
        mensagem = "-1,0,0\n"
        arduino.write(mensagem.encode())
        print("Nenhuma tag detectada.")

    # Mostra a imagem na janela do computador
    cv2.imshow("Rastreamento de Tag", frame)

    # Pressione a tecla 'q' no teclado para fechar o programa
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Limpa a memória ao fechar
cap.release()
cv2.destroyAllWindows()