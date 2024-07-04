import customtkinter as ctk
import tkinter as tk
from CTkSpinbox import CTkSpinbox
from PIL import Image
import threading
import time

class Interface:
    def __init__(self, root, update_callback):
        self.root = root
        self.update_callback = update_callback

        # Inicializa a janela principal
        ctk.set_appearance_mode("System")
        ctk.set_default_color_theme("blue")
        self.root.title("Interface de Usuário")
        self.root.geometry("500x550")
        self.root.minsize(500, 550)

        self.font_size = 20
        self.font = ("Arial", self.font_size)
        self.square_size = self.font_size

        self.images = {}
        self.spin_vars = {}
        self.flag = 0

        options = [("vermelho", "#ff0000"), ("verde", "#00ff00"), ("azul", "#0000ff"), ("amarelo", "#ffff00")]

        title_label = ctk.CTkLabel(self.root, text="Sistema de Automação Logística", font=("arial bold", 25))
        title_label.pack(pady=10)

        color_label = ctk.CTkLabel(self.root, text="\nEscolha a quantidade para cada\ntipo de contêiner:", font=self.font)
        color_label.pack(pady=10)

        for text, color in options:
            self.images[color] = self.create_color_square(color, self.square_size)
            frame = ctk.CTkFrame(self.root)
            frame.pack(anchor='center', pady=5)
            lbl = ctk.CTkLabel(frame, text=text, font=self.font, width=150)
            lbl.pack(side='left')
            color_label = ctk.CTkLabel(frame, image=self.images[color], text=None)
            color_label.pack(side='left', padx=10)
            spin_var = ctk.IntVar()
            self.spin_vars[text] = spin_var
            spinbox = CTkSpinbox(frame, start_value=0, min_value=0, max_value=3, scroll_value=1, variable=spin_var)
            spinbox.pack(side='right')

        self.confirm_button = ctk.CTkButton(self.root, text="Confirmar", command=self.confirm_selection, font=self.font, corner_radius=25, border_color="grey", border_width=2)
        self.confirm_button.pack(pady=20)

        self.progress_label = ctk.CTkLabel(self.root, text="Movimentação de contêiner em curso...", font=self.font)

        self.progress_bar = ctk.CTkProgressBar(self.root)
        self.progress_bar.set(0)

    def create_color_square(self, color, size):
        image = Image.new("RGB", (size, size), color)
        return ctk.CTkImage(light_image=image, dark_image=image, size=(size, size))

    def confirm_selection(self):
        container_info = {color: spin_var.get() for color, spin_var in self.spin_vars.items()}
        self.flag = 1
        self.progress_label.pack(pady=20)
        self.progress_bar.pack(pady=20)
        self.progress_bar.start()
        self.confirm_button.configure(state="disabled")
        self.update_callback(container_info)

    def stop_progress(self):
        self.flag = 0
        self.progress_label.pack_forget()
        self.progress_bar.stop()
        self.progress_bar.pack_forget()
        self.confirm_button.configure(state="normal")

    def show_dialog(self, message):
        tk.messagebox.showinfo("Aviso", message, icon="warning")
