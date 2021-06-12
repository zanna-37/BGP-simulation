# Docs builder

FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
	&& rm -rf /var/lib/apt/lists/*

RUN pip3 install sphinx; \
    pip3 install recommonmark; \
    pip3 install sphinx-rtd-theme;

WORKDIR /docs

CMD ["make", "html"]
