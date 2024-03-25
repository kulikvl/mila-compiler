FROM debian:latest

WORKDIR /usr/src/app

RUN apt-get update && apt-get install -y \
    llvm \
    llvm-dev \
    clang \
    git \
    cmake \
    zlib1g-dev \
    micro

COPY . .

RUN mkdir build && cd build && cmake .. && make && mv src/main/mila ../milac

ENTRYPOINT bash

# If you dont want to install LLVM locally, you can use the docker container to compile the code
# docker build -t mila-compiler-image .
# docker run -it --name=mila-compiler -v ./input.mila:/usr/src/app/input.mila mila-compiler-image
# Now write code in input.mila and compile via the mila compiler from the debian docker container