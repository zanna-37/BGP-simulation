# Local build environment
#
# Build:
#     docker build -t igang/build-cpp-env -f Dockerfile .
#
# Run:
#     docker run --rm -v "absolute/path/to/project:/project" --name build_env igang/build-cpp-env [make command]
#

FROM ubuntu:20.04 AS base

ARG DEBIAN_FRONTEND=noninteractive

# Europe/Helsinki
ENV TZ=Europe/Rome

ENV CLANG_VERSION=11

RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    cmake \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    clang-${CLANG_VERSION} \
    && rm -rf /var/lib/apt/lists/*

RUN ln -s /usr/bin/clang++-${CLANG_VERSION} /usr/bin/clang++ && \
    ln -s /usr/bin/clang-${CLANG_VERSION}   /usr/bin/clang



FROM base

RUN apt-get update && apt-get install -y \
    # PcapPlusPlus requirements
    libpcap-dev \
    # END: PcapPlusPlus requirements
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    # For vscode TODO create another Dockerfile
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project

CMD ["make", "build"]
