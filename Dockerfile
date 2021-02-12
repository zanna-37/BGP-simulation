# Local build environment
#
# Build:
#     docker build -t igang/build-cpp-env -f Dockerfile .
#
# Run:
#     docker run --rm -v ".:/project" --name build_env igang/build-cpp-env [make command]
#
#
# Troubleshooting:
#     Windows requires full path when using docker run
# 	  example: docker run --rm -v "absolute\path\to\project:/project" --name build_env igang/build-cpp-env [make command]

FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
# Europe/Helsinki
ENV TZ=Europe/Rome

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    gdb \
    cmake \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /project

CMD ["make", "build"]
