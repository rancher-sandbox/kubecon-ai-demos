FROM python:3.10

# install libusb and python imaging library
RUN apt update && apt -y upgrade && \
apt install -y git curl vim libusb-1.0-0 python3-pil

WORKDIR /root

COPY listen_computer_move.py .
COPY requirements.txt .
RUN pip install -r requirements.txt


ENTRYPOINT ["/bin/sh"]