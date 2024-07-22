import customtkinter as ctk
from CTkSpinbox import CTkSpinbox
from PIL import Image
from CTkMessagebox import CTkMessagebox

class Interface:
    """
    Interface class to create a GUI for a logistics automation system.

    Attributes:
        root (ctk.CTk): The main window of the interface.
        update_callback (function): A callback function to update the container information.
        font_size (int): The font size used in the interface.
        font (tuple): The font type and size used in the interface.
        square_size (int): The size of the color squares.
        images (dict): A dictionary to store images of color squares.
        spin_vars (dict): A dictionary to store spinbox variables.
        flag (int): A flag to indicate the progress state.
        confirm_button (ctk.CTkButton): A button to confirm the container selection.
        progress_label (ctk.CTkLabel): A label to show progress.
        progress_bar (ctk.CTkProgressBar): A progress bar to show progress.
    """
    def __init__(self, root, update_callback):
        """
        Initializes the Interface class.

        Args:
            root (ctk.CTk): The main window of the interface.
            update_callback (function): A callback function to update the container information.
        """
        self.root = root
        self.update_callback = update_callback

        width = 500
        height = 550

        screen_width = root.winfo_screenwidth()
        screen_height = root.winfo_screenheight()
        x = (screen_width // 2) - (width // 2)
        y = (screen_height // 2) - (height // 2)

        # Inicializa a janela principal
        ctk.set_appearance_mode("System")
        ctk.set_default_color_theme("blue")
        self.root.title("Interface de Usuário")
        self.root.geometry(f"{width}x{height}+{x}+{y}")
        self.root.minsize(500, 550)

        self.font_size = 20
        self.font = ("Arial", self.font_size)
        self.square_size = self.font_size

        self.images = {}
        self.spin_vars = {}
        self.flag = 0

        options = [("vermelho", "#ff0000"), ("verde", "#00ff00"), ("azul", "#0000ff")]

        title_label = ctk.CTkLabel(self.root, text="Sistema de Automação Logística", font=("arial bold", 25))
        title_label.pack(pady=10)

        color_label = ctk.CTkLabel(self.root, text="\nEscolha a quantidade para cada\ntipo de contêiner:", font=self.font)
        color_label.pack(pady=10)

        for text, color in options:
            color_int = 0
            if(text == 'verde'):
                color_int = 1
            elif(text == 'azul'):
                color_int = 2
            self.images[color] = self.create_color_square(color, self.square_size)
            frame = ctk.CTkFrame(self.root)
            frame.pack(anchor='center', pady=5)
            lbl = ctk.CTkLabel(frame, text=text, font=self.font, width=150)
            lbl.pack(side='left')
            color_label = ctk.CTkLabel(frame, image=self.images[color], text=None)
            color_label.pack(side='left', padx=10)
            spin_var = ctk.IntVar()
            self.spin_vars[color_int] = spin_var
            spinbox = CTkSpinbox(frame, start_value=0, min_value=0, max_value=2, scroll_value=1, variable=spin_var)
            spinbox.pack(side='right')

        self.confirm_button = ctk.CTkButton(self.root, text="Confirmar", command=self.confirm_selection, font=self.font, corner_radius=25, border_color="grey", border_width=2)
        self.confirm_button.pack(pady=20)

        self.progress_label = ctk.CTkLabel(self.root, text="Movimentação de contêiner em curso...", font=self.font)

        self.progress_bar = ctk.CTkProgressBar(self.root)
        self.progress_bar.set(0)

    def create_color_square(self, color, size):
        """
        Creates a square image of a given color and size.

        Args:
            color (str): The color of the square.
            size (int): The size of the square.

        Returns:
            ctk.CTkImage: The created color square image.
        """
        image = Image.new("RGB", (size, size), color)
        return ctk.CTkImage(light_image=image, dark_image=image, size=(size, size))

    def confirm_selection(self):
        """
        Confirms the selection of container quantities and starts the progress bar.
        """
        container_info = {color: spin_var.get() for color, spin_var in self.spin_vars.items()}
        self.flag = 1
        # for spin_var in self.spin_vars.values():
        #     spin_var.set(0)
        self.progress_label.pack(pady=20)
        self.progress_bar.pack(pady=20)
        self.progress_bar.start()
        self.confirm_button.configure(state="disabled")
        self.update_callback(container_info)

    def stop_progress(self):
        """
        Stops the progress bar and resets the interface.
        """
        self.flag = 0
        self.progress_label.pack_forget()
        self.progress_bar.stop()
        self.progress_bar.pack_forget()
        self.confirm_button.configure(state="normal")

    def show_dialog(self, message, icon):
        """
        Shows a dialog message box.

        Args:
            message (str): The message to display.
            icon (str): The icon to display in the message box.
        """
        # tk.messagebox.showinfo("Aviso", message, icon="warning")
        CTkMessagebox(title="Info", message=message, font=('Arial', 18), icon=icon)