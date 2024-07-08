import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation

class SerialPlotter:
    def __init__(self, port, baudrate=9600):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.x_data = []
        self.y_data = []
        self.filtered_x = 0.0
        self.filtered_y = 0.0

        self.fig, self.ax = plt.subplots()
        self.red_dot, = self.ax.plot([], [], 'ro')
        self.blue_dot, = self.ax.plot([], [], 'bo')

        self.ani = animation.FuncAnimation(self.fig, self.update_plot, interval=1, init_func=self.init_plot, blit=True)

    def init_plot(self):
        self.ax.set_xlim(0, 20000)
        self.ax.set_ylim(0, 20000)
        self.red_dot.set_data([], [])
        self.blue_dot.set_data([], [])
        return self.red_dot, self.blue_dot

    def read_serial(self):
        if self.ser.in_waiting > 0:
            try:
                line = self.ser.readline().decode().strip()
                x, y, _, _ = map(float, line.split(','))
                self.x_data = [x]
                self.y_data = [y]
                self.apply_filter(x, y)
                print(x, ' ', y)
            except Exception as e:
                print(f"Error reading line: {e}")

    def apply_filter(self, x, y):
        alpha = 0.2
        self.filtered_x = alpha * x + (1 - alpha) * self.filtered_x
        self.filtered_y = alpha * y + (1 - alpha) * self.filtered_y

    def update_plot(self, frame):
        self.read_serial()
        self.red_dot.set_data(self.x_data, self.y_data)
        self.blue_dot.set_data([self.filtered_x], [self.filtered_y])
        return self.red_dot, self.blue_dot

    def start(self):
        plt.show()

if __name__ == '__main__':
    port = 'COM4'
    baudrate = 115200
    plotter = SerialPlotter(port, baudrate)
    plotter.start()

