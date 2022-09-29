FROM registry.suse.com/bci/nodejs:latest
WORKDIR /usr/src/app
COPY package*.json ./
RUN npm i

COPY . .

EXPOSE 8080

CMD node server.mjs