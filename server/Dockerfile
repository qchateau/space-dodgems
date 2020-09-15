FROM ubuntu:20.04

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
        build-essential \
        cmake \
        clang-10 \
        libc++-10-dev \
        libc++abi-10-dev \
        lld-10 \
        python3 \
        python3-pip

RUN python3 -m pip install -U pip
RUN python3 -m pip install conan

RUN mkdir -p /opt/server/
WORKDIR /opt/server/

COPY . .

CMD ["/bin/bash", "-c", "./build.sh && ./build/bin/server $BIND_IP $PORT $NWORLDS"]
