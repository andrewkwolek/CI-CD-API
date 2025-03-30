FROM ubuntu:latest

RUN apt update && apt install -y build-essential cmake autoconf libtool pkg-config
RUN apt update && apt install -y libgrpc++-dev libgrpc-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc

WORKDIR /app
COPY proto_files /app/proto_files
COPY server/src /app/src
COPY server/CMakeLists.txt /app

WORKDIR /app/server
RUN mkdir -p build && cd build 
RUN cmake ..
RUN make -j 4

EXPOSE 8082

ENTRYPOINT [ "./rest_api_server" ]
