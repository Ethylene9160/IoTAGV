import math
import random

T = 0.05

V_SCALAR = 50

class PolarPoint:
    def __init__(self, r, t):
        self.r = r
        self.t = t

class CartPoint:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class CartVelocity:
    def __init__(self, vx, vy, w):
        self.vx = vx
        self.vy = vy
        self.w = w

class VehicleController:
    v_cons = 26.0/V_SCALAR
    v_k = 12.0/V_SCALAR

    def __init__(self, self_id, current_point, target_point):
        self.self_id = self_id
        self.target_point = target_point
        self.self_point = current_point
        self.self_vel = CartVelocity(0.0, 0.0, 0.0)
        self.isTerminal = False
        self.vehicle_position = {}

    def tick(self):
        _v_norm = 20.0/V_SCALAR
        total_weight_x = 0.0
        total_weight_y = 0.0
        bias_x = 0.0
        bias_y = 0.0

        _dx = self.target_point.x - self.self_point.x
        _dy = self.target_point.y - self.self_point.y
        _d = math.sqrt(_dx * _dx + _dy * _dy)

        for vehicle in self.vehicle_position.values():
           bias_x, bias_y = self._update_self_vel(vehicle, bias_x, bias_y, total_weight_x, total_weight_y)

        self.self_vel.vx = VehicleController.v_cons * _dx / _d + bias_x * VehicleController.v_k
        self.self_vel.vy = VehicleController.v_cons * _dy / _d + bias_y * VehicleController.v_k

        _v_square = math.sqrt(self.self_vel.vx * self.self_vel.vx + self.self_vel.vy * self.self_vel.vy)
        # if 0.05 < _v_square < 18.0:
        #     _k = _v_norm / _v_square
        #     self.self_vel.vx *= _k
        #     self.self_vel.vy *= _k

        if abs(self.self_point.x - self.target_point.x) < 0.08 and abs(self.self_point.y - self.target_point.y) < 0.08:
            self.isTerminal = True
        # self.self_vel.vx, self.self_vel.vy = self._add_noise_to_velocity(self.self_vel.vx, self.self_vel.vy)

        self.self_point.x += self.get_self_velocity().vx * T
        self.self_point.y += self.get_self_velocity().vy * T
    def _is_obstacle_near(self, obstacle, vx, vy):
        dx = obstacle.x - self.self_point.x
        dy = obstacle.y - self.self_point.y
        distance = math.sqrt(dx * dx + dy * dy)

        return distance < 0.2 and (dx * vx + dy * vy) > 0

    def _add_noise_to_velocity(self, vx, vy):
        vx += random.gauss(0.0, 1.0) * 0.02
        vy += random.gauss(0.0, 1.0) * 0.02
        return vx, vy

    def _update_self_vel(self, obstacle, bias_x, bias_y, total_weight_x, total_weight_y):
        if math.isnan(obstacle.x) or math.isnan(obstacle.y):
            return bias_x, bias_y

        dx = obstacle.x - self.self_point.x
        dy = obstacle.y - self.self_point.y

        d2 = dx * dx + dy * dy
        distance = math.sqrt(d2)

        if distance < 0.1 or distance > 1.8:
            return bias_x, bias_y

        weight = 1.15 / d2

        bias_x -= weight * dx
        bias_y -= weight * dy

        # total_weight_x += weight
        # total_weight_y += weight
        return bias_x, bias_y

    def push_back(self, id, point):
        if id == self.self_id:
            point.x = (self.self_point.x + point.x) / 2.0
            point.y = (self.self_point.y + point.y) / 2.0
            self.set_self_point(point)
            if abs(self.self_point.x - self.target_point.x) < 0.08 and abs(self.self_point.y - self.target_point.y) < 0.08:
                self.isTerminal = True
        else:
            if id not in self.vehicle_position:
                self.vehicle_position[id] = point
            else:
                existing_point = self.vehicle_position[id]
                existing_point.x = (point.x + existing_point.x) / 2.0
                existing_point.y = (point.y + existing_point.y) / 2.0
                self.vehicle_position[id] = existing_point

    def set_self_point(self, point):
        self.self_point = point

    def get_self_point(self):
        return self.self_point

    def set_self_velocity(self, velocity):
        self.self_vel = velocity

    def is_near_terminal(self):
        return self.isTerminal

    def get_self_velocity(self):
        if self.isTerminal:
            return CartVelocity(0.0, 0.0, 0.0)
        return self.self_vel
