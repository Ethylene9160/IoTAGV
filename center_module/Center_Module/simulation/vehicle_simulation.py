from IPython.core.display_functions import clear_output

from Center_Module.simulation.vehicle_simulation.vehicle_controller import VehicleController, CartPoint, T
import matplotlib.pyplot as plt
import matplotlib.animation as animation

start_point1 = CartPoint(0.035, 0.1)
target_point1 = CartPoint(0.035, 4.9)
vehicle1 = VehicleController(0, start_point1, target_point1)

start_point2 = CartPoint(-0.035, 4.9)
target_point2 = CartPoint(-0.035, 0.1)
vehicle2 = VehicleController(1, start_point2, target_point2)

start_point3 = CartPoint(-2.4, 2.365)
target_point3 = CartPoint(2.4, 2.365)
vehicle3 = VehicleController(2, start_point3, target_point3)

start_point4 = CartPoint(2.4, 2.435)
target_point4 = CartPoint(-2.4, 2.435)
vehicle4 = VehicleController(3, start_point4, target_point4)

def main():
    # 设置终点
    target_point = CartPoint(0.0, 5.0)
    # 设置起点
    start_point = CartPoint(0.75, 1.0)

    # 创建车辆控制器
    vehicle = VehicleController(129, start_point, target_point)

    # 创建障碍物
    obstacle = CartPoint(0.76, 1.73)

    # 将障碍物放入哈希表
    vehicle.push_back(0, obstacle)

    # 计算车辆当前应该给的速度：
    vehicle.tick()
    v = vehicle.get_self_velocity()
    # print(f"Vehicle velocity: {v.vx:.2f}, {v.vy:.2f}")

# 更新车辆状态
def update_vehicles(vehicle1, vehicle2, vehicle3, vehicle4):
    vehicle1.tick()
    vehicle2.tick()
    vehicle3.tick()
    vehicle4.tick()

    vehicle1.push_back(1, vehicle2.get_self_point())
    vehicle1.push_back(2, vehicle3.get_self_point())
    vehicle1.push_back(3, vehicle4.get_self_point())

    vehicle2.push_back(0, vehicle1.get_self_point())
    vehicle2.push_back(2, vehicle3.get_self_point())
    vehicle2.push_back(3, vehicle4.get_self_point())

    vehicle3.push_back(0, vehicle1.get_self_point())
    vehicle3.push_back(1, vehicle2.get_self_point())
    vehicle3.push_back(3, vehicle4.get_self_point())

    vehicle4.push_back(0, vehicle1.get_self_point())
    vehicle4.push_back(1, vehicle2.get_self_point())
    vehicle4.push_back(2, vehicle3.get_self_point())

    # print(f"Vehicle 1: {vehicle1.get_self_point().x}, {vehicle1.get_self_point().y}")
    # print(f"Vehicle 2: {vehicle2.get_self_point().x}, {vehicle2.get_self_point().y}")
    # print(f"Vehicle 3: {vehicle3.get_self_point().x}, {vehicle3.get_self_point().y}")




arr_cons = 2.5

def plot_vehicle_dynamics(p1, p2, p3, p4, step=50):
    plt.figure(figsize=(8, 6))
    plt.scatter(p1.x, p1.y, c='red')
    plt.scatter(p2.x, p2.y, c='green')
    plt.scatter(p3.x, p3.y, c='blue')
    plt.scatter(p4.x, p4.y, c='yellow')
    plt.scatter(target_point1.x, target_point1.y, color='red', marker='s', s=250)
    plt.scatter(target_point2.x, target_point2.y, color='green', marker='s', s=250)
    plt.scatter(target_point3.x, target_point3.y, color='blue', marker='s', s = 250)
    plt.scatter(target_point4.x, target_point4.y, color='yellow', marker='s', s = 250)

    plt.arrow(p1.x, p1.y, vehicle1.get_self_velocity().vx*arr_cons, vehicle1.get_self_velocity().vy*arr_cons, head_width=0.05, head_length=0.1, fc='r', ec='r')
    plt.arrow(p2.x, p2.y, vehicle2.get_self_velocity().vx*arr_cons, vehicle2.get_self_velocity().vy*arr_cons, head_width=0.05, head_length=0.1, fc='g', ec='g')
    plt.arrow(p3.x, p3.y, vehicle3.get_self_velocity().vx*arr_cons, vehicle3.get_self_velocity().vy*arr_cons, head_width=0.05, head_length=0.1, fc='b', ec='b')
    plt.arrow(p4.x, p4.y, vehicle4.get_self_velocity().vx*arr_cons, vehicle4.get_self_velocity().vy*arr_cons, head_width=0.05, head_length=0.1, fc='y', ec='y')
    plt.xlim(-2.5, 2.5)
    plt.ylim(0.0, 5.0)
    # plt.title("Simulation for 4 vehicles")
    plt.title(f'Simulation of 4 vehicles at t={step * T:.2f} seconds')
    plt.savefig(f"simulation/fig{step}.png")
    plt.grid(False)
    plt.show()
    clear_output(wait=True)


