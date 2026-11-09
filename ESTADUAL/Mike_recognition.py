# py -3.13 "c:/Users/Aluno/Documents/GitHub/ONSTAGE-26-27/ESTADUAL/Mike_recognition.py"

from typing import Literal

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


#porta_arduino = "/dev/ttyUSB0"
porta_arduino = "COM8"

try:
    arduino = serial.Serial(porta_arduino, 9600, timeout=1)

    # O Arduino pode reiniciar quando a porta serial é aberta
    time.sleep(2)

    print(f"Arduino conectado em {porta_arduino}")

except serial.SerialException as e:
    print(f"Erro ao conectar com o Arduino: {e}")
    exit()
# 2. INICIALIZA A WEBCAM
# O argumento '0' geralmente é a webcam integrada do notebook
cap = cv2.VideoCapture(0)

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
            DISTANCIA_DESEJADA = 240

            # Erro de distância.
            erro_distancia = DISTANCIA_DESEJADA - tagLength
            
            print("TAG 0")
            mensagem = f"{tag.tag_id},{erro_x},{erro_distancia:.2f},0\n"
            arduino.write(mensagem.encode())

            print("-------------------------------------")
            print("Mensagem enviada ao Arduino:")

            print(mensagem.strip())

            print("-------------------------------------")

        elif tag.tag_id == 2:
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

            razao = tagWidth / tagHeight
            erro_Perspectiva = (1 - razao) * np.sign(erro_x)

            Vx = 0.6 * erro_distancia
            Wx = 0.5 * erro_x
            Vy = 100 * erro_Perspectiva
                
            print("TAG 2")
            mensagem = f"{tag.tag_id},{erro_x},{erro_distancia:.2f},{erro_Perspectiva:.2f}\n"
            arduino.write(mensagem.encode())

            print("-------------------------------------")
            print("Mensagem enviada ao Arduino:")

            print(f"{erro_x:.2f}, {erro_distancia:.2f}, {erro_Perspectiva:.2f}, {Vx:.2f}, {Wx:.2f}, {Vy:.2f}")
 
            print("-------------------------------------")
            
        elif tag.tag_id == 1:
            tag_detectada = True
            
            print("TAG 1")
            mensagem = "1,0,0,0\n"
            arduino.write(mensagem.encode())
    
        elif tag.tag_id == 3:
            tag_detectada = True

            altura, largura, _ = frame.shape
            posicaoX = largura // 3
            posicaoY = altura // 3
            cv2.line(frame, (int(posicaoX), 0), (int(posicaoX),altura), (255, 130, 171), 2)
            cv2.line(frame, (int(posicaoX*2), 0), (int(posicaoX*2),altura), (255, 130, 171), 2)   
            cv2.line(frame, (0, int(posicaoY)), (largura, int(posicaoY)), (255, 130, 171), 2)
            cv2.line(frame, (0, int(posicaoY*2)), (largura, int(posicaoY*2)), (255, 130, 171), 2)

            if 0 < cx < posicaoX:
                print("coluna1")
                if 0 < cy < posicaoY:
                    print("linha1")
                    posicao = 1
                if posicaoY < cy < posicaoY*2:
                    print("linha2")
                    posicao = 2
                if posicaoY*2 < cy < altura:
                    print("linha3")
                    posicao = 3
            if posicaoX < cx < posicaoX*2:
                print("coluna2")
                if 0 < cy < posicaoY:
                    print("linha1")
                    posicao = 4
                if posicaoY < cy < posicaoY*2:
                    print("linha2")
                    posicao = 5
                if posicaoY*2 < cy < altura:
                    print("linha3")
                    posicao = 6

            if posicaoX*2 < cx < largura:
                print("coluna3")
                if 0 < cy < posicaoY:
                    print("linha1")
                    posicao = 7
                if posicaoY < cy < posicaoY*2:
                    print("linha2")
                    posicao = 8
                if posicaoY*2 < cy < altura:
                    print("linha3")
                    posicao = 9
            print(f"Posição: {posicao}")
            mensagem = f"{tag.tag_id},0,0,{posicao}\n"
            arduino.write(mensagem.encode())

            print("TAG 3")
            print("-------------------------------------")
            print("Mensagem enviada ao Arduino:")
            
            print(mensagem.strip())
            
            print("-------------------------------------")

        elif tag.tag_id == 4:
             tag_detectada = True
            
             print("TAG 4")
             mensagem = "4,0,0,0\n"
             arduino.write(mensagem.encode())

        elif tag.tag_id == 5:
                     tag_detectada = True
                    
                     print("TAG 5")
                     mensagem = "5,0,0,0\n"
                     arduino.write(mensagem.encode())

        elif tag.tag_id == 6:
                     tag_detectada = True
                    
                     print("TAG 6")
                     mensagem = "6,0,0,0\n" 
                     arduino.write(mensagem.encode())
            

    if not tag_detectada:
        mensagem = "-1,0,0,0\n"
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
