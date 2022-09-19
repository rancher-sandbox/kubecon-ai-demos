import express from 'express'
import { createServer } from "http"
import { Server } from "socket.io"
import * as mqtt from 'mqtt'

// Read configuration
import { readFileSync } from 'fs'
const mqtt_url = readFileSync('/configurations/mqtt/url', 'utf8')
const username = readFileSync('/configurations/mqtt/username', 'utf8')
const password = readFileSync('/configurations/mqtt/password', 'utf8')


// HTTP & Websocket Server
const app = express()
app.use(express.static('client/build'))

const httpServer = createServer(app)
const io = new Server(httpServer)
const events = io.of("/events")

httpServer.listen(process.env.PORT || 8080, () => {
  console.log('Listening')
})


// MQTT
const mqttClient = mqtt.connect(mqtt_url, {username, password})

console.log(mqttClient)


mqttClient.on('connect', () => {
  console.log('SUBSCRIBING TO MQTT')
  mqttClient.subscribe('#', (err) => {
    if (!!err) {
      console.error('ERROR MQTT SUB', err)
      process.exit()
    } else {
      console.log('MQTT SUBSCRIBE')
    }
  })
})

mqttClient.on('error', (err)=>{
  console.error('ERROR MQTT ', err)
})


mqttClient.on('message', (topic, message)=>{
  // message is Buffer
  console.log('Message: ',topic, " -- ",message.toString())
  events.emit('msg', JSON.stringify({
    topic, 
    message: message.toString()
  }))
})