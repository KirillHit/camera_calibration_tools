FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
        build-essential \
        cmake \
        ninja-build \
        ca-certificates \
        # Project-specific dependencies
        qt6-base-dev \
        qt6-base-dev-tools \
        libopencv-dev \
        libceres-dev \
        qt6-tools-dev \
        # Packaging
        dpkg \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["bash"]