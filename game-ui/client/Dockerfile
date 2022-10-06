FROM registry.suse.com/bci/nodejs:latest as build-client
WORKDIR /usr/src/app
COPY package*.json ./
RUN npm i

COPY . .

RUN npm run build 


FROM nginx

COPY --from=build-client /usr/src/app/build /usr/share/nginx/html 
