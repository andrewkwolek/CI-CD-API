FROM python:3.9-slim-bullseye

COPY client /app
RUN python -m pip install /app --extra-index-url https://www.piwheels.org/simple

EXPOSE 8002

ENTRYPOINT [ "main.py" ]