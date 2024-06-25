import time
import serial

MAX_BUFF_LEN = 1024

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
    cleaned_string = bytearray()
    for byte in string:
        if byte <= 0x7F:  # Consider only valid ASCII bytes
            cleaned_string.append(byte)
    try:
        decoded_string = cleaned_string.decode('ascii')
        decoded_string_strip = decoded_string.strip()
        return decoded_string_strip
    
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

def setup_serial_comunication(port_name):
    SETUP 		 = False
    port 		 = None
    prev         = time.time()

    while(not SETUP):
        try:
            #Serial port(windows-->COM), baud rate, timeout limit
            port = serial.Serial(port_name, 115200, timeout=0.1)
        except:
            if(time.time() - prev > 2): # Don't spam with msg
                print('No serial detected, please plug your uController')
                prev = time.time()

        if(port is not None): # We're connected
            SETUP = True

    return port
