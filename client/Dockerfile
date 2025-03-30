FROM python:3.9-slim-bullseye

WORKDIR /app
COPY proto_files /app/proto_files
COPY client/src /app/src
COPY client/pyproject.toml /app
RUN python -m pip install /app --extra-index-url https://www.piwheels.org/simple

RUN python -m grpc_tools.protoc -I/app/proto_files --python_out=/app/src --grpc_python_out=/app/src --pyi_out=/app/src /app/proto_files/data_transfer.proto

EXPOSE 8002

WORKDIR /app/src
ENTRYPOINT [ "python3", "main.py" ]