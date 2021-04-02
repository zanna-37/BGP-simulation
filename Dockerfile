# Local build environment
#
# Build:
#     docker build -t igang/build-cpp-env -f Dockerfile .
#
# Run:
#     docker run --rm -v "absolute/path/to/project:/project" --name build_env igang/build-cpp-env [make command]
#

FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
# Europe/Helsinki
ENV TZ=Europe/Rome

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    gdb \
    cmake \
    valgrind \
	&& rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    # PcapPlusPlus requirements
    libpcap-dev \
    # END: PcapPlusPlus requirements
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project

CMD ["make", "build"]