def sim():
    time = 16
    elps = (int)(time/T)
    for i in range(elps+1):
        update_vehicles(vehicle1, vehicle2, vehicle3, vehicle4)
        plot_vehicle_dynamics(vehicle1.get_self_point(), vehicle2.get_self_point(), vehicle3.get_self_point(), vehicle4.get_self_point(), i)


# 绘制单帧车辆动态
def record_vehicle_dynamics(ax, p1, p2, p3, p4, target_points, vehicle_velocities, arr_cons):
    ax.clear()
    ax.scatter(p1.x, p1.y, c='red')
    ax.scatter(p2.x, p2.y, c='green')
    ax.scatter(p3.x, p3.y, c='blue')
    ax.scatter(p4.x, p4.y, c='yellow')
    ax.scatter(target_points[0].x, target_points[0].y, color='red', marker='s', s=250)
    ax.scatter(target_points[1].x, target_points[1].y, color='green', marker='s', s=250)
    ax.scatter(target_points[2].x, target_points[2].y, color='blue', marker='s', s=250)
    ax.scatter(target_points[3].x, target_points[3].y, color='yellow', marker='s', s=250)

    ax.arrow(p1.x, p1.y, vehicle_velocities[0].vx * arr_cons, vehicle_velocities[0].vy * arr_cons,
             head_width=0.05, head_length=0.1, fc='r', ec='r')
    ax.arrow(p2.x, p2.y, vehicle_velocities[1].vx * arr_cons, vehicle_velocities[1].vy * arr_cons,
             head_width=0.05, head_length=0.1, fc='g', ec='g')
    ax.arrow(p3.x, p3.y, vehicle_velocities[2].vx * arr_cons, vehicle_velocities[2].vy * arr_cons,
             head_width=0.05, head_length=0.1, fc='b', ec='b')
    ax.arrow(p4.x, p4.y, vehicle_velocities[3].vx * arr_cons, vehicle_velocities[3].vy * arr_cons,
             head_width=0.05, head_length=0.1, fc='y', ec='y')
    ax.set_xlim(-2.5, 2.5)
    ax.set_ylim(0.0, 5.0)
    ax.set_title("Simulation for 4 vehicles")
    ax.grid(False)


# 保存动画为视频
# 保存动画为视频
def save_animation(filename='vehicle_simulation.mp4'):
    fig, ax = plt.subplots(figsize=(8, 6))

    # 初始化目标点和箭头因子
    start_point1 = CartPoint(0.0, 0.0)
    target_point1 = CartPoint(0.0, 5.0)
    vehicle1 = VehicleController(0, start_point1, target_point1)

    start_point2 = CartPoint(0.0, 5.0)
    target_point2 = CartPoint(0.0, 0.0)
    vehicle2 = VehicleController(1, start_point2, target_point2)

    start_point3 = CartPoint(0.0, -2.5)
    target_point3 = CartPoint(0.0, 2.5)
    vehicle3 = VehicleController(2, start_point3, target_point3)

    start_point4 = CartPoint(0.0, -5.5)
    target_point4 = CartPoint(0.0, 3.5)
    vehicle4 = VehicleController(3, start_point4, target_point4)

    target_points = [target_point1, target_point2, target_point3, target_point4]
    arr_cons = 0.2

    # 定义动画更新函数
    def update(frame):
        update_vehicles(vehicle1, vehicle2, vehicle3, vehicle4)
        record_vehicle_dynamics(ax, vehicle1.get_self_point(), vehicle2.get_self_point(), vehicle3.get_self_point(),
                                vehicle4.get_self_point(), target_points,
                                [vehicle1.get_self_velocity(), vehicle2.get_self_velocity(),
                                 vehicle3.get_self_velocity(), vehicle4.get_self_velocity()], arr_cons)

    # 创建动画
    ani = animation.FuncAnimation(fig, update, frames=30, interval=100, repeat=False)

    # 保存动画为视频文件
    ani.save(filename, writer='ffmpeg', fps=10)


if __name__ == "__main__":
    save_animation()
    # sim()
    # pass
    # simulation()
    # start_point1 = CartPoint(0.0, 0.0)
    # target_point1 = CartPoint(0.0, 5.0)
    # vehicle1 = VehicleController(0, start_point1, target_point1)
    #
    # start_point2 = CartPoint(0.0, 5.0)
    # target_point2 = CartPoint(0.0, 0.0)
    # vehicle2 = VehicleController(1, start_point2, target_point2)
    #
    # start_point3 = CartPoint(-2.5, 2.5)
    # target_point3 = CartPoint(2.5, 2.5)
    # vehicle3 = VehicleController(2, start_point3, target_point3)
    # for i in range(10):
    #     update_vehicles(vehicle1, vehicle2,vehicle3)
