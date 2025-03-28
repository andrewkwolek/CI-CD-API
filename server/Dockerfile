FROM ubuntu:latest

RUN apt update && apt install -y build-essential cmake libboost-all-dev

RUN mkdir /app
COPY src /app/src
ADD CMakeLists.txt /app
RUN mkdir -p /app/build

WORKDIR /app/build

RUN cmake ..
RUN make

EXPOSE 8082

ENTRYPOINT [ "./rest_api_server", "0.0.0.0", "8082" ]
