import express from 'express'
import { createServer } from "http"
import * as mqtt from 'mqtt'

const mqtt_url = process.env.MQTT_URL


// HTTP & Websocket Server
const app = express()

const httpServer = createServer(app)

httpServer.listen(process.env.PORT || 8080, () => {
  console.log('Listening')
})

// MQTT
const mqttClient = mqtt.connect(`mqtt://${mqtt_url}`)

const options = ['ROCK', 'PAPER', 'SCISSORS']

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

  const robotPlay = options[Math.floor(Math.random() * options.length)]
  const endMsg = JSON.stringify({
    robotPlay
  })

  mqttClient.publish('round/start',"")

  await publishAfterTime('round/countdown',"3", 1000)
  await publishAfterTime('round/countdown',"2", 1000)
  await publishAfterTime('round/countdown',"1", 1000)
  await publishAfterTime('round/end', endMsg, 1000)

  res.send(204)
})
