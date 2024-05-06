FROM python:3.9-slim

WORKDIR /app

COPY requirements.txt ./

RUN pip install --no-cache-dir -r requirements.txt

COPY app.py .

ENV PORT 5000
ENV NEXT_SERVICE_URL "http://localhost:5000/"
ENV SERVICE_NAME "service"

# 对外暴露端口
EXPOSE $PORT

# 运行应用
CMD ["python", "./app.py"]
