import cv2
import numpy as np
import serial
import time

#==================== ESP SERIAL COMUNICATION VARIABLES AND FUNCTIONS ====================
PORT_NAME = 'COM7'

# read one char (default)
def read_ser(port, num_char=1):
    string = port.read(num_char)
    try:
        return string.decode('utf-8')
    except UnicodeDecodeError as e:
        print(f"Erro de decodificação: {e}")
        return ""

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

# ============= TEST BEFORE USE ==========
# initial min and max HSV filter to RED
H_MIN_RED = 0
H_MAX_RED = 16
S_MIN_RED = 11
S_MAX_RED = 85
V_MIN_RED = 251
V_MAX_RED = 255

# initial min and max HSV filter to GREEN
H_MIN_GREEN = 42
H_MAX_GREEN = 103
S_MIN_GREEN = 42
S_MAX_GREEN = 100
V_MIN_GREEN = 121
V_MAX_GREEN = 240

# initial min and max HSV filter to BLUE
H_MIN_BLUE = 60
H_MAX_BLUE = 106
S_MIN_BLUE = 25
S_MAX_BLUE = 221
V_MIN_BLUE = 249
V_MAX_BLUE = 255

# initial min and max HSV filter to YELLOW
H_MIN_YELLOW = 29
H_MAX_YELLOW = 43
S_MIN_YELLOW = 0
S_MAX_YELLOW = 20
V_MIN_YELLOW = 241
V_MAX_YELLOW = 255
# ============= END TEST BEFORE USE ==========

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
    #cv2.createTrackbar("Color", trackbar_window_name, COLOR, len(my_colors_thresholds) - 1, onTrackbar_COLOR)

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
    img = cv2.dilate(img, dilateElement)

    return img

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
    imgTemp = img_mask.copy()

    contours, hierarchy = cv2.findContours(imgTemp, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

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


def select_color_of_containers():
    while True:
        print("BACKEND: Qual a cor do container que você deseja?")
        print("Opções:")
        print("0 - RED")
        print("1 - GREEN")
        print("2 - BLUE")
        print("3 - YELLOW")

        color_container = input("Digite o número correspondente à cor: ").strip()
        
        if color_container.isdigit():
            color_container = int(color_container)
            if color_container in range(4):
                return color_container
            else:
                print("Opção inválida. Por favor, escolha um número entre 0 e 3.")
        else:
            print("Entrada inválida. Por favor, insira um número.")

def select_number_of_containers():
    while True:
        number_containers = input("BACKEND: Quantos containers você deseja?\n").strip()
        
        if number_containers.isdigit():
            number_containers = int(number_containers)
            if number_containers > 0:
                return number_containers
            else:
                print("Número inválido. Por favor, insira um número maior que 0.")
        else:
            print("Entrada inválida. Por favor, insira um número.")

#==================== END OBJECT IDENTIFIER AND TRACKING VARIABLES AND FUNCTIONS ====================

def main():
    #==================== ESP SERIAL COMUNICATION SETUP ====================
    MAX_BUFF_LEN = 1024
    SETUP 		 = False
    port 		 = None
    prev         = time.time()

    while(not SETUP):
        try:
            #Serial port(windows-->COM), baud rate, timeout msg
            port = serial.Serial(PORT_NAME, 9600, timeout=1)
        except:
            if(time.time() - prev > 2): # Don't spam with msg
                print('No serial detected, please plug your uController')
                prev = time.time()

        if(port is not None): # We're connected
            SETUP = True
    #=================== END ESP SERIAL COMUNICATION SETUP ==================

    #==================== OBJECT POSITION/CAMERA VARIABLES SETUP ====================
    X_CENTER = FRAME_WIDTH / 2
    Y_CENTER = FRAME_HEIGHT / 2
    x = 0
    y = 0
    delta_x = 0
    delta_y = 0

    # some boolean variables for different functionality within this program
    track_objects = True
    use_morph_ops = True
    
    # Read from webcam
    capture = cv2.VideoCapture(0)

    # Abrindo logo as telas para nao termos delay depois
    ret, img_original = capture.read()
    img_HSV = cv2.cvtColor(img_original, cv2.COLOR_BGR2HSV)
    img_mask = np.zeros((4, FRAME_HEIGHT, FRAME_WIDTH), dtype=np.uint8)
    for i, thresholds in enumerate(my_colors_thresholds):
        lower = np.array(thresholds[:3])
        upper = np.array(thresholds[3:])
        img_mask[i] = cv2.inRange(img_HSV, lower, upper)
        if use_morph_ops:
            img_mask[i] = clean_noise_morph_ops(img_mask[i])
        cv2.imshow(COLORS_NAME[i], img_mask[i])
        
    cv2.imshow("Image Original", img_original)
    cv2.imshow("Image HSV", img_HSV)

    create_trackbars()
    #=================== END OBJECT POSITION VARIABLES SETUP =================

    first_loop_flag = False

    # FIRST WHILE LOOP - IT'S REPRESENT EVERY TIME AS THE COMAND TO SEARCH A CONTAINER IS SEND
    while( not first_loop_flag ):
  
        number_containers = select_number_of_containers()
        COLOR = select_color_of_containers()
        
        for i in range(number_containers):
            print(f"BACKEND: Buscando container {i+1} da cor {COLORS_NAME[COLOR]}")

            #==================== ESP SERIAL COMUNICATION SENDING BEGIN COMAND ====================
            # SERIAL COMUNICATION - READ
            string = read_ser(port, MAX_BUFF_LEN)
            print(string)

            time.sleep(2)

            # SERIAL COMUNICATION - WRITE
            cmd = 'begin'
            write_ser(port, cmd)

            time.sleep(0.1)

            # SERIAL COMUNICATION - READ
            string = read_ser(port, MAX_BUFF_LEN)
            print(string)
            #==================== END ESP SERIAL COMUNICATION SENDING BEGIN COMAND ====================

            # Reset the track control variable
            track_objects = True

            # SECOND WHILE LOOP - IT'S REPRESENT THE SEARCH FOR THE SELECTED CONTAINER
            while True:
                # capture the actual frame from the webcam
                ret, img_original = capture.read()
                if not ret:
                    break
                
                # convert the BGR spacecolor to HSV to be easy to apply filters
                img_HSV = cv2.cvtColor(img_original, cv2.COLOR_BGR2HSV)

                # CREATE A MASK FOR EVERY TRACKED COLOR
                for i, thresholds in enumerate(my_colors_thresholds):
                    lower = np.array(thresholds[:3])
                    upper = np.array(thresholds[3:])
                    img_mask[i] = cv2.inRange(img_HSV, lower, upper)
                    if use_morph_ops:
                        img_mask[i] = clean_noise_morph_ops(img_mask[i])
                    cv2.imshow(COLORS_NAME[i], img_mask[i])

                    # if track_objects and i == COLOR:
                    #     my_container_point = get_center_of_nearest_container(img_mask[COLOR], img_original, COLOR)
                    #     if my_container_point != (0, 0):
                    #         drawCrosshairs(my_container_point[0], my_container_point[1], COLOR, img_original)

                # #Definição dos valores mínimos e máximos para a máscara HSV
                # lower = np.array([H_MIN, S_MIN, V_MIN])
                # upper = np.array([H_MAX, S_MAX, V_MAX])

                # lower = np.array([H_MIN_BLUE, S_MIN_BLUE, V_MIN_BLUE])
                # upper = np.array([H_MAX_BLUE, S_MAX_BLUE, V_MAX_BLUE])

                # # Filtragem da imagem HSV entre os valores e armazenamento na matriz 'img_mask'
                # img_mask = cv2.inRange(img_HSV, lower, upper)

                # # Operações morfológicas na imagem thresholded para eliminar ruído
                # # e enfatizar o(s) objeto(s) filtrado(s)
                # if use_morph_ops:
                #     img_mask = clean_noise_morph_ops(img_mask)

                # Passagem do frame thresholded para nossa função de rastreamento de objetos
                # esta função retornará as coordenadas x e y do objeto filtrado
                if track_objects:
                    # (x,y) = track_filtered_object(GREEN, img_mask, img_original)
                    (x,y) = get_center_of_nearest_container(img_mask[COLOR], img_original, COLOR)
                    if x != 0 and y != 0 :
                        drawCrosshairs(x, y, COLOR, img_original)
                    delta_x = int (X_CENTER - x)
                    delta_y = int (Y_CENTER - y)

                    # Preenchimento com zeros à esquerda para garantir 3 dígitos
                    delta_x_str = str(delta_x).zfill(4)
                    delta_y_str = str(delta_y).zfill(4)

                    # SERIAL COMUNICATION - WRITE
                    cmd = f'{delta_x_str} {delta_y_str}'
                    write_ser(port, cmd)

                    time.sleep(0.1)

                    # SERIAL COMUNICATION - READ
                    string = read_ser(port, MAX_BUFF_LEN)
                    if(len(string)):
                        print(f"{string}")
                        if (string == "ESP: container centralizado"):
                            print("BACKEND: Parando de rastrear container...")
                            track_objects = False

                else:
                    string = read_ser(port, MAX_BUFF_LEN)
                    if(len(string)):
                        print(f"{string}")
                        if(string == "ESP: end"):
                            print("BACKEND: Buscando proximo container, caso for o ultimo volta para o primeiro loop ('begin')")
                            break
                    

                # Exibição das imagens
                cv2.imshow("Image Original", img_original)
                cv2.imshow("Image HSV", img_HSV)
                # cv2.imshow("Image Threshold", img_mask)

                key = cv2.waitKey(1)
                if key == 27:  # Press ESC to exit
                    first_loop_flag = True
                    break

    # capture.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
