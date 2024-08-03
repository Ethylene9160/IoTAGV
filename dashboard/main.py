import json
import random
import asyncio

from sanic import Sanic, response
from sanic import HTTPResponse, Request, Websocket

from clients import WebSocketClientsPool
from serials import SerialManager
from core import Agent, AgentsPool, Anchor, AnchorsPool


APP_NAME = 'Enormous233'
APP_HOST = 'localhost'
APP_PORT = 4560
SERIAL_POLL_INTERVAL = 0.1

app = Sanic(APP_NAME)
clients = WebSocketClientsPool()
serials = SerialManager()
agents = AgentsPool()
anchors = AnchorsPool()

anchors.append(Anchor(0x00, [0.0, 0.0]))
anchors.append(Anchor(0x01, [2.0, 0.0]))

# TODO: mock agents
agents.append(Agent(0x80, [1.0, 2.0], [-0.3, 1.0], [3.0, 4.0]))
agents.append(Agent(0x81, [2.0, 2.0], [-0.3, -0.4], [-0.5, 3.0]))


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


"""
    后台监听串口
"""
async def serial_listener():
    while True:
        if serials.is_ready():
            # 串口处于开启状态
            if serials.is_received():
                try:
                    # 输入缓存中有数据, 处理指令
                    command = serials.readline().decode().strip().split(',')
                    canvas_updated = False
                    if len(command) > 0:
                        if int(command[0]) == 0: # 坐标: 0, id, x, y
                            id, x, y = float(command[1]), float(command[2]), float(command[3])
                            agent = agents.get_agent_by_id(id)
                            if agent is None:
                                # 未见过的 agent, 创建
                                agents.append(Agent(id, [x, y]))
                            else:
                                agent.position = [x, y] # TODO: 是引用吗
                            
                            canvas_updated = True
                        elif int(command[0]) == 1: # 速度: 1, id, vx, vy
                            id, vx, vy = float(command[1]), float(command[2]), float(command[3])
                            agent = agents.get_agent_by_id(id)
                            if agent is not None: # 必须先有 position, 故仅收到未见过的 agent 的 velocity 不予创建
                                agent.velocity = [vx, vy]
                            
                            canvas_updated = True
                        elif int(command[0]) == 2: # 目标: 2, id, tx, ty
                            id, tx, ty = float(command[1]), float(command[2]), float(command[3])
                            agent = agents.get_agent_by_id(id)
                            if agent is not None: # 必须先有 position, 故仅收到未见过的 agent 的 target_position 不予创建
                                agent.target_position = [tx, ty]
                            
                            canvas_updated = True
                        else:
                            pass
                    
                    # 如果有画布信息更新则广播
                    if canvas_updated:
                        await clients.broadcast(json.dumps({
                            'type': 'plot',
                            'agents': agents.to_list(),
                            'anchors': anchors.to_list()
                        }))
                except Exception as e:
                    print(f'{e}') # TODO
        await asyncio.sleep(SERIAL_POLL_INTERVAL)

async def mock_serial_talker():
    while True:
        if serials.is_ready():
            # print('mock')
            serials.write(f'0,233,{random.uniform(-1, 3)},{random.uniform(0, 4)}\n'.encode())
            serials.write(f'1,233,{random.uniform(-1, 3)},{random.uniform(0, 4)}\n'.encode())
            serials.write(f'2,233,{random.uniform(-1, 3)},{random.uniform(0, 4)}\n'.encode())
        await asyncio.sleep(0.1)

@app.listener('before_server_start')
async def setup_background_task(app, loop):
    loop.create_task(serial_listener())
    loop.create_task(mock_serial_talker())


"""
    启动服务端
"""
if __name__ == '__main__':
    app.run(host = APP_HOST, port = APP_PORT)

