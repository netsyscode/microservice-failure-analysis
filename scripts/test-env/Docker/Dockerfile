# 使用 Ubuntu 最新的官方镜像作为基础镜像
FROM ubuntu:latest

# 设置工作目录，可选
WORKDIR /root

# 更新 Ubuntu 的软件包源
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get clean

# 拷贝本地目录到容器的根目录
COPY ./microservice-failure-analysis /root/microservice-failure-analysis

# 容器启动时执行的命令，这里仅保持容器运行，可根据需要修改
CMD ["tail", "-f", "/dev/null"]
