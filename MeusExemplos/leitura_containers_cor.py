import cv2
import numpy as np

# Carregar a imagem da câmera
imagem = cv2.imread('containers1.jpg')

# Converter a imagem para o espaço de cores HSV
imagem_hsv = cv2.cvtColor(imagem, cv2.COLOR_BGR2HSV)

# Definir o intervalo de cor que representa os containers (verde, por exemplo)
limite_inferior = np.array([40, 50, 50])
limite_superior = np.array([80, 255, 255])

# Criar uma máscara para os pixels na faixa de cor desejada
mascara = cv2.inRange(imagem_hsv, limite_inferior, limite_superior)

# Aplicar a máscara na imagem original para segmentar os containers
containers_segmentados = cv2.bitwise_and(imagem, imagem, mask=mascara)

# Converter a imagem segmentada para tons de cinza
cinza = cv2.cvtColor(containers_segmentados, cv2.COLOR_BGR2GRAY)

cv2.imshow('Cinza', cinza)

# Aplicar um limiar (threshold) para binarizar a imagem
_, binaria = cv2.threshold(cinza, 50, 255, cv2.THRESH_BINARY)

# Encontrar contornos na imagem binarizada
contornos, _ = cv2.findContours(binaria, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

print(contornos)

# Iterar sobre os contornos para identificar os containers e desenhar suas bordas
for contorno in contornos:
    x, y, w, h = cv2.boundingRect(contorno)
    
    # Desenhar um retângulo ao redor do container identificado
    cv2.rectangle(imagem, (x, y), (x + w, y + h), (0, 255, 0), 2)
    
    # Extrair a região do container na imagem original
    roi = imagem[y:y+h, x:x+w]
    
    # Calcular a cor média da região do container
    cor_media = np.mean(roi, axis=(0, 1)).astype(int)
    
    # Exibir a cor média do container
    texto_cor = f'Cor: {cor_media}'
    cv2.putText(imagem, texto_cor, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

# Exibir a imagem com os containers identificados pela cor e suas bordas destacadas
cv2.imshow('Imagem com containers identificados e bordas destacadas', imagem)
cv2.waitKey(0)
cv2.destroyAllWindows()
