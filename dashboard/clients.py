import json

from sanic import Websocket


class WebSocketClientsPool:
    def __init__(self):
        self.clients = []
    
    def append(self, client: Websocket):
        self.clients.append(client)
    
    def remove(self, client: Websocket):
        self.clients.remove(client)
    
    async def broadcast(self, msg: str):
        for client in self.clients:
            try:
                await client.send(msg)
            except Exception as e:
                # print(f"Failed to send message to a client: {e}")
                print(f"Deprecated client removed.")
                self.clients.remove(client) # TODO
    
