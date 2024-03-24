import cv2
import numpy as np
import pytesseract

# Configurar o caminho para o executável do Tesseract OCR
pytesseract.pytesseract.tesseract_cmd = r'c:/users/thiag/appdata/local/packages/pythonsoftwarefoundation.python.3.11_qbz5n2kfra8p0/localcache/local-packages/python311/site-packages'

# Carregar a imagem da câmera
imagem = cv2.imread('containers.jpg')

# Converter a imagem para tons de cinza
cinza = cv2.cvtColor(imagem, cv2.COLOR_BGR2GRAY)

# Aplicar um limiar (threshold) para segmentar a imagem e obter a região dos números em branco
_, binaria = cv2.threshold(cinza, 200, 255, cv2.THRESH_BINARY)

# Encontrar contornos na imagem binarizada
contornos, _ = cv2.findContours(binaria, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

# Iterar sobre os contornos para identificar os números
for contorno in contornos:
    x, y, w, h = cv2.boundingRect(contorno)
    
    # Descartar contornos muito pequenos (possíveis ruídos)
    if w * h > 100:
        # Desenhar um retângulo ao redor do número identificado
        cv2.rectangle(imagem, (x, y), (x + w, y + h), (0, 255, 0), 2)
        
        # Aplicar OCR para reconhecer o número
        numero = pytesseract.image_to_string(cinza[y:y+h, x:x+w], config='--psm 6')
        
        # Exibir o número identificado na imagem
        cv2.putText(imagem, f'Numero: {numero}', (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)

# Exibir a imagem com os números identificados
cv2.imshow('Imagem com números identificados', imagem)
cv2.waitKey(0)
cv2.destroyAllWindows()
