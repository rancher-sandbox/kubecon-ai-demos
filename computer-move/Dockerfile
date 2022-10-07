FROM python:3.8.14-alpine3.16

WORKDIR /usr/src
COPY generate_computer_move.py .
COPY requirements.txt .
RUN pip install -r requirements.txt
ENTRYPOINT ["python", "generate_computer_move.py"]
