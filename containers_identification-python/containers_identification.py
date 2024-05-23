import cv2
import numpy as np
import serial
import time

#==================== ESP SERIAL COMMUNICATION VARIABLES AND FUNCTIONS ====================
PORT_NAME = 'COM7'

def read_ser(port, num_char=1):
    """
    Reads a specified number of characters from the serial port.

    Args:
        port: The serial port instance.
        num_char (int): Number of characters to read. Default is 1.

    Returns:
        str: The read characters, decoded as UTF-8. Returns an empty string if a decoding error occurs.
    """
    string = port.read(num_char)
    try:
        return string.decode('utf-8')
    except UnicodeDecodeError as e:
        print(f"Decoding error: {e}")
        return ""

def write_ser(port, cmd):
    """
    Writes a command to the serial port.

    Args:
        port: The serial port instance.
        cmd (str): The command to send to the serial port.
    """
    cmd = cmd + '\n'
    port.write(cmd.encode())
#==================== END ESP SERIAL COMMUNICATION VARIABLES AND FUNCTIONS ====================

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
H_MAX_RED = 176
S_MIN_RED = 99
S_MAX_RED = 136
V_MIN_RED = 181
V_MAX_RED = 255

# initial min and max HSV filter to GREEN
H_MIN_GREEN = 42
H_MAX_GREEN = 103
S_MIN_GREEN = 42
S_MAX_GREEN = 100
V_MIN_GREEN = 121
V_MAX_GREEN = 240

# initial min and max HSV filter to BLUE
H_MIN_BLUE = 97
H_MAX_BLUE = 108
S_MIN_BLUE = 144
S_MAX_BLUE = 247
V_MIN_BLUE = 124
V_MAX_BLUE = 255

# initial min and max HSV filter to YELLOW
H_MIN_YELLOW = 23
H_MAX_YELLOW = 48
S_MIN_YELLOW = 47
S_MAX_YELLOW = 121
V_MIN_YELLOW = 174
V_MAX_YELLOW = 232
# ============= END TEST BEFORE USE ==========

RED     = 0
GREEN   = 1
BLUE    = 2
YELLOW  = 3

COLORS_NAME = ["RED", "GREEN", "BLUE", "YELLOW"]

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
MIN_OBJECT_AREA = 50 * 50                                   # ADJUST WHEN DOING CONTAINER DETECTION ON CRANE
MAX_OBJECT_AREA = int(FRAME_HEIGHT * FRAME_WIDTH / 1.5)

trackbar_window_name = "Trackbars"

def create_trackbars():
    """
    Creates a trackbars window to adjust HSV values.
    """
    cv2.namedWindow(trackbar_window_name, cv2.WINDOW_NORMAL)
    cv2.createTrackbar("Hue Min", trackbar_window_name, H_MIN, H_MAX, onTrackbar_H_MIN)
    cv2.createTrackbar("Hue Max", trackbar_window_name, H_MAX, H_MAX, onTrackbar_H_MAX)
    cv2.createTrackbar("Sat Min", trackbar_window_name, S_MIN, S_MAX, onTrackbar_S_MIN)
    cv2.createTrackbar("Sat Max", trackbar_window_name, S_MAX, S_MAX, onTrackbar_S_MAX)
    cv2.createTrackbar("Val Min", trackbar_window_name, V_MIN, V_MAX, onTrackbar_V_MIN)
    cv2.createTrackbar("Val Max", trackbar_window_name, V_MAX, V_MAX, onTrackbar_V_MAX)

def onTrackbar_H_MIN(value):
    """
    Callback to adjust Hue minimum value.

    Args:
        value (int): New minimum Hue value.
    """
    global H_MIN
    H_MIN = value

def onTrackbar_H_MAX(value):
    """
    Callback to adjust Hue maximum value.

    Args:
        value (int): New maximum Hue value.
    """
    global H_MAX
    H_MAX = value

def onTrackbar_S_MIN(value):
    """
    Callback to adjust Saturation minimum value.

    Args:
        value (int): New minimum Saturation value.
    """
    global S_MIN
    S_MIN = value

def onTrackbar_S_MAX(value):
    """
    Callback to adjust Saturation maximum value.

    Args:
        value (int): New maximum Saturation value.
    """
    global S_MAX
    S_MAX = value

def onTrackbar_V_MIN(value):
    """
    Callback to adjust Value minimum value.

    Args:
        value (int): New minimum Value value.
    """
    global V_MIN
    V_MIN = value

def onTrackbar_V_MAX(value):
    """
    Callback to adjust Value maximum value.

    Args:
        value (int): New maximum Value value.
    """
    global V_MAX
    V_MAX = value

def onTrackbar_COLOR(value):
    """
    Callback to adjust the selected container color.

    Args:
        value (int): New color index selected.
    """
    global COLOR
    COLOR = value

def clean_noise_morph_ops(img):
    """
    Applies morphological operations to remove noise from the image.

    Args:
        img (numpy.ndarray): Input image.

    Returns:
        numpy.ndarray: Image after noise removal.
    """
    # create structuring element that will be used to "erode" and "dilate" image to be nicely visible
	# the element chosen here is a 3px by 3px rectangle
    # remove some noise
    erodeElement = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
    # dilate with larger element so make sure object is nicely visible
    dilateElement = cv2.getStructuringElement(cv2.MORPH_RECT, (8, 8))

    img = cv2.erode(img, erodeElement)
    img = cv2.dilate(img, dilateElement)

    return img

def draw_crosshair(x, y, color, img):
    """
    Draws a crosshair on the image around the specified coordinates.

    Args:
        x (int): x coordinate of the crosshair center.
        y (int): y coordinate of the crosshair center.
        color (int): Index of the crosshair color.
        img (numpy.ndarray): Image to draw the crosshair on.
    """
    
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
    """
    Get the center coordinates of the nearest container identified of the especified color.

    Args:
        img_mask (numpy array): Binary mask image of the containers.
        img_original (numpy array): Original image.
        color (int): Index of the color of the container to search.

    Returns:
        tuple: Tuple containing the x and y coordinates of the center of the nearest container.
    """
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


def select_colors_and_quantities():
    """
    Prompt the user to select colors and quantities for containers.

    Returns:
        Tuple[List[int], List[int]]: Two lists, one containing the selected colors and the other containing the corresponding quantities.
    """
    colors = []
    quantities = []

    while True:
        print("BACKEND: Quantos tipos diferentes de containers você deseja procurar?")
        num_types = input("Digite o número de tipos: ").strip()

        if num_types.isdigit() and int(num_types) > 0:
            num_types = int(num_types)
            break
        else:
            print("Entrada inválida. Por favor, insira um número inteiro maior que 0.")

    for i in range(num_types):
        print(f"Tipo de container {i + 1}:")
        color = select_color()
        quantity = select_quantity()
        colors.append(color)
        quantities.append(quantity)

    return colors, quantities

def select_color():
    """
    Prompt the user to select a color for a container type.

    Returns:
        int: Index corresponding to the selected color.
    """
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

def select_quantity():
    """
    Prompt the user to select the quantity of containers for a specific type.

    Returns:
        int: Quantity of containers.
    """
    while True:
        quantity = input("BACKEND: Quantos containers deste tipo você deseja?\n").strip()

        if quantity.isdigit():
            quantity = int(quantity)
            if quantity > 0:
                return quantity
            else:
                print("Número inválido. Por favor, insira um número maior que 0.")
        else:
            print("Entrada inválida. Por favor, insira um número.")

#==================== END OBJECT IDENTIFIER AND TRACKING VARIABLES AND FUNCTIONS ====================

def main():
    """
    Main function to control the object detection and tracking process.
    """
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

    # Opening the screens immediately so we don't have delays later
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
        
        # input of the container(s) information
        colors, quantities = select_colors_and_quantities()
        print("Cores selecionadas:", colors)
        print("Quantidades selecionadas:", quantities)

        # for each set of containers of a color
        for index, color in enumerate(colors):
            # for each container
            for i in range(quantities[index]):
                print(f"BACKEND: Buscando container {i+1}/{quantities[index]} da cor {COLORS_NAME[color]}")

                #==================== ESP SERIAL COMUNICATION SENDING BEGIN COMAND ====================
                # SERIAL COMUNICATION - READ
                string = read_ser(port, MAX_BUFF_LEN)
                print(string)

                # delay to ensure that ESP has time to read and respond to the Serial
                time.sleep(1)

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
                    
                    # convert the BGR spacecolor to HSV, to be easy to apply filters
                    img_HSV = cv2.cvtColor(img_original, cv2.COLOR_BGR2HSV)

                    # Create a mask for every tracked color, and show them
                    for i, thresholds in enumerate(my_colors_thresholds):
                        lower = np.array(thresholds[:3])
                        upper = np.array(thresholds[3:])
                        img_mask[i] = cv2.inRange(img_HSV, lower, upper)
                        if use_morph_ops:
                            img_mask[i] = clean_noise_morph_ops(img_mask[i])
                        cv2.imshow(COLORS_NAME[i], img_mask[i])

                    if track_objects:
                        # get the center of nearest container tracked and draw crosshairs above it 
                        (x,y) = get_center_of_nearest_container(img_mask[color], img_original, color)
                        if x != 0 and y != 0 :
                            draw_crosshair(x, y, color, img_original)
                        
                        # calculates the difference between the container's current position and the center of the image
                        delta_x = int (X_CENTER - x)
                        delta_y = int (Y_CENTER - y)

                        # Padding with leading zeros to guarantee 4 digits (with sign)
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
                            # If the difference between the current position of the object in the camera
                            # and the center is within the allowed limit, ESP sends a message to our BACKEND informing this
                            if (string == "ESP: container centralizado"):
                                print("BACKEND: Parando de rastrear container...")
                                track_objects = False

                    else:
                        string = read_ser(port, MAX_BUFF_LEN)
                        if(len(string)):
                            print(f"{string}")
                            # When the bridge has completed the search cycle for a container,
                            # the ESP also sends a message to the BACKEND informing this fact
                            if(string == "ESP: end"):
                                print("BACKEND: Buscando proximo container, caso for o ultimo volta para o primeiro loop ('begin')")
                                break
                        

                    # Display of original and HSV images
                    cv2.imshow("Image Original", img_original)
                    cv2.imshow("Image HSV", img_HSV)

                    key = cv2.waitKey(1)
                    # Press ESC to exit
                    if key == 27:
                        first_loop_flag = True
                        break

    capture.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
