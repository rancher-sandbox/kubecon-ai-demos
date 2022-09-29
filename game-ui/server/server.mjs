import express from 'express'
import { createServer } from "http"
import { Server } from "socket.io"

import { connect, StringCodec } from 'nats'
const sc = StringCodec();

const nats_url = process.env.NATS_URL

// HTTP & Websocket Server
const app = express()

const httpServer = createServer(app)
const io = new Server(httpServer)
const events = io.of("/events")

if (process.env)
io.on('connect', (socket)=>{
  socket.emit('system/videoSrc', process.env.CAMERA_URL)
})

httpServer.listen(process.env.PORT || 8080, () => {
  console.log('Listening')
})

const natsClient = await connect({servers:[nats_url]})

const sub = natsClient.subscribe('>')

const startSub = async () => {

  // WTF is this pattern...
  // From https://github.com/nats-io/nats.js/blob/main/examples/nats-sub.js
  for await (const m of sub) {
    const topic = m.subject
    const message = sc.decode(m.data)

    console.log(`[${sub.getProcessed()}]: ${topic}: ${message}`)

    events.emit('msg', JSON.stringify({
      topic, 
      message
    }))
  }

}
startSub().then() 
