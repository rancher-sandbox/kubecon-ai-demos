import express from 'express'
import { createServer } from "http"
import { Server } from "socket.io"

import { connect, StringCodec } from 'nats'
const sc = StringCodec();

const nats_url = process.env.NATS_URL
const frames_to_change = process.env.FRAME_COUNTER || 20

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


const publishAfterTime = (topic, msg, delay)=>{
  return new Promise((resolve)=>{
    setTimeout(()=>{
      natsClient.publish(topic, sc.encode(msg))
      resolve()
    },delay)
  })
}

const runRound = async ()=>{
  console.log('round start')

  natsClient.publish('round.start', "")

  await publishAfterTime('round.countdown', "3", 1000)
  await publishAfterTime('round.countdown', "2", 1000)
  await publishAfterTime('round.countdown', "1", 1000)

  //TODO parallelize with wait time

  const {data} = await natsClient.request('get_computer_move', "", {timeout:1000})
  const computer_move = sc.decode(data)

  await publishAfterTime('round.end', JSON.stringify({
    robotPlay: computer_move.toUpperCase()
  }), 1000)
} 


// State

let currentPlayerMove = ''
let nextPlayerMove = ''
let nextPlayerMoveCounter = 0

// TODO timeouts for reset on no move

let timeout = null

let lastPlayerMoves = []
const addDetection = (move)=>{
  console.log('detected: ', move)
  currentPlayerMove = move

  lastPlayerMoves.push(move)
  if(lastPlayerMoves.length>3) {
    lastPlayerMoves.shift()
  }

  if (!timeout) {
    clearTimeout(timeout)
    timeout=null
  }
  timeout = setTimeout(()=>{
    console.log('Timed out so clearing moves')
    currentPlayerMove = ''
    lastPlayerMoves = []
    nextPlayerMoveCounter = 0
  },5000)
}


const shouldStart = ()=>(
  (lastPlayerMoves.length==3) &&
  (lastPlayerMoves[0] == 'ROCK') &&
  (lastPlayerMoves[1] == 'PAPER') &&
  (lastPlayerMoves[2] == 'SCISSORS')
)


const broadcast = (topic, message)=>{
  events.emit('msg', JSON.stringify({
    topic, 
    message: (typeof message == 'object')?JSON.stringify(message): message
  }))
}



const startSub = async () => {

  // WTF is this pattern...
  // From https://github.com/nats-io/nats.js/blob/main/examples/nats-sub.js
  for await (const m of sub) {
    const topic = m.subject
    const message = sc.decode(m.data)
    //console.log(`[${sub.getProcessed()}]: ${topic}: ${message}`)

    if(topic == 'human_move') {
      //currentPlayerMove = message
      if(currentPlayerMove != message ) {
        if(nextPlayerMove == message) { // if still same as last frame(s)
          if (++nextPlayerMoveCounter > frames_to_change){ // increment counter and only change when enough frames in a row (detection has stayed the same for a while)
            addDetection(message)
            broadcast('detection', currentPlayerMove)
            if(shouldStart()) {
              runRound()
            }
          }
        } else { // reset counter
          nextPlayerMoveCounter = 0
          nextPlayerMove = message
        }
      }
    }

    if(topic == 'computer_move') {
      console.log(topic,message)
      const robotPlay = message
      broadcast(topic,{
        robotPlay,
        humanPlay: currentPlayerMove
      })
    }

    if(topic == 'round.end') {
      const {robotPlay} = JSON.parse(message)
      broadcast(topic,{
        robotPlay,
        humanPlay: currentPlayerMove
      })
    }

    if(topic == 'round.countdown' || topic == 'round.start') {
      broadcast(topic,message)
    }
  }
}


startSub().then() 
