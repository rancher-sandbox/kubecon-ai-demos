import express from 'express'
import { createServer } from "http"
import { Server } from "socket.io"
import * as mqtt from 'mqtt'


// import { readFileSync } from 'fs'
// const mqtt_url = readFileSync('/services/mqtt/url', 'utf8')

const mqtt_url = 'test.mosquitto.org:1883'
const mqttClient  = mqtt.connect(mqtt_url)

const port = process.env.PORT || 8080

const app = express()
const httpServer = createServer(app)


const io = new Server(httpServer, {
  // Specifying CORS 
  cors: {
  origin: "*"
  }
})
const events = io.of("/events")

app.use(express.static('client/build'))

httpServer.listen(port, () => {
  console.log('ENV is: ', process.env)
})

mqttClient.on('connect', function () {
  mqttClient.subscribe('score', function (err) {
    if (!!err) {
      console.error('ERROR MQTT SUB', err)
      process.exit()
    }
  })
})


mqttClient.on('message', function (topic, message) {
  // message is Buffer
  console.log(message.toString())

  events.emit(topic,message)
})