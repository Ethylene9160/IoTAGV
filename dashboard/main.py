import json
import struct
import random
import asyncio

from sanic import Sanic, response
from sanic import HTTPResponse, Request, Websocket

from clients import WebSocketClientsPool
from serials import SerialManager
from core import Agent, AgentsPool, Anchor, AnchorsPool, CommandUtils, CommandFSM


APP_NAME = 'Enormous233'
APP_HOST = 'localhost'
APP_PORT = 4560
SERIAL_POLL_INTERVAL = 0.2

app = Sanic(APP_NAME)
clients = WebSocketClientsPool()
serials = SerialManager()
agents = AgentsPool()
anchors = AnchorsPool()

anchors.append(Anchor(0x00, [0.0, 0.0]))
anchors.append(Anchor(0x01, [2.0, 0.0]))

# TODO: mock agents
# agents.append(Agent(0x80, [1.0, 2.0], [-0.3, 1.0], [3.0, 4.0]))
# agents.append(Agent(0x81, [2.0, 2.0], [-0.3, -0.4], [-0.5, 3.0]))


"""
    跨域请求处理中间件
"""
@app.middleware('request')
async def cors_middle_req(request: Request):
    if request.method.lower() == 'options':
        allow_headers = [
            'Authorization',
            'content-type'
        ]
        headers = {
            'Access-Control-Allow-Methods': ', '.join(['GET', 'POST', 'OPTIONS']),
            'Access-Control-Max-Age': '86400',
            'Access-Control-Allow-Headers': ', '.join(allow_headers),
        }
        return HTTPResponse('', headers=headers)

@app.middleware('response')
def cors_middle_res(request: Request, response: HTTPResponse):
    allow_origin = '*'
    response.headers.update(
        {
            'Access-Control-Allow-Origin': allow_origin,
        }
    )


"""
    路由
"""
@app.route('/') # Index
async def index(request: Request):
    return response.html(f'<p>{APP_NAME}</p>')

@app.websocket('/notify') # 客户端注册 WebSocket
async def notify(request: Request, ws: Websocket):
    clients.append(ws)
    try:
        async for msg in ws:
            print(f'Received: {msg}')
    except Exception as e:
        print(f'WebSocket error: {e}')

@app.get('/serials') # 扫描串口
async def get_serials_list(request: Request):
    return response.json(serials.scan_serials())

@app.post('/serial') # 操作串口, opt in ('open', 'close')
async def open_serial(request: Request):
    data = request.json
    opt = data.get('opt', 'open')
    port = data.get('port', None)
    
    if opt == 'close':
        status = serials.close()
    elif opt == 'open' and port is not None:
        status = serials.open(**{
            'port': port,
            'baudrate': data.get('baudrate', 115200),
            'bytesize': data.get('bytesize', 8),
            'parity': data.get('parity', 'N'),
            'stopbits': data.get('stopbits', 1)
        })
    else:
        status = False
    
    await clients.broadcast(json.dumps({'new_status': serials.is_ready()})) # 操作串口后广播串口连接状态
    
    return response.json({'status': status})

@app.get('/serial')
async def check_serial(request: Request):
    return response.json({'status': serials.is_ready()})

@app.post('/command')
async def command(request: Request):
    data = request.json
    msg_type = data.get('type', None)
    id = data.get('id', None)
    opt1 = data.get('opt1', None)
    opt2 = data.get('opt2', None)
    
    if msg_type is None or id is None:
        return response.json({'status': False})
    
    if msg_type == 1 and opt1 is not None and opt2 is not None: # 设置目标位置
        cmd = CommandUtils.pack_set_target_position_command(id, opt1, opt2)
    elif msg_type == 2 and opt1 is not None: # 设置速度系数
        cmd = CommandUtils.pack_set_velocity_ratio_command(id, opt1)
    elif msg_type == 3: # 暂停
        cmd = CommandUtils.pack_pause_command(id)
    elif msg_type == 4: # 继续
        cmd = CommandUtils.pack_resume_command(id)
    else:
        return response.json({'status': False})
    
    serials.write(cmd)
    print(f'command sent: type = {msg_type}, id = {id}, opt1 = {opt1}, opt2 = {opt2}, raw = {cmd}.')
    
    return response.json({'status': True})


