import express from 'express'
import { createServer } from "http"
import { connect, StringCodec } from 'nats'
const sc = StringCodec()
const nats_url = process.env.NATS_URL


// HTTP & Websocket Server
const app = express()

const httpServer = createServer(app)

httpServer.listen(process.env.PORT || 8080, () => {
  console.log('Listening')
})

const natsClient = await connect({servers:[nats_url]})

// For testing endpoint 
const publishAfterTime = (topic, msg, delay)=>{
  return new Promise((resolve)=>{
    setTimeout(()=>{
      natsClient.publish(topic, sc.encode(msg))
      resolve()
    },delay)
  })
}
// Testing endpoint 
app.get('/runtest',async (req,res)=>{

  natsClient.publish('round.start',"")

  await publishAfterTime('round.countdown',"3", 1000)
  await publishAfterTime('round.countdown',"2", 1000)
  await publishAfterTime('round.countdown',"1", 1000)
  const {data} = await natsClient.request('get_computer_move',"", {timeout:1000})
  const computer_move = sc.decode(data)

  await publishAfterTime('round.end', JSON.stringify({
    robotPlay: computer_move.toUpperCase()
  }), 1000)

  res.send(204)
})
