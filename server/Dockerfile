FROM ubuntu:20.04 as dev

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    cmake \
    clang-10 \
    libc++-10-dev \
    libc++abi-10-dev \
    lld-10 \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install -U pip && python3 -m pip install "conan<2"

RUN mkdir -p /opt/server/
WORKDIR /opt/server/

COPY . .
CMD ["/bin/bash", "-c", "./build.sh && ./build/bin/server"]

FROM dev as builder
RUN ./build.sh

FROM alpine:latest as release
COPY --from=builder /opt/server/build/bin/server /root/server
CMD ["/root/server"]
