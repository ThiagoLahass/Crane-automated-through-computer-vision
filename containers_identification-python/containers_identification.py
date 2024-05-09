import cv2
import numpy as np
import serial
import time

#==================== ESP SERIAL COMUNICATION VARIABLES AND FUNCTIONS ====================
PORT_NAME = 'COM6'

# read one char (default)
def read_ser(port, num_char = 1):
	string = port.read(num_char)
	return string.decode()

# Write whole strings
def write_ser(port, cmd):
	cmd = cmd + '\n'
	port.write(cmd.encode())
#==================== END ESP SERIAL COMUNICATION VARIABLES AND FUNCTIONS ====================

#==================== OBJECT IDENTIFIER AND TRACKING VARIABLES AND FUNCTIONS ====================
# initial min and max HSV filter values.
# these will be changed using trackbars
H_MIN = 0
H_MAX = 255
S_MIN = 0
S_MAX = 255
V_MIN = 0
V_MAX = 255

# ============= BASED ON TESTS ==========
# initial min and max HSV filter to RED
H_MIN_RED = 145
H_MAX_RED = 182
S_MIN_RED = 69
S_MAX_RED = 151
V_MIN_RED = 202
V_MAX_RED = 255

# initial min and max HSV filter to GREEN
H_MIN_GREEN = 92
H_MAX_GREEN = 105
S_MIN_GREEN = 94
S_MAX_GREEN = 214
V_MIN_GREEN = 98
V_MAX_GREEN = 173

# initial min and max HSV filter to BLUE
H_MIN_BLUE = 90
H_MAX_BLUE = 104
S_MIN_BLUE = 168
S_MAX_BLUE = 255
V_MIN_BLUE = 87
V_MAX_BLUE = 134

# initial min and max HSV filter to YELLOW
H_MIN_YELLOW = 0
H_MAX_YELLOW = 49
S_MIN_YELLOW = 14
S_MAX_YELLOW = 69
V_MIN_YELLOW = 157
V_MAX_YELLOW = 255
# ============= END BASED ON TESTS ==========

RED		= 0
GREEN	= 1
BLUE	= 2
YELLOW	= 3

COLORS_NAME = ["RED", "GRENN", "BLUE", "YELLOW"]

COLOR = 0

my_colors_thresholds = [
    [H_MIN_RED,     S_MIN_RED,      V_MIN_RED,      H_MAX_RED,      S_MAX_RED,      V_MAX_RED   ],      # RED
    [H_MIN_GREEN,   S_MIN_GREEN,    V_MIN_GREEN,    H_MAX_GREEN,    S_MAX_GREEN,    V_MAX_GREEN ],      # GREEN
    [H_MIN_BLUE,    S_MIN_BLUE,     V_MIN_BLUE,     H_MAX_BLUE,     S_MAX_BLUE,     V_MAX_BLUE  ],      # BLUE
    [H_MIN_YELLOW,  S_MIN_YELLOW,   V_MIN_YELLOW,   H_MAX_YELLOW,   S_MAX_YELLOW,   V_MAX_YELLOW]       # YELLOW
]

my_colors_values = [
#    B      G       R
    (0,     0,      255 ),      # RED
    (0,     255,    0   ),      # GREEN
    (255,   0,      0   ),      # BLUE
    (0,     255,    255 )       # YELLOW
]

# default capture width and height
FRAME_WIDTH     = 640
FRAME_HEIGHT    = 480

# max number of objects to be detected in frame
MAX_NUM_OBJECTS = 5

# minimum and maximum object area
MIN_OBJECT_AREA = 20 * 20                                   # AJUSTAR QUANDO TIVER FAZENDO A DETECÇÃO DE CONTAINERS NA PONTE ROLANTE
MAX_OBJECT_AREA = int(FRAME_HEIGHT * FRAME_WIDTH / 1.5)

trackbar_window_name = "Trackbars"

def create_trackbars():
    # create window for trackbars
    cv2.namedWindow(trackbar_window_name, cv2.WINDOW_NORMAL)

    # create trackbars and insert them into window
    cv2.createTrackbar("Hue Min", trackbar_window_name, H_MIN, H_MAX, onTrackbar_H_MIN)
    cv2.createTrackbar("Hue Max", trackbar_window_name, H_MAX, H_MAX, onTrackbar_H_MAX)
    cv2.createTrackbar("Sat Min", trackbar_window_name, S_MIN, S_MAX, onTrackbar_S_MIN)
    cv2.createTrackbar("Sat Max", trackbar_window_name, S_MAX, S_MAX, onTrackbar_S_MAX)
    cv2.createTrackbar("Val Min", trackbar_window_name, V_MIN, V_MAX, onTrackbar_V_MIN)
    cv2.createTrackbar("Val Max", trackbar_window_name, V_MAX, V_MAX, onTrackbar_V_MAX)
    cv2.createTrackbar("Color", trackbar_window_name, COLOR, len(my_colors_thresholds) - 1, onTrackbar_COLOR)

def onTrackbar_H_MIN(value):
    global H_MIN
    H_MIN = value

def onTrackbar_H_MAX(value):
    global H_MAX
    H_MAX = value

def onTrackbar_S_MIN(value):
    global S_MIN
    S_MIN = value

def onTrackbar_S_MAX(value):
    global S_MAX
    S_MAX = value

def onTrackbar_V_MIN(value):
    global V_MIN
    V_MIN = value

def onTrackbar_V_MAX(value):
    global V_MAX
    V_MAX = value

def onTrackbar_COLOR(value):
    global COLOR
    COLOR = value

def clean_noise_morph_ops(img):
    # create structuring element that will be used to "dilate" and "erode" image.
	# the element chosen here is a 3px by 3px rectangle
    erodeElement = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
    # dilate with larger element so make sure object is nicely visible
    dilateElement = cv2.getStructuringElement(cv2.MORPH_RECT, (8, 8))

    img = cv2.erode(img, erodeElement)
    # img = cv2.erode(img, erodeElement)

    # dilate with larger element so make sure object is nicely visible
    img = cv2.dilate(img, dilateElement)
    # img = cv2.dilate(img, dilateElement)

def track_filtered_object(color, img_mask, img_original):
    imgTemp = img_mask.copy()

    # find contours of filtered image using openCV findContours function
    contours, hierarchy = cv2.findContours(imgTemp, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)

    ref_area = 0
    object_found = False

    # use moments method to find our filtered object
    for index, cnt in enumerate(contours):
        area = cv2.contourArea(cnt)

        # if the area is less than 20 px by 20px then it is probably just noise
		# if the area is the same as the 3/2 of the image size, probably just a bad filter
		# we only want the object with the largest area so we safe a reference area each
		# iteration and compare it to the area in the next iteration.

        if area > MIN_OBJECT_AREA and area < MAX_OBJECT_AREA and area > ref_area:
            moment = cv2.moments(cnt)
            x = int(moment["m10"] / moment["m00"])
            y = int(moment["m01"] / moment["m00"])
            object_found = True
            ref_area = area

            cv2.circle(img_original, (x, y), 20, my_colors_values[color], 2)
            cv2.line(img_original, (x, y - 25), (x, y + 25), my_colors_values[color], 2)
            cv2.line(img_original, (x - 25, y), (x + 25, y), my_colors_values[color], 2)

            cv2.putText(img_original, f"{x},{y}", (x, y + 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

    if object_found:
        # let user know you found an object
        cv2.putText(img_original, "Tracking Object", (0, 50), cv2.FONT_HERSHEY_SIMPLEX, 2, (0, 255, 0), 2)
        drawCrosshairs(x, y, color, img_original)
        return (x,y)
        
    else:
        cv2.putText(img_original, "Too much noise! Adjust filter", (0, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        return (0,0)
    
def drawCrosshairs(x, y, color, img):
    # use some of the openCV drawing functions to draw crosshairs on tracked image
    cv2.circle(img, (x, y), 20, my_colors_values[color], 2)

    if y - 25 > 0:
        cv2.line(img, (x, y), (x, y - 25), my_colors_values[color], 2)
    else:
        cv2.line(img, (x, y), (x, 0), my_colors_values[color], 2)

    if y + 25 < FRAME_HEIGHT:
        cv2.line(img, (x, y), (x, y + 25), my_colors_values[color], 2)
    else:
        cv2.line(img, (x, y), (x, FRAME_HEIGHT), my_colors_values[color], 2)

    if x - 25 > 0:
        cv2.line(img, (x, y), (x - 25, y), my_colors_values[color], 2)
    else:
        cv2.line(img, (x, y), (0, y), my_colors_values[color], 2)

    if x + 25 < FRAME_WIDTH:
        cv2.line(img, (x, y), (x + 25, y), my_colors_values[color], 2)
    else:
        cv2.line(img, (x, y), (FRAME_WIDTH, y), my_colors_values[color], 2)

    cv2.putText(img, f"{x},{y}", (x, y + 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)


def get_center_of_nearest_container(img_mask, img_original, color):
    contours, hierarchy = cv2.findContours(img_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    contours_poly = [None] * len(contours)
    bound_rect = [None] * len(contours)

    my_point = (0, 0)

    # filter by area before draw to reject noise
    for i in range(len(contours)):
        area = cv2.contourArea(contours[i])

        if area > MIN_OBJECT_AREA:
            perimeter = cv2.arcLength(contours[i], True)
            epsilon = 0.01 * perimeter
            contours_poly[i] = cv2.approxPolyDP(contours[i], epsilon, True)
            bound_rect[i] = cv2.boundingRect(contours_poly[i])

            cv2.drawContours(img_original, contours_poly, i, my_colors_values[color], 2)

            if (bound_rect[i][1] + bound_rect[i][3] // 2) > my_point[1]:
                my_point = (bound_rect[i][0] + bound_rect[i][2] // 2, bound_rect[i][1] + bound_rect[i][3] // 2)

    return my_point

#==================== END OBJECT IDENTIFIER AND TRACKING VARIABLES AND FUNCTIONS ====================

def main():
    #==================== ESP SERIAL COMUNICATION SETUP ====================
    MAX_BUFF_LEN = 255
    SETUP 		 = False
    port 		 = None
    prev = time.time()
    while(not SETUP):
        try:
            #Serial port(windows-->COM), baud rate, timeout msg
            port = serial.Serial(PORT_NAME, 9600, timeout=1)

        except: # Bad way of writing excepts (always know your errors)
            if(time.time() - prev > 2): # Don't spam with msg
                print('No serial detected, please plug your uController')
                prev = time.time()

        if(port is not None): # We're connected
            SETUP = True
    #=================== END ESP SERIAL COMUNICATION SETUP ==================

    #==================== OBJECT POSITION VARIABLES ====================
    X_CENTER = FRAME_WIDTH / 2
    Y_CENTER = FRAME_HEIGHT / 2
    x = 0
    y = 0
    delta_x = 0
    delta_y = 0
    #=================== END OBJECT POSITION VARIABLES =================

    # some boolean variables for different functionality within this program
    track_objects = True
    use_morph_ops = False

    # ====== PARA TESTES COM IMAGEM ======:
    # imagem de "conteiners" para testes
    # path = "Projeto-Ponte_rolante_automatizada\containers_identification-python\Resources\containers1.jpg"
    
    capture = cv2.VideoCapture(0)
    create_trackbars()

    while True:
        # ====== PARA TESTES COM IMAGEM ======:
        # img_original = cv2.imread(path)
        # img_original = cv2.resize(img_original, (FRAME_WIDTH, FRAME_HEIGHT) )

        ret, img_original = capture.read()
        if not ret:
            break

        img_HSV = cv2.cvtColor(img_original, cv2.COLOR_BGR2HSV)

        # for i, thresholds in enumerate(my_colors_thresholds):
        #     lower = np.array(thresholds[:3])
        #     upper = np.array(thresholds[3:])
        #     img_mask = cv2.inRange(img_HSV, lower, upper)
        #     clean_noise_morph_ops(img_mask)
        #     cv2.imshow(COLORS_NAME[i], img_mask)

        #     if track_objects and i == COLOR:
        #         my_container_point = get_center_of_nearest_container(img_mask, img_original, COLOR)
        #         if my_container_point != (0, 0):
        #             drawCrosshairs(my_container_point[0], my_container_point[1], COLOR, img_original)

        # Definição dos valores mínimos e máximos para a máscara HSV
        # lower = np.array([H_MIN, S_MIN, V_MIN])
        # upper = np.array([H_MAX, S_MAX, V_MAX])

        lower = np.array([H_MIN_BLUE, S_MIN_BLUE, V_MIN_BLUE])
        upper = np.array([H_MAX_BLUE, S_MAX_BLUE, V_MAX_BLUE])

        # Filtragem da imagem HSV entre os valores e armazenamento na matriz 'img_mask'
        img_mask = cv2.inRange(img_HSV, lower, upper)

        # Operações morfológicas na imagem thresholded para eliminar ruído
        # e enfatizar o(s) objeto(s) filtrado(s)
        if use_morph_ops:
            img_mask = clean_noise_morph_ops(img_mask)

        # Passagem do frame thresholded para nossa função de rastreamento de objetos
        # esta função retornará as coordenadas x e y do objeto filtrado
        if track_objects:
            (x,y) = track_filtered_object(GREEN, img_mask, img_original)
            delta_x = int (X_CENTER - x)
            delta_y = int (Y_CENTER - y)

            # Preenchimento com zeros à esquerda para garantir 3 dígitos
            delta_x_str = str(delta_x).zfill(3)
            delta_y_str = str(delta_y).zfill(3)

            # SERIAL COMUNICATION - WRITE
            cmd = f'{delta_x_str} {delta_y_str}'
            write_ser(port, cmd)

            # SERIAL COMUNICATION - READ
            # string = read_ser(port, MAX_BUFF_LEN)
            # if(len(string)):
            #     print(string)

        # Exibição das imagens
        cv2.imshow("Image Original", img_original)
        cv2.imshow("Image HSV", img_HSV)
        cv2.imshow("Image Threshold", img_mask)

        key = cv2.waitKey(1)
        if key == 27:  # Press ESC to exit
            break

    # capture.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
