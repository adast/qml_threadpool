FROM ubuntu:18.04
ARG DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y --no-install-recommends \
        # Build tools
        build-essential cmake \
        # Qt
        qt5-default qtdeclarative5-dev qml-module-qtquick-controls \
        qml-module-qtquick-controls2 \
        # For big int arithmetics
        libgmp-dev \ 
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Copy sources and switch to sources folder
COPY . /app

# Build app
RUN mkdir /app/build && cd /app/build \
    && cmake .. && make -j $(( $(nproc) + 1 ))

# Run app
CMD ["/app/build/qml_threadpool"]
