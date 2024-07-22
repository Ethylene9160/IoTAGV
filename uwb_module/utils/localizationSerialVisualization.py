import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.patches as patches
import re

FACTOR = 10000
XMIN = -1
XMAX = 3
YMIN = 0
YMAX = 5
ARROW_WIDTH = 0.0005
RADIUS = 0.3

def extract_numbers(string):
    numbers = re.findall(r'\d+', string)
    return list(map(int, numbers[:4]))

class SerialPlotter:
    def __init__(self, port, baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.dot0_x = 0.0
        self.dot0_y = 0.0
        self.dot1_x = 0.0
        self.dot1_y = 0.0
        self.dot2_x = 0.0
        self.dot2_y = 0.0

        self.vx = 0.0
        self.vy = 0.0

        self.fig, self.ax = plt.subplots()
        self.dot0, = self.ax.plot([], [], 'ro')
        self.dot1, = self.ax.plot([], [], 'go')
        self.dot2, = self.ax.plot([], [], 'bo')
        self.arrow = patches.FancyArrow(0, 0, 0, 0, width=ARROW_WIDTH, color='green')
        self.ax.add_patch(self.arrow)

        self.circle0 = patches.Circle((0, 0), RADIUS, color='red', fill=False, linestyle='dashed')
        self.circle1 = patches.Circle((0, 0), RADIUS, color='green', fill=False, linestyle='dashed')
        self.circle2 = patches.Circle((0, 0), RADIUS, color='blue', fill=False, linestyle='dashed')
        self.ax.add_patch(self.circle0)
        self.ax.add_patch(self.circle1)
        self.ax.add_patch(self.circle2)

        self.ani = animation.FuncAnimation(self.fig, self.update_plot, interval=1, init_func=self.init_plot, blit=True)

    def init_plot(self):
        self.ax.set_xlim(XMIN, XMAX)
        self.ax.set_ylim(YMIN, YMAX)
        self.ax.set_aspect('equal')
        self.dot0.set_data([], [])
        self.dot1.set_data([], [])
        self.dot2.set_data([], [])
        self.circle0.center = (0, 0)
        self.circle1.center = (0, 0)
        self.circle2.center = (0, 0)
        
        return self.dot0, self.dot1, self.dot2, self.arrow, self.circle0, self.circle1, self.circle2

    def read_serial(self):
        if self.ser.in_waiting > 0:
            try:
                line = self.ser.readline().decode().strip()
                num_list = extract_numbers(line)
                
                if num_list[0] == 0:
                    id, x, y = num_list[1:4]
                    x /= FACTOR
                    y /= FACTOR
                    
                    if id == 128:
                        self.dot0_x = x
                        self.dot0_y = y
                    elif id == 129:
                        self.dot1_x = x
                        self.dot1_y = y
                    elif id == 130:
                        self.dot2_x = x
                        self.dot2_y = y
                    print(id, ' ', x, ' ', y)
                elif num_list[0] == 1:
                    vx, vy = num_list[1:3]
                    vx /= FACTOR
                    vy /= FACTOR
                    
                    self.vx = (vx - 5) / 4
                    self.vy = (vy - 5) / 4
            
            except Exception as e:
                print(f"Error reading line: {e}")

    def update_plot(self, frame):
        self.read_serial()
        self.dot0.set_data(self.dot0_x, self.dot0_y)
        self.dot1.set_data(self.dot1_x, self.dot1_y)
        self.dot2.set_data(self.dot2_x, self.dot2_y)

        self.arrow.remove()
        self.arrow = patches.FancyArrow(self.dot0_x, self.dot0_y, self.vx, self.vy, width=ARROW_WIDTH, color='green')
        self.ax.add_patch(self.arrow)

        self.circle0.center = (self.dot0_x, self.dot0_y)
        self.circle1.center = (self.dot1_x, self.dot1_y)
        self.circle2.center = (self.dot2_x, self.dot2_y)
        
        return self.dot0, self.dot1, self.dot2, self.arrow, self.circle0, self.circle1, self.circle2

    def start(self):
        plt.show()

if __name__ == '__main__':
    port = 'COM5'
    baudrate = 115200
    plotter = SerialPlotter(port, baudrate)
    plotter.start()
