from flask import Flask
import requests
import os

app = Flask(__name__)

import logging
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)

# 从环境变量获取配置
PORT = int(os.getenv('PORT', 5000))
NEXT_SERVICE_URL = os.getenv('NEXT_SERVICE_URL', 'http://localhost:5000/')
SERVICE_NAME = os.getenv('SERVICE_NAME', 'service')

@app.route('/')
def get():
    if NEXT_SERVICE_URL != 'none':
        # 调用下一个服务
        response = requests.get(NEXT_SERVICE_URL)
        assert response.status_code == 200
        return f"Hello from the {SERVICE_NAME} + " + response.text
    else:
        # 如果没有下一个服务，返回静态文本
        return "Hello from the final service!"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=PORT)
