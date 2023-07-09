FROM alpine
RUN apk update
RUN apk add cmake
RUN apk add coreutils gcc g++ make
RUN apk add curl-dev openssl-dev
RUN apk add git
RUN apk add bash
RUN apk add curl zip unzip tar
RUN apk add build-base ninja zip unzip curl git
RUN apk add linux-headers
RUN apk add perl
RUN apk add openssl-libs-static

# oribli
RUN git clone --bare https://github.com/r-const-dev/oribli.git /usr/local/src/oribli
WORKDIR /usr/local/src/oribli
RUN make && make install

# vcpkg
RUN git clone https://github.com/Microsoft/vcpkg.git /opt/vcpkg
# VCPKG_FORCE_SYSTEM_BINARIES=1 is required by vcpkg on alpine
ENV VCPKG_FORCE_SYSTEM_BINARIES=1
RUN /opt/vcpkg/bootstrap-vcpkg.sh
# vcpkg builds release and debug by default for all packages; disable the default and only build release version
RUN echo "set(VCPKG_BUILD_TYPE release)" >> /opt/vcpkg/triplets/x64-linux.cmake
