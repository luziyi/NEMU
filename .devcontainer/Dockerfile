# 使用 Ubuntu 18.04 作为基础映像
FROM ubuntu:18.04

# 更新包列表并安装必要的软件
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-doc \
    gdb \
    git \
    time \
    libreadline-dev \
    libsdl-dev \
    vim \
    ctags \
    tmux \
    software-properties-common

# 添加 Trusty 仓库
RUN add-apt-repository 'deb http://archive.ubuntu.com/ubuntu/ trusty main' \
    && add-apt-repository 'deb http://archive.ubuntu.com/ubuntu/ trusty universe' \
    && apt-get update

# 如果需要，可以安装额外的软件包
RUN apt-get install -y gcc-4.4 gcc-4.4-doc gcc-4.4-multilib g++-4.4

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.4 10 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.4 10 \
    && update-alternatives --config gcc \
    && update-alternatives --config g++

# 清理缓存
RUN apt-get clean && rm -rf /var/lib/apt/lists/*
