FROM ubuntu:20.10

RUN apt-get update -y --fix-missing                                                     \
    && apt-get upgrade -y                                                               \
    && apt-get install -y --no-install-recommends ca-certificates git gdb gcc-10 g++-10 \
    wget vim build-essential ccache doxygen cppcheck clang clang-format

RUN rm -rf /var/lib/apt/lists/*

ENV CMAKE_VERSION=3.19.5
ENV CMAKE_DIR=3.19
RUN wget -qO- "https://cmake.org/files/v${CMAKE_DIR}/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local

RUN useradd --uid 1000 -m user

ENV HOME /home/user
WORKDIR /home/user

CMD ["bash"]