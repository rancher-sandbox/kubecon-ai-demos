FROM registry.suse.com/bci/nodejs:latest
WORKDIR /usr/src/app
COPY package*.json ./
RUN npm i

COPY . .

RUN npm run build-client

EXPOSE 8080
