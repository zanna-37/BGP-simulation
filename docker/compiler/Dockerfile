# Local build environment

FROM ubuntu:20.04 AS base

ARG DEBIAN_FRONTEND=noninteractive

# Europe/Helsinki
ENV TZ=Europe/Rome

# Keep this two version in sync otherwise iwyu doesn't find some headers.
# This is tracked in the github issue #100 (https://github.com/include-what-you-use/include-what-you-use/issues/100)
# See also https://github.com/include-what-you-use/include-what-you-use/blob/master/README.md#how-to-install.
#
# Reference to the version codes: https://github.com/include-what-you-use/include-what-you-use#how-to-build
ENV CLANG_VERSION=11
ENV IWYU_VERSION=clang_11

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



FROM base AS iwyu_builder

RUN apt-get update && apt-get install -y \
    llvm-${CLANG_VERSION}-dev \
    libclang-${CLANG_VERSION}-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir iwyu && mkdir iwyu/build && \
    git clone -b ${IWYU_VERSION} https://github.com/include-what-you-use/include-what-you-use.git \
        iwyu/include-what-you-use

WORKDIR /iwyu/build

ENV CC clang
ENV CXX clang++
ENV CMAKE_C_COMPILER clang
ENV CMAKE_CXX_COMPILER clang++

RUN cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/usr/lib/llvm-${CLANG_VERSION} ../include-what-you-use && \
    make && make install && \
    ln -s "$(command -v include-what-you-use)" /usr/local/bin/iwyu



FROM base

COPY --from=iwyu_builder /usr/local/bin/iwyu /usr/local/bin/include-what-you-use /usr/local/bin/

RUN apt-get update && apt-get install -y \
    # PcapPlusPlus requirements
    libpcap-dev \
    # END: PcapPlusPlus requirements
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project

CMD ["make", "build"]
