import json
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
                # 输入缓存中有数据
                data = serials.read()
                
                # TODO 处理指令
                
                # TODO 如果有信息更新则广播画布更新信息
                # if
                await clients.broadcast(json.dumps({
                    'type': 'plot',
                    'agents': agents.to_list(),
                    'anchors': anchors.to_list()
                }))
        await asyncio.sleep(SERIAL_POLL_INTERVAL)

async def mock_serial_talker():
    while True:
        if serials.is_ready():
            # print('mock')
            serials.write(b'mock')
        await asyncio.sleep(1.0)

@app.listener('before_server_start')
async def setup_background_task(app, loop):
    loop.create_task(serial_listener())
    loop.create_task(mock_serial_talker())


"""
    启动服务端
"""
if __name__ == '__main__':
    app.run(host = APP_HOST, port = APP_PORT)

