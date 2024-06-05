#!/bin/bash

# 构建镜像
docker build -t ubuntu_custom -f ./Dockerfile /root

# 运行并自动删除容器
docker run -it -v /boot/config-$(uname -r):/boot/config-$(uname -r) --rm ubuntu_custom bash

# 删除镜像
docker rmi ubuntu_custom
