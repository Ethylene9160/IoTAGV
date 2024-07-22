import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.patches as patches
import matplotlib.lines as lines
import re

MODE = 'display'  # 'debug', 'display'
FACTOR = 10000
XMIN = -1.5
XMAX = 3.5
YMIN = -0.5
YMAX = 5.5
ARROW_WIDTH = 0.0005
RADIUS = 0.3
THRESHOLD = 0.3
ALPHA = 0.7

TARGET_0 = [1.0, 4.5]
TARGET_1 = [3.0, 1.5]
TARGET_2 = [-0.3, 1.5]

ANCHOR_0_POS = [0.0, 0.0]
ANCHOR_1_POS = [2.0, 0.0]

def extract_numbers(string):
    numbers = re.findall(r'-?\d+', string)
    return list(map(int, numbers[:4]))

class SerialPlotter:
    def __init__(self, port, baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.raw_dot0_x = 0.0
        self.raw_dot0_y = 0.0
        self.raw_dot1_x = 0.0
        self.raw_dot1_y = 0.0
        self.raw_dot2_x = 0.0
        self.raw_dot2_y = 0.0

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
        self.arrow = patches.FancyArrow(0, 0, 0, 0, width=ARROW_WIDTH, color='red')
        self.ax.add_patch(self.arrow)

        self.circle0 = patches.Circle((0, 0), RADIUS, color='red', fill=False, linestyle='dashed')
        self.circle1 = patches.Circle((0, 0), RADIUS, color='green', fill=False, linestyle='dashed')
        self.circle2 = patches.Circle((0, 0), RADIUS, color='blue', fill=False, linestyle='dashed')
        self.ax.add_patch(self.circle0)
        self.ax.add_patch(self.circle1)
        self.ax.add_patch(self.circle2)

        self.target0, = self.ax.plot(TARGET_0[0], TARGET_0[1], 'rx')
        self.target1, = self.ax.plot(TARGET_1[0], TARGET_1[1], 'gx')
        self.target2, = self.ax.plot(TARGET_2[0], TARGET_2[1], 'bx')

        self.threshold0 = patches.Circle(TARGET_0, THRESHOLD, color='grey', fill=False, linestyle='dotted')
        self.threshold1 = patches.Circle(TARGET_1, THRESHOLD, color='grey', fill=False, linestyle='dotted')
        self.threshold2 = patches.Circle(TARGET_2, THRESHOLD, color='grey', fill=False, linestyle='dotted')
        self.ax.add_patch(self.threshold0)
        self.ax.add_patch(self.threshold1)
        self.ax.add_patch(self.threshold2)

        self.anchor0, = self.ax.plot(ANCHOR_0_POS[0], ANCHOR_0_POS[1], 'o', color='grey')
        self.anchor1, = self.ax.plot(ANCHOR_1_POS[0], ANCHOR_1_POS[1], 'o', color='grey')

        self.text0 = self.ax.text(0, 0, '0', fontsize=9, color='black')
        self.text1 = self.ax.text(0, 0, '1', fontsize=9, color='black')
        self.text2 = self.ax.text(0, 0, '2', fontsize=9, color='black')

        self.line0 = lines.Line2D([], [], linestyle='dotted', color='grey')
        self.line1 = lines.Line2D([], [], linestyle='dotted', color='grey')
        self.line2 = lines.Line2D([], [], linestyle='dotted', color='grey')
        self.ax.add_line(self.line0)
        self.ax.add_line(self.line1)
        self.ax.add_line(self.line2)

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
        self.text0.set_position((0, 0))
        self.text1.set_position((0, 0))
        self.text2.set_position((0, 0))
        self.line0.set_data([], [])
        self.line1.set_data([], [])
        self.line2.set_data([], [])
        
        return (self.dot0, self.dot1, self.dot2, self.arrow, self.circle0, 
                self.circle1, self.circle2, self.target0, self.target1, self.target2, 
                self.threshold0, self.threshold1, self.threshold2, self.anchor0, self.anchor1, 
                self.text0, self.text1, self.text2, self.line0, self.line1, self.line2)

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
                        self.raw_dot0_x = x
                        self.raw_dot0_y = y
                    elif id == 129:
                        self.raw_dot1_x = x
                        self.raw_dot1_y = y
                    elif id == 130:
                        self.raw_dot2_x = x
                        self.raw_dot2_y = y
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

        if MODE == 'display':
            self.dot0_x = ALPHA * self.raw_dot0_x + (1 - ALPHA) * self.dot0_x
            self.dot0_y = ALPHA * self.raw_dot0_y + (1 - ALPHA) * self.dot0_y
            self.dot1_x = ALPHA * self.raw_dot1_x + (1 - ALPHA) * self.dot1_x
            self.dot1_y = ALPHA * self.raw_dot1_y + (1 - ALPHA) * self.dot1_y
            self.dot2_x = ALPHA * self.raw_dot2_x + (1 - ALPHA) * self.dot2_x
            self.dot2_y = ALPHA * self.raw_dot2_y + (1 - ALPHA) * self.dot2_y
        else:
            self.dot0_x = self.raw_dot0_x
            self.dot0_y = self.raw_dot0_y
            self.dot1_x = self.raw_dot1_x
            self.dot1_y = self.raw_dot1_y
            self.dot2_x = self.raw_dot2_x
            self.dot2_y = self.raw_dot2_y

        self.dot0.set_data(self.dot0_x, self.dot0_y)
        self.dot1.set_data(self.dot1_x, self.dot1_y)
        self.dot2.set_data(self.dot2_x, self.dot2_y)

        self.arrow.remove()
        self.arrow = patches.FancyArrow(self.dot0_x, self.dot0_y, self.vx, self.vy, width=ARROW_WIDTH, color='red')
        self.ax.add_patch(self.arrow)

        self.circle0.center = (self.dot0_x, self.dot0_y)
        self.circle1.center = (self.dot1_x, self.dot1_y)
        self.circle2.center = (self.dot2_x, self.dot2_y)

        TEXT_OFFSET = 0.06
        self.text0.set_position((self.dot0_x + TEXT_OFFSET, self.dot0_y + TEXT_OFFSET))
        self.text1.set_position((self.dot1_x + TEXT_OFFSET, self.dot1_y + TEXT_OFFSET))
        self.text2.set_position((self.dot2_x + TEXT_OFFSET, self.dot2_y + TEXT_OFFSET))

        self.line0.set_data([self.dot0_x, TARGET_0[0]], [self.dot0_y, TARGET_0[1]])
        self.line1.set_data([self.dot1_x, TARGET_1[0]], [self.dot1_y, TARGET_1[1]])
        self.line2.set_data([self.dot2_x, TARGET_2[0]], [self.dot2_y, TARGET_2[1]])
        
        return (self.dot0, self.dot1, self.dot2, self.arrow, self.circle0, 
                self.circle1, self.circle2, self.target0, self.target1, self.target2, 
                self.threshold0, self.threshold1, self.threshold2, self.anchor0, self.anchor1, 
                self.text0, self.text1, self.text2, self.line0, self.line1, self.line2)

    def start(self):
        plt.show()

if __name__ == '__main__':
    port = 'COM5'
    baudrate = 115200
    plotter = SerialPlotter(port, baudrate)
    plotter.start()
