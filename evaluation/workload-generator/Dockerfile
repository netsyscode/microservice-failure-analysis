FROM alpine:latest as builder

RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apk/repositories && cat /etc/apk/repositories
RUN apk add --no-cache build-base git openssl-dev zlib-dev

RUN git clone https://github.com/giltene/wrk2.git

WORKDIR /wrk2
RUN make

FROM alpine:latest

RUN apk add --no-cache curl libgcc libstdc++

COPY --from=builder /wrk2/wrk /usr/local/bin/wrk

ENTRYPOINT [ "sleep", "infinity" ]