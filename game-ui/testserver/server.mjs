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

const runRound = async ()=>{

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


// Testing endpoint 
app.get('/runtest', async (req,res)=>{
  await runRound()
  res.send("done")
})


const formPageHtml = `
<html>
<head><title>RPS Control</title></head>
<body>
  <form action="#" method="POST">
    <button type="submit">Start Game</button>
  </form>
</body>
</html>
`

app.post('/runRound', async (req,res)=>{
  if (!!req.query.cheat) {
    //TODO
    console.log('NO CHEATING')
  }
  runRound()
  res.send(formPageHtml)
})


app.get('/runRound', async (req,res)=>{
  res.send(formPageHtml)
})