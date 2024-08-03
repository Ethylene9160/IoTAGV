import random

class Agent:
    used_colors = set()

    def __init__(self, id, position, velocity=None, target_position=None):
        self.id = id
        self.position = position
        self.velocity = velocity
        self.target_position = target_position
        self.color = self.generate_unique_color()

    @staticmethod
    def generate_unique_color():
        while True:
            color = [random.randint(0, 255) for _ in range(3)]
            color_tuple = tuple(color)
            if color_tuple not in Agent.used_colors:
                Agent.used_colors.add(color_tuple)
                return color

class AgentsPool:
    def __init__(self):
        self.agents = []
    
    def append(self, agent: Agent):
        self.agents.append(agent)
    
    def remove(self, agent: Agent):
        self.agents.remove(agent)
    
    def to_list(self):
        return [{
            'id': item.id,
            'position': item.position,
            'velocity': item.velocity,
            'target_position': item.target_position,
            'color': item.color
        } for item in self.agents]
    
    def get_agent_by_id(self, id):
        for agent in self.agents:
            if agent.id == id:
                return agent
        return None

class Anchor:
    def __init__(self, id, position):
        self.id = id
        self.position = position

class AnchorsPool:
    def __init__(self):
        self.anchors = []
    
    def append(self, anchor: Anchor):
        self.anchors.append(anchor)
    
    def remove(self, anchor: Anchor):
        self.anchors.remove(anchor)
    
    def to_list(self):
        return [{
            'id': item.id,
            'position': item.position
        } for item in self.anchors]

