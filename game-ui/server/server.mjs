import express from 'express'
import { createServer } from "http"
import { Server } from "socket.io"

import { connect, StringCodec } from 'nats'
const sc = StringCodec();

const NATS_URL = process.env.NATS_URL
const TRACE = process.env.TRACE || false
const FRAMES_BEFORE_CHANGE = process.env.FRAME_COUNTER || 10
const DETECTION_TIMEOUT = process.env.DETECTION_TIMEOUT || 5000
const BROADCAST_ALL_DETECTIONS = process.env.BROADCAST_ALL_DETECTIONS || true


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

const natsClient = await connect({servers:[NATS_URL]})

const sub = natsClient.subscribe('>')

const timoutAsync = (delay) => (
  new Promise((resolve)=>{
    setTimeout(()=>{
      resolve()
    },delay)
  })
)

const publishAfterTime = async (topic, msg, delay)=>{
  await timoutAsync(delay)
  natsClient.publish(topic, sc.encode(msg))
}

let roundStarting = false //MUTEX
const runRound = async (cheat)=>{ // TODO Cheat mode
  if(roundStarting) return
  roundStarting = true
  
  console.log('round start')

  natsClient.publish('round.start', "")

  await publishAfterTime('round.countdown', "3", 1000)
  await publishAfterTime('round.countdown', "2", 1000)
  await publishAfterTime('round.countdown', "1", 1000)

  const natsProm = natsClient.request('get_computer_move', "", {timeout:1000})

  const [{data}, ..._]  = await Promise.all([natsProm, timoutAsync(1000)])
  const computer_move = sc.decode(data)

  natsClient.publish('round.end', JSON.stringify({
    robotPlay: computer_move.toUpperCase()
  }))

  roundStarting = false
} 


// State

let currentPlayerMove = ''
let nextPlayerMove = ''
let nextPlayerMoveCounter = 0

let timeout = null

let lastPlayerMoves = []
const addDetection = (move)=>{
  console.log('detected: ', move)
  currentPlayerMove = move

  lastPlayerMoves.push(move)
  if(lastPlayerMoves.length>3) {
    lastPlayerMoves.shift()
  }

  if (!!timeout) {
    clearTimeout(timeout)
    timeout=null
  }

  timeout = setTimeout(()=>{
    console.log('Timed out so clearing moves')
    currentPlayerMove = ''
    lastPlayerMoves = []
    nextPlayerMoveCounter = 0

    broadcast('detection', '')
  }, DETECTION_TIMEOUT)
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
    if(TRACE) console.log(`[${sub.getProcessed()}]: ${topic}: ${message}`)

    if(topic == 'human_move') {
      if (BROADCAST_ALL_DETECTIONS) broadcast('human_move', message)
      
      // Only change "current move" if player stays steady for several frames
      if(currentPlayerMove != message ) {
        if(nextPlayerMove == message) { // if still same as last frame(s)
          if (++nextPlayerMoveCounter > FRAMES_BEFORE_CHANGE){ // increment counter and only change when enough frames in a row (detection has stayed the same for a while)
            addDetection(message)
            broadcast('detection', currentPlayerMove)
            if(shouldStart()) {
              runRound(false)
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



// Testing and manual trigger
const formPageHtml = `
<html>
<head>
<title>RPS Control</title>
<style>
form {
  width: 100%;
  height: 50%;
}
button {
  width: 100%;
  height: 100%;
}
</style>
</head>
<body>
  <form action="#?cheat=no" method="GET">
    <button type="submit">Start Game</button>
  </form>
  <form action="#?cheat=yes" method="GET">
    <button type="submit">Cheat</button>
  </form>
</body>
</html>
`

app.post('/runRound', async (req,res)=>{
  console.log('Query: ',req.query)
  if (!!req.query.cheat) {
    if(req.query.cheat ==  'no')
      runRound(false)
    else
      runRound(true)
  }
  res.send(formPageHtml)
})


app.get('/runRound', async (req,res)=>{
  res.send(formPageHtml)
})