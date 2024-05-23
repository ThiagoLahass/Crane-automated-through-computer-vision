import cv2
import numpy as np

# initial min and max HSV filter values.
# these will be changed using trackbars
H_MIN = 0
H_MAX = 255
S_MIN = 0
S_MAX = 255
V_MIN = 0
V_MAX = 255

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

def main():

    use_morph_ops = False
    
    # Read from webcam
    capture = cv2.VideoCapture(0)

    create_trackbars()

    while True:
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

        lower = np.array([H_MIN, S_MIN, V_MIN])
        upper = np.array([H_MAX, S_MAX, V_MAX])

        # Filtragem da imagem HSV entre os valores e armazenamento na matriz 'img_mask'
        img_mask = cv2.inRange(img_HSV, lower, upper)

        # Operações morfológicas na imagem thresholded para eliminar ruído
        # e enfatizar o(s) objeto(s) filtrado(s)
        if use_morph_ops:
            img_mask = clean_noise_morph_ops(img_mask)

        # Exibição das imagens
        cv2.imshow("Image Original", img_original)
        cv2.imshow("Image HSV", img_HSV)
        cv2.imshow("Image Threshold", img_mask)

        key = cv2.waitKey(1)
        if key == 27:  # Press ESC to exit
            break

    capture.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()