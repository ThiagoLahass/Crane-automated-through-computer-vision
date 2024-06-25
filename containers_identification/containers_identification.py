import cv2
import numpy as np
import time
import my_serial
from utils import FRAME_WIDTH, FRAME_HEIGHT, my_colors_thresholds, COLORS_NAME, clean_noise_morph_ops, select_colors_and_quantities, get_center_of_nearest_container, draw_crosshair

def main():
    """
    Main function to control the object detection and tracking process.
    """
    PORT_NAME = 'COM7'
    port = my_serial.setup_serial_comunication(PORT_NAME)

    #==================== OBJECT POSITION/CAMERA VARIABLES SETUP ====================
    X_CENTER = FRAME_WIDTH  / 2
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

    #=================== END OBJECT POSITION VARIABLES SETUP =================

    # WAITING THE CRANE BE AT THE CENTRAL POSITION (DEFAULT)
    while(1):
        string = my_serial.read_ser(port, my_serial.MAX_BUFF_LEN)
        string = string.strip()
        if(len(string)):
            print(f"'{string}'")
            if (string == "ESP: Cpr"):       # Received when 'Central position reached'
                print("BACKEND: Crane at the central position, now we can start!\n")
                break
        time.sleep(0.1)

    first_loop_flag = False

    # FIRST WHILE LOOP - IT'S REPRESENT EVERY TIME AS THE COMAND TO SEARCH A CONTAINER IS SEND
    while( not first_loop_flag ):
        
        # input of the container(s) information
        colors, quantities = select_colors_and_quantities()
        print("BACKEND: Selected colors: ", colors)
        print("BACKEND: Selected quantities: ", quantities)

        # for each set of containers of a color
        for index, color in enumerate(colors):
            # for each container
            for i in range(quantities[index]):
                print(f"BACKEND: Searching for container {i+1}/{quantities[index]} of {COLORS_NAME[color]} color")

                #==================== ESP SERIAL COMUNICATION SENDING BEGIN COMAND ====================
                string = my_serial.read_ser(port, my_serial.MAX_BUFF_LEN).strip()
                print(string)

                # delay to ensure that ESP has time to read and respond to the Serial
                time.sleep(1)

                print("BACKEND: Sending 'begin' command to ESP...")
                cmd = 'begin'
                my_serial.write_ser(port, cmd)

                time.sleep(1)

                string = my_serial.read_ser(port, my_serial.MAX_BUFF_LEN).strip()
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
                        #FIXME: ADICIONAR A DIFERENÇA ENTRE O CENTRO DO ELETROÍMÃ PARA A CÂMERA
                        delta_x = int (X_CENTER - x + 80)
                        delta_y = int (Y_CENTER - y)

                        # Padding with leading zeros to guarantee 4 digits (with sign)
                        delta_x_str = str(delta_x).zfill(4)
                        delta_y_str = str(delta_y).zfill(4)

                        cmd = f'{delta_x_str} {delta_y_str}\n'
                        my_serial.write_ser(port, cmd)

                        # time.sleep(0.1)

                        # SERIAL COMUNICATION - READ
                        string = my_serial.read_ser(port, my_serial.MAX_BUFF_LEN).strip()
                        if(len(string)):
                            print(f"{string}")
                            # If the difference between the current position of the object in the camera
                            # and the center is within the allowed limit, ESP sends a message to our BACKEND informing this
                            if (string == "ESP: Cc"):       # Received when the crane its above the select container ("Container centrilized")
                                print("BACKEND: Centralized container, stopping tracking...")
                                track_objects = False
                            elif (string == "ESP: EoC1"):   # Received when End of course activated, going to the center position ("End of Course 1")
                                print("BACKEND: One of the translation limits was reached, the container was inaccessible, so we had to return to the central position and start searching for the next container...")
                                track_objects = False

                    else:
                        string = my_serial.read_ser(port, my_serial.MAX_BUFF_LEN).strip()
                        if(len(string)):
                            print(f"{string}")
                            # When the bridge has completed the search cycle for a container,
                            # the ESP also sends a message to the BACKEND informing this fact
                            if(string == "ESP: end"):       # Received when the complete cycle of take a container is ended
                                print("BACKEND: Searching for the next container, if it is the last one, return to the first loop ('begin')")
                                break
                            elif (string == "ESP: EoC2"):   # Received when End of course was activated, and the center position was reached ("End of Course 2")
                                print("BACKEND: Crane recentralized, now we can start searching for the next container...\n")
                                time.sleep(1)
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
