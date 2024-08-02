# 使用官方Python基础镜像
FROM python:3.9-slim

# 设置工作目录
WORKDIR /app

# 复制当前目录内容到工作目录
COPY . /app

# 安装Flask
RUN pip install flask

# 暴露端口
EXPOSE 5000

# 运行Flask应用
CMD ["python", "server.py"]