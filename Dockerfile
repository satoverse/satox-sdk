# Multi-stage build for Satox SDK
FROM ubuntu:22.04 AS base

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV SATOX_SDK_ROOT=/usr/local/satox-sdk
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SATOX_SDK_ROOT/lib
ENV PYTHONPATH=$PYTHONPATH:$SATOX_SDK_ROOT/lib/python3.10/site-packages

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    python3 \
    python3-pip \
    python3-dev \
    libssl-dev \
    libboost-all-dev \
    libspdlog-dev \
    libgtest-dev \
    libgmock-dev \
    nlohmann-json3-dev \
    libfmt-dev \
    libgflags-dev \
    valgrind \
    gdb \
    curl \
    wget \
    unzip \
    libcurl4-openssl-dev \
    libsqlite3-dev \
    nodejs \
    npm \
    golang-go \
    rustc \
    cargo \
    lua5.4 \
    liblua5.4-dev \
    openjdk-17-jdk \
    maven \
    dotnet-sdk-6.0 \
    librocksdb-dev \
    libsnappy-dev \
    libzstd-dev \
    liblz4-dev \
    libbz2-dev \
    libgmp-dev \
    libmpfr-dev \
    libmpc-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Python dependencies
RUN pip3 install --no-cache-dir \
    pytest \
    pytest-cov \
    pytest-mock \
    black \
    flake8 \
    mypy \
    setuptools \
    wheel \
    build

# Install Node.js dependencies
RUN npm install -g typescript @types/node

# Build stage
FROM base AS builder

# Install prometheus-cpp dependencies
RUN apt-get update && apt-get install -y \
    libcurl4-openssl-dev \
    libssl-dev \
    libgtest-dev \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Build and install prometheus-cpp
WORKDIR /tmp
RUN git clone --recurse-submodules https://github.com/jupp0r/prometheus-cpp.git && \
    cd prometheus-cpp && \
    mkdir -p _build && cd _build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DENABLE_PUSH=OFF -DENABLE_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/usr/local/prometheus-cpp && \
    make -j$(nproc) && \
    make install

# Copy source code
WORKDIR /workspace
COPY . .

# Clean up any existing build artifacts and create fresh build directory
RUN rm -rf build && mkdir -p build

# Configure and build the project with all components and bindings
RUN cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local/satox-sdk \
        -DCMAKE_PREFIX_PATH=/usr/local/prometheus-cpp \
        -DSATOX_CORE_TESTING=ON \
        -DBUILD_TESTS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DENABLE_LOGGING=ON \
        -DENABLE_MONITORING=ON \
        -DENABLE_QUANTUM_SECURITY=ON \
        -DBUILD_PYTHON_BINDINGS=ON \
        -DBUILD_NODEJS_BINDINGS=ON \
        -DBUILD_GO_BINDINGS=ON \
        -DBUILD_RUST_BINDINGS=ON \
        -DBUILD_JAVA_BINDINGS=ON \
        -DBUILD_CSHARP_BINDINGS=ON \
        -DBUILD_LUA_BINDINGS=ON \
        -DBUILD_WASM_BINDINGS=ON

# Build the project
RUN cd build && make -j$(nproc)

# Build Python bindings
RUN cd /workspace/bindings/python && \
    python3 setup.py build_ext --inplace && \
    python3 setup.py install

# Build Node.js/TypeScript bindings
RUN cd /workspace/bindings/typescript && \
    npm install && \
    npm run build

# Build JavaScript bindings
RUN cd /workspace/bindings/javascript && \
    npm install && \
    npm run build

# Build Go bindings
RUN cd /workspace/bindings/go && \
    go mod tidy && \
    go build -o satox-sdk-go.so -buildmode=c-shared .

# Build Rust bindings
RUN cd /workspace/bindings/rust && \
    cargo build --release

# Build Java bindings
RUN cd /workspace/bindings/java && \
    mvn clean package

# Build C# bindings
RUN cd /workspace/bindings/csharp && \
    dotnet build -c Release

# Build Lua bindings
RUN cd /workspace/bindings/lua && \
    make

# Build WebAssembly bindings
RUN cd /workspace/bindings/wasm && \
    npm install && \
    npm run build

# Install shared libraries to standard location for tests
RUN cd build && \
    find . -name "*.so" -exec cp {} /usr/local/lib/ \; && \
    ldconfig

# Run tests with proper library paths
ENV LD_LIBRARY_PATH=/usr/local/lib:/workspace/build:$LD_LIBRARY_PATH

# Install the project
RUN cd build && make install

# Install bindings to appropriate locations
RUN mkdir -p $SATOX_SDK_ROOT/bindings && \
    cp -r /workspace/bindings/python $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/typescript $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/javascript $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/go $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/rust $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/java $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/csharp $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/lua $SATOX_SDK_ROOT/bindings/ && \
    cp -r /workspace/bindings/wasm $SATOX_SDK_ROOT/bindings/

# Copy additional component headers and libraries
RUN mkdir -p $SATOX_SDK_ROOT/include/satox && \
    cp -r /workspace/satox-quantum/include/* $SATOX_SDK_ROOT/include/satox/quantum/ 2>/dev/null || true && \
    cp -r /workspace/satox-transactions/include/* $SATOX_SDK_ROOT/include/satox/transactions/ 2>/dev/null || true && \
    cp -r /workspace/satox-assets/include/* $SATOX_SDK_ROOT/include/satox/assets/ 2>/dev/null || true && \
    cp -r /workspace/satox-api/include/* $SATOX_SDK_ROOT/include/satox/api/ 2>/dev/null || true

# Runtime stage
FROM ubuntu:22.04 AS runtime

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV SATOX_SDK_ROOT=/usr/local/satox-sdk
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SATOX_SDK_ROOT/lib
ENV PYTHONPATH=$PYTHONPATH:$SATOX_SDK_ROOT/lib/python3.10/site-packages
ENV NODE_PATH=$NODE_PATH:$SATOX_SDK_ROOT/bindings/typescript/lib
ENV GOPATH=$GOPATH:$SATOX_SDK_ROOT/bindings/go
ENV RUST_BACKTRACE=1

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    libssl3 \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-thread1.74.0 \
    libspdlog1 \
    libfmt8 \
    libcurl4 \
    libsqlite3-0 \
    curl \
    nodejs \
    npm \
    golang-go \
    rustc \
    lua5.4 \
    openjdk-17-jre \
    dotnet-runtime-6.0 \
    librocksdb7.8 \
    libsnappy1v5 \
    libzstd1 \
    liblz4-1 \
    libbz2-1.0 \
    libgmp10 \
    libmpfr6 \
    libmpc3 \
    && rm -rf /var/lib/apt/lists/*

# Create satox user
RUN useradd -m -s /bin/bash satox && \
    mkdir -p /home/satox/logs /home/satox/data /home/satox/config /home/satox/bindings && \
    chown -R satox:satox /home/satox

# Copy installed files from builder
COPY --from=builder $SATOX_SDK_ROOT $SATOX_SDK_ROOT

# Copy Python packages
COPY --from=builder /usr/local/lib/python3.10/dist-packages /usr/local/lib/python3.10/dist-packages

# Copy bindings to user directory
RUN cp -r $SATOX_SDK_ROOT/bindings/* /home/satox/bindings/ && \
    chown -R satox:satox /home/satox/bindings

# Set up environment
RUN echo "export SATOX_SDK_ROOT=$SATOX_SDK_ROOT" >> /home/satox/.bashrc && \
    echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH" >> /home/satox/.bashrc && \
    echo "export PYTHONPATH=$PYTHONPATH" >> /home/satox/.bashrc && \
    echo "export NODE_PATH=$NODE_PATH" >> /home/satox/.bashrc && \
    echo "export GOPATH=$GOPATH" >> /home/satox/.bashrc && \
    echo "export RUST_BACKTRACE=1" >> /home/satox/.bashrc

# Switch to satox user
USER satox
WORKDIR /home/satox

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Default command
CMD ["/usr/local/satox-sdk/bin/satox-sdk"] 