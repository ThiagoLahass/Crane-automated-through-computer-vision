import cv2

# ============= TEST BEFORE USE ==========
# initial min and max HSV filter to RED
H_MIN_RED = 0
H_MAX_RED = 183
S_MIN_RED = 14
S_MAX_RED = 141
V_MIN_RED = 245
V_MAX_RED = 255

# initial min and max HSV filter to GREEN
H_MIN_GREEN = 94
H_MAX_GREEN = 105
S_MIN_GREEN = 94
S_MAX_GREEN = 187
V_MIN_GREEN = 75
V_MAX_GREEN = 117

# initial min and max HSV filter to BLUE
H_MIN_BLUE = 92
H_MAX_BLUE = 143
S_MIN_BLUE = 131
S_MAX_BLUE = 255
V_MIN_BLUE = 178
V_MAX_BLUE = 255

# initial min and max HSV filter to YELLOW
H_MIN_YELLOW = 17
H_MAX_YELLOW = 44
S_MIN_YELLOW = 21
S_MAX_YELLOW = 112
V_MIN_YELLOW = 209
V_MAX_YELLOW = 246
# ============= END TEST BEFORE USE ==========

RED     = 0
GREEN   = 1
BLUE    = 2
YELLOW  = 3

COLORS_NAME = ["RED", "GREEN", "BLUE", "YELLOW"]

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
MIN_OBJECT_AREA = 40 * 40                                   # ADJUST WHEN DOING CONTAINER DETECTION ON CRANE
MAX_OBJECT_AREA = int(FRAME_HEIGHT * FRAME_WIDTH / 1.5)

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
        print("BACKEND: How many different types of containers do you want to search for?")
        num_types = input("Enter the number of types: ").strip()

        if num_types.isdigit() and int(num_types) > 0:
            num_types = int(num_types)
            break
        else:
            print("Invalid Input. Please enter an integer greater than 0.")

    for i in range(num_types):
        print(f"Type of container {i + 1}:")
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
        print("BACKEND: What color container do you want?")
        print("Options:")
        print("0 - RED")
        print("1 - GREEN")
        print("2 - BLUE")
        print("3 - YELLOW")

        color_container = input("Enter the number corresponding to the color: ").strip()

        if color_container.isdigit():
            color_container = int(color_container)
            if color_container in range(4):
                return color_container
            else:
                print("Invalid option. Please choose a number between 0 and 3.")
        else:
            print("Invalid Input. Please enter a number.")

def select_quantity():
    """
    Prompt the user to select the quantity of containers for a specific type.

    Returns:
        int: Quantity of containers.
    """
    while True:
        quantity = input("BACKEND: How many containers of this type do you want?\n").strip()

        if quantity.isdigit():
            quantity = int(quantity)
            if quantity > 0:
                return quantity
            else:
                print("Invalid number. Please enter a number greater than 0.")
        else:
            print("Invalid Input. Please enter a number.")