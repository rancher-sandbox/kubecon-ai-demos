import express from 'express'
import { createServer } from "http"
import { Server } from "socket.io"
import * as mqtt from 'mqtt'

import { connect, StringCodec } from 'nats'
const sc = StringCodec();

// Read configuration
import { readFileSync } from 'fs'
const mqtt_url = readFileSync('/configurations/mqtt/url', 'utf8')


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
const mqttClient = mqtt.connect(`mqtt://${mqtt_url}`)

mqttClient.on('connect', () => {
  console.log('CONNECTED TO MQTT')
})

mqttClient.on('error', (err)=>{
  console.error('ERROR MQTT ', err)
})


// For testing endpoint 
const publishAfterTime = (topic, msg, delay)=>{
  return new Promise((resolve)=>{
    setTimeout(()=>{
      mqttClient.publish(topic, msg)
      resolve()
    },delay)
  })
}
// Testing endpoint 
app.get('/runtest',async (req,res)=>{
  const endMsg = JSON.stringify({
    winner: "human",
    humanPlay: "rock",
    robotPlay: "paper"
  })

  mqttClient.publish('round/start',"")

  await publishAfterTime('round/countdown',"3", 1000)
  await publishAfterTime('round/countdown',"2", 1000)
  await publishAfterTime('round/countdown',"1", 1000)
  await publishAfterTime('round/end', endMsg, 1000)
})


// Setup NATS 
try{
  const natsClient = await connect({servers:[mqtt_url]})

  const sub = natsClient.subscribe('>')

  // WTF is this pattern...
  // From https://github.com/nats-io/nats.js/blob/main/examples/nats-sub.js
  (async()=>{
    for await (const m of sub) {
      const topic = m.subject
      const message = sc.decode(m.data)
      console.log(`[${sub.getProcessed()}]: ${topic}: ${message}`);
      events.emit('msg', JSON.stringify({
        topic, 
        message
      }))
    }
  })().then() 

} catch(err) {
  console.error(err)
}
