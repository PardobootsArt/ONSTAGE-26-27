import cv2
import math
import numpy as np
from pyapriltags import Detector

# 1. INICIALIZA O DETECTOR DE APRILTAGS DO PC
# (Especificamos a família padrão: tag36h11)
at_detector = Detector(families='tag36h11',
                       nthreads=1,
                       quad_decimate=1.0,
                       quad_sigma=0.0,
                       refine_edges=1,
                       decode_sharpening=0.25,
                       debug=0)

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

        if tag.tag_id == 1:
            tag_detectada = True
            
            # Calcula largura e altura aproximadas com base nos cantos
            tagWidth = np.linalg.norm(corners[0] - corners[1])
            tagHeight = np.linalg.norm(corners[1] - corners[2])
            
            # Calcula a diagonal (Pitágoras)
            tagLength = math.sqrt((tagWidth ** 2) + (tagHeight ** 2))
            
            # Desenha a linha lilás do centro da tag até o centro da tela
            cv2.line(frame, (cx, cy), (centro_tela_x, centro_tela_y), (255, 130, 171), 2)
            
            # Desenha o círculo azul no centro da tela
            cv2.circle(frame, (centro_tela_x, centro_tela_y), 6, (255, 144, 30), -1)
            
            # --- LÓGICA DOS MOTORES NO SERIAL ---
            tolerancia = 25  # Maior porque a resolução da webcam é maior
            print("\n--------------------------------------------------")
            print(f"Tag 0 Detectada -> Centro X: {cx} | Distância Relativa: {tagLength:.1f}")
            
            if cx < (centro_tela_x - tolerancia):
                print("[AÇÃO] Virar para a ESQUERDA 🔄")
                print("[MOTORES] Motor Direito: LIGADO [>>] | Motor Esquerdo: DESLIGADO [..]")
            elif cx > (centro_tela_x + tolerancia):
                print("[AÇÃO] Virar para a DIREITA 🔄")
                print("[MOTORES] Motor Esquerdo: LIGADO [>>] | Motor Direito: DESLIGADO [..]")
            else:
                print("[AÇÃO] Alinhado! Seguir em Frente ⬆️")
                print("[MOTORES] Motor Esquerdo: LIGADO [>>] | Motor Direito: LIGADO [>>]")
            print("--------------------------------------------------")
            

    if not tag_detectada:
        print("[STATUS] Procurando Tag ID 0... | [MOTORES] PARADOS [..] [..]")

    # Mostra a imagem na janela do computador
    cv2.imshow("Rastreamento de Tag", frame)

    # Pressione a tecla 'q' no teclado para fechar o programa
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Limpa a memória ao fechar
cap.release()
cv2.destroyAllWindows()