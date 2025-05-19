FROM docker.io/ubuntu:24.04

# 対話型プロンプトを表示しないようにする
ENV DEBIAN_FRONTEND=noninteractive

# パッケージリストの更新と必要なパッケージのインストール
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    git \
    python3 \
    python3-pip \
    pkg-config \
    libusb-1.0-0-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Pico SDKのクローンと設定
WORKDIR /opt
RUN git clone --recursive https://github.com/raspberrypi/pico-sdk.git
ENV PICO_SDK_PATH=/opt/pico-sdk

# ピコツールのクローン（ブランチ指定なし・オプション機能）
# RUN git clone https://github.com/raspberrypi/picotool.git \
#     && cd picotool \
#     && mkdir build \
#     && cd build \
#     && cmake .. \
#     && make -j4\
#     && cp picotool /usr/local/bin


# 汎用的なビルドスクリプトを追加
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
# The WORKDIR is /thermohygrometer\n\
# The project source subdirectory (e.g., '\''src'\'', containing the main CMakeLists.txt)\n\
# relative to /thermohygrometer is passed as the first argument.\n\
PROJECT_SRC_SUBDIR=${1:-src} # Default to '\''src'\'' if no argument is given\n\
\n\
echo "Building project from source subdirectory: ${PROJECT_SRC_SUBDIR} within /thermohygrometer"\n\
\n\
BASE_DIR="/thermohygrometer"\n\
BUILD_DIR="${BASE_DIR}/build"\n\
OUTPUT_DIR="${BASE_DIR}/output"\n\
PROJECT_SOURCE_PATH="${BASE_DIR}/${PROJECT_SRC_SUBDIR}"\n\
\n\
# Clean build directory if it exists and is not empty\n\
if [ -d "${BUILD_DIR}" ] && [ "$(ls -A "${BUILD_DIR}")" ]; then\n\
    echo "Build directory ${BUILD_DIR} exists and is not empty. Clearing it."\n\
    rm -rf "${BUILD_DIR:?}"/*\n\
fi\n\
\n\
# Create build and output directories\n\
mkdir -p "${BUILD_DIR}"\n\
mkdir -p "${OUTPUT_DIR}"\n\
\n\
# Configure CMake\n\
echo "Configuring CMake with source: ${PROJECT_SOURCE_PATH} and build directory: ${BUILD_DIR}"\n\
cmake -B "${BUILD_DIR}" -S "${PROJECT_SOURCE_PATH}" -DPICO_SDK_PATH=/opt/pico-sdk -DPICOTOOL_FORCE_FETCH_FROM_GIT=ON\n\
\n\
# Build the project\n\
echo "Building project in ${BUILD_DIR}"\n\
cmake --build "${BUILD_DIR}"\n\
\n\
# Copy .uf2 files to the output directory\n\
echo "Searching for .uf2 files in ${BUILD_DIR}"\n\
find "${BUILD_DIR}" -name "*.uf2" -exec cp {} "${OUTPUT_DIR}" \\;\n\
\n\
if [ -n "$(find "${OUTPUT_DIR}" -name '\''*.uf2'\'' -print -quit)" ]; then\n\
    echo "Build complete: .uf2 files copied to ${OUTPUT_DIR}"\n\
else\n\
    echo "Build complete, but no .uf2 files found in ${BUILD_DIR} to copy to ${OUTPUT_DIR}."\n\
fi\n\
' > /usr/local/bin/build-pico-project.sh

RUN chmod +x /usr/local/bin/build-pico-project.sh

# 作業ディレクトリの設定
WORKDIR /thermohygrometer
VOLUME ["/thermohygrometer"]

# ビルドコマンドのエントリポイント
CMD ["bash"]
