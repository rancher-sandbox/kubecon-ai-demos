FROM restreamio/gstreamer:latest-prod
RUN apt-get update && apt-get install -y software-properties-common gcc && \
    add-apt-repository -y ppa:deadsnakes/ppa
RUN apt-get update && apt-get install -y  python3-distutils python3-pip python3-apt
WORKDIR /usr/src
COPY determine_human_move.py .
COPY training_data.csv .
COPY requirements.txt .
RUN pip install -r requirements.txt
ENTRYPOINT [ "python3", "determine_human_move.py" ]
