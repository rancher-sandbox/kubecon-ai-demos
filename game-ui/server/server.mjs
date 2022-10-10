import express from 'express'
import { createServer } from "http"
import { Server } from "socket.io"

import { connect, StringCodec } from 'nats'
const sc = StringCodec();

const nats_url = process.env.NATS_URL

// HTTP & Websocket Server
const app = express()

const httpServer = createServer(app)

const io = new Server(httpServer, {
  cors: {
    origin: "*",
    methods: ["GET", "POST", "HEAD"]
  }
})

const events = io.of("/events")


httpServer.listen(process.env.PORT || 8080, () => {
  console.log('Listening')
})

const natsClient = await connect({servers:[nats_url]})

const sub = natsClient.subscribe('>')



let currentPlayerMove = ''


const startSub = async () => {

  // WTF is this pattern...
  // From https://github.com/nats-io/nats.js/blob/main/examples/nats-sub.js
  for await (const m of sub) {
    const topic = m.subject
    const message = sc.decode(m.data)
    console.log(`[${sub.getProcessed()}]: ${topic}: ${message}`)

    if(topic == 'human_move') {
      currentPlayerMove = message
    }

    if(topic == 'round.end') {
      const {robotPlay} = JSON.parse(message)
      events.emit('msg', JSON.stringify({
        topic, 
        message: JSON.stringify({
          robotPlay,
          humanPlay: currentPlayerMove
        })
      }))
    }

    if(topic == 'round.countdown' || topic == 'round.start') {
      events.emit('msg', JSON.stringify({
        topic, 
        message
      }))
    }

  }
}


startSub().then() 
