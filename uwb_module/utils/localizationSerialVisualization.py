import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.patches as patches

import re

def extract_numbers(string):
    numbers = re.findall(r'\d+', string)
    return list(map(int, numbers[:4]))

class SerialPlotter:
    def __init__(self, port, baudrate=9600):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.x_data = []
        self.y_data = []
        self.filtered_x = 0.0
        self.filtered_y = 0.0
        self.x_data2 = []
        self.y_data2 = []
        self.filtered_x2 = 0.0
        self.filtered_y2 = 0.0

        self.vx = 0.0
        self.vy = 0.0

        self.fig, self.ax = plt.subplots()
        self.red_dot, = self.ax.plot([], [], 'ro')
        self.blue_dot, = self.ax.plot([], [], 'bo')
        self.green_dot, = self.ax.plot([], [], 'go')
        self.blue_dot2, = self.ax.plot([], [], 'bo')
        self.arrow = patches.FancyArrow(0, 0, 0, 0, width=50, color='green')
        self.ax.add_patch(self.arrow)

        self.ani = animation.FuncAnimation(self.fig, self.update_plot, interval=1, init_func=self.init_plot, blit=True)

    def init_plot(self):
        self.ax.set_xlim(-10000, 30000)
        self.ax.set_ylim(0, 50000)
        self.red_dot.set_data([], [])
        self.blue_dot.set_data([], [])
        self.green_dot.set_data([], [])
        self.blue_dot2.set_data([], [])
        return self.red_dot, self.blue_dot, self.green_dot, self.blue_dot2, self.arrow

    def read_serial(self):
        if self.ser.in_waiting > 0:
            try:
                line = self.ser.readline().decode().strip()
                num_list = extract_numbers(line)
                
                if num_list[0] == 0:
                    id, x, y = num_list[1:4]
                    
                    if id == 128:
                        self.x_data = [x]
                        self.y_data = [y]
                    elif id == 129:
                        self.x_data2 = [x]
                        self.y_data2 = [y]
                    self.apply_filter(id, x, y)
                    print(id, ' ', x, ' ', y)
                elif num_list[0] == 1:
                    vx, vy = num_list[1:3]
                    self.vx = (vx - 50000) / 4
                    self.vy = (vy - 50000) / 4
                
            except Exception as e:
                print(f"Error reading line: {e}")

    def apply_filter(self, id, x, y):
        alpha = 0.2
        if id == 128:
            self.filtered_x = alpha * x + (1 - alpha) * self.filtered_x
            self.filtered_y = alpha * y + (1 - alpha) * self.filtered_y
        elif id == 129:
            self.filtered_x2 = alpha * x + (1 - alpha) * self.filtered_x2
            self.filtered_y2 = alpha * y + (1 - alpha) * self.filtered_y2

    def update_plot(self, frame):
        self.read_serial()
        self.red_dot.set_data(self.x_data, self.y_data)
        self.blue_dot.set_data([self.filtered_x], [self.filtered_y])
        self.green_dot.set_data(self.x_data2, self.y_data2)
        self.blue_dot2.set_data([self.filtered_x2], [self.filtered_y2])

        # 更新箭头位置和方向
        self.arrow.remove()
        self.arrow = patches.FancyArrow(self.filtered_x, self.filtered_y, self.vx, self.vy, width=50, color='green')
        self.ax.add_patch(self.arrow)
        
        return self.red_dot, self.blue_dot, self.green_dot, self.blue_dot2, self.arrow

    def start(self):
        plt.show()

if __name__ == '__main__':
    port = 'COM5'
    baudrate = 115200
    plotter = SerialPlotter(port, baudrate)
    plotter.start()
