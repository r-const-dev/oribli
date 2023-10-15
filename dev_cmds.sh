#!/bin/sh
# Common commands for development.

# Ensure that a mongo instance is running on the local machine, with default params.
EnsureLocalMongo() {
  if [ ! "$(docker ps -a -q -f name='^mongo$')" ]; then
    set -x
    docker run --network host --name mongo --rm -d mongo
    set +x
  fi
}

# Standard cmake build command.
CMakeDefault() {
  if [ ! -d build ]; then
    mkdir build
  fi
  cd build
  cmake -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake ..
  make
}
