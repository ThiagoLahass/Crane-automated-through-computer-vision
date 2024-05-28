import cv2
import numpy as np

#==================== OBJECT IDENTIFIER AND TRACKING VARIABLES AND FUNCTIONS ====================
# initial min and max HSV filter values.
# these will be changed using trackbars
H_MIN = 0
H_MAX = 255
S_MIN = 0
S_MAX = 255
V_MIN = 0
V_MAX = 255

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

def main():

    use_morph_ops = False
    
    # Read from webcam
    capture = cv2.VideoCapture(0)

    create_trackbars()

    while True:
        ret, img_original = capture.read()
        if not ret:
            break
        
        # convert the BGR spacecolor to HSV, to be easy to apply filters
        img_HSV = cv2.cvtColor(img_original, cv2.COLOR_BGR2HSV)
        lower = np.array([H_MIN, S_MIN, V_MIN])
        upper = np.array([H_MAX, S_MAX, V_MAX])

        # Create a mask
        img_mask = cv2.inRange(img_HSV, lower, upper)

        if use_morph_ops:
            img_mask = clean_noise_morph_ops(img_mask)

        # Showing the images
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