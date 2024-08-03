class Agent:
    def __init__(self, id, position):
        self.id = id
        self.position = position
        self.velocity = None
        self.target_position = None
        self.color = [0, 0, 255]

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

