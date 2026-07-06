import cv2

# 1. Liga a webcam do computador
camera = cv2.VideoCapture(0)
detector = cv2.QRCodeDetector()

while True:
    # Captura a imagem da câmera
    sucesso, frame = camera.read()
    
    # 2. Detecta o QR Code e lê o texto dele
    dados, pontos, _ = detector.detectAndDecode(frame)
    
    # Se encontrou um QR Code, mostra os dados na tela
    if pontos is not None:
        print(f"QR Code detectado com o texto: {dados}")
        # Aqui você pegaria a posição dos 'pontos' para mover a câmera!

    # Mostra a imagem da câmera na tela
    cv2.imshow("Camera da Robotica", frame)
    
    # Aperte 'q' para fechar o programa
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

camera.release()
cv2.destroyAllWindows()