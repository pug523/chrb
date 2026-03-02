FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV XMAKE_ROOT=y
ENV XMAKE_COLORTERM=color256

RUN apt-get update && apt-get install -y \
    curl \
    build-essential \
    git \
    libreadline-dev \
    # unzip \
    # wget \
    # lsb-release \
    # software-properties-common \
    # gnupg \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://xmake.io/shget.text | bash

# RUN wget https://apt.llvm.org/llvm.sh \
#     && chmod +x llvm.sh \
#     && ./llvm.sh 21 all

ENV PATH="/root/.local/bin:$PATH"

WORKDIR /app

COPY . .

RUN xmake config -y -m release && xmake build

CMD ["xmake", "run"]
