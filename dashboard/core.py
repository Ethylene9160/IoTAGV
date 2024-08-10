import random
import struct

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

class CommandUtils:
    @staticmethod
    def pack_position_datagram(id, x, y): # mock
        return b'\x5A\x5A\x5A\x5A\x5A\xFF' + b'\x0A\x00' + struct.pack('<BBff', 1, id, x, y) + b'\x7F'
    
    @staticmethod
    def pack_velocity_datagram(id, vx, vy): # mock
        return b'\x5A\x5A\x5A\x5A\x5A\xFF' + b'\x0A\x00' + struct.pack('<BBff', 2, id, vx, vy) + b'\x7F'
    
    @staticmethod
    def pack_target_position_datagram(id, tx, ty): # mock
        return b'\x5A\x5A\x5A\x5A\x5A\xFF' + b'\x0A\x00' + struct.pack('<BBff', 3, id, tx, ty) + b'\x7F'

    @staticmethod
    def pack_set_target_position_command(id, tx, ty): # to agent
        return b'\x5A' + struct.pack('<BBff', 1, id, tx, ty) + b'\x7F'
    
    @staticmethod
    def pack_set_velocity_ratio_command(id, ratio): # to agent
        return b'\x5A' + struct.pack('<BBff', 2, id, ratio, 0) + b'\x7F'
    
    @staticmethod
    def pack_pause_command(id): # to agent
        return b'\x5A' + struct.pack('<BBff', 3, id, 0, 0) + b'\x7F'
    
    @staticmethod
    def pack_resume_command(id): # to agent
        return b'\x5A' + struct.pack('<BBff', 4, id, 0, 0) + b'\x7F'
    
    @staticmethod
    def pack_turn_left_a_bit_command(id): # to agent
        return b'\x5A' + struct.pack('<BBff', 5, id, 0, 0) + b'\x7F'
    
    @staticmethod
    def pack_turn_right_a_bit_command(id): # to agent
        return b'\x5A' + struct.pack('<BBff', 6, id, 0, 0) + b'\x7F'

class CommandFSM:
    def __init__(self, func):
        self.func = func
        
        self.reset()
    
    def reset(self):
        self.state = 'sync' # 'sync', 'head', 'len', 'content', 'tail'
        self.sync_bits = 0 # -> 3
        self.head_bits = 0 # -> 1
        self.len_bits = 0 # -> 2
        self.content_bits = 0 # -> self.content_length
        # self.end_bits = 0 # -> 1
        
        self.content_length = 0
        self.content = b''
    
    async def parse(self, data: bytes):
        for i in range(len(data)):
            peek: int = data[i]
            if self.state == 'sync':
                if peek == 0x5A: # 连续的 0x5A, 累计
                    self.sync_bits += 1
                    if self.sync_bits >= 3: # 累计收到三个 0x5A, 跳转 HEAD
                        self.state = 'head'
                else: # 被打断, 重置
                    self.reset()
            elif self.state == 'head':
                if peek == 0x5A: # 继续的 0x5A
                    self.sync_bits += 1
                elif peek == 0xFF: # 紧跟的 0xFF, 跳转 LEN
                    self.head_bits += 1
                    self.state = 'len'
                else: # 被打断, 重置
                    self.reset()
            elif self.state == 'len':
                self.len_bits += 1
                if self.len_bits == 1: # 低字节
                    self.content_length = peek
                else: # 高字节
                    self.content_length = peek * 256 + self.content_length
                    if self.content_length > 0: # 有内容, 跳转 CONTENT
                        self.state = 'content'
                    else: # 无内容, 跳转 TAIL
                        self.state = 'tail'
            elif self.state == 'content':
                self.content_bits += 1
                self.content = self.content + int.to_bytes(peek)
                if self.content_bits >= self.content_length: # 内容足够, 跳转 TAIL
                    self.state = 'tail'
            elif self.state == 'tail':
                if peek == 0x7F: # 结束符正确, 执行
                    await self.func(self.content)
                self.reset() # 重置
            else:
                self.reset()


# fsm = CommandFSM(lambda content: print(content))
# fsm.parse(b'\x5A\x5A\x5A\x5A\x5A\x5A\xFF' + b'\x0A\x00' + b'\x00\x81\xcd\xcc\xcc=\xcd\xccL>' + b'\x7f')