"""
    串口与接收报文处理任务
"""
async def broadcast_canvas_update():
    await clients.broadcast(json.dumps({
        'type': 'plot',
        'agents': agents.to_list(),
        'anchors': anchors.to_list()
    }))

async def execute_datagram_update(content):
    """
    1 坐标更新:
        0x5A ... 0x5A (同步, 三个及以上)
        0xFF (开始)
        0x0A 0x00 (长度, uint16)
        0x00 (type, 0x00 为坐标)
        0x00 (id, uint8)
        0x 0x 0x 0x (x, float)
        0x 0x 0x 0x (y, float)
        0x7F (结束)

    2 速度更新:
        0x5A ... 0x5A (同步, 三个及以上)
        0xFF (开始)
        0x0A 0x00 (长度, uint16)
        0x01 (type, 0x01 为速度)
        0x00 (id, uint8)
        0x 0x 0x 0x (vx, float)
        0x 0x 0x 0x (vy, float)
        0x7F (结束)

    3 目标坐标更新:
        0x5A ... 0x5A (同步, 三个及以上)
        0xFF (开始)
        0x0A 0x00 (长度, uint16)
        0x02 (type, 0x02 为目标坐标)
        0x00 (id, uint8)
        0x 0x 0x 0x (tx, float)
        0x 0x 0x 0x (ty, float)
        0x7F (结束)
    """
    print(f'datagram executed: {content}')
    
    msg_type = content[0]
    if msg_type in [1, 2, 3] and len(content) == 10:
        id, opt1, opt2 = struct.unpack('<Bff', content[1:])
        agent = agents.get_agent_by_id(id)
        if msg_type == 1:
            if agent is None: # 未见过的 agent, 创建
                agents.append(Agent(id, [opt1, opt2]))
            else:
                agent.position = [opt1, opt2]
        elif msg_type == 2:
            if agent is not None: # 必须先有 position, 故仅收到未见过的 agent 的 velocity 不予创建
                agent.velocity = [opt1, opt2]
        else: # msg_type == 3:
            if agent is not None: # 必须先有 position, 故仅收到未见过的 agent 的 target_position 不予创建
                agent.target_position = [opt1, opt2]
        
        await broadcast_canvas_update() # 1, 2, 3 命令皆需广播画布更新

fsm = CommandFSM(execute_datagram_update)

async def serial_listener():
    while True:
        if serials.is_ready(): # 串口处于开启状态
            if serials.is_received(): # 输入缓存中有数据
                await fsm.parse(serials.readall())
        else: # 串口关闭, 清空状态
            fsm.reset()
        await asyncio.sleep(SERIAL_POLL_INTERVAL) # 间隔取 buffer, 每次处理取的部分

async def serial_talker_mock():
    while True:
        if serials.is_ready():
            serials.write(CommandUtils.pack_position_datagram(233, random.uniform(-1, 3), random.uniform(0, 4)))
            serials.write(CommandUtils.pack_velocity_datagram(233, random.uniform(-1, 3), random.uniform(0, 4)))
            serials.write(CommandUtils.pack_target_position_datagram(233, random.uniform(-1, 3), random.uniform(0, 4)))
        await asyncio.sleep(0.2)


"""
    启动后台任务和服务端
"""
@app.listener('before_server_start')
async def setup_background_task(app, loop):
    loop.create_task(serial_listener())
    loop.create_task(serial_talker_mock())

if __name__ == '__main__':
    app.run(host = APP_HOST, port = APP_PORT)

