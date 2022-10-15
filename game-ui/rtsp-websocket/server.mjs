import express from 'express'
import expressWs from 'express-ws'
import rtspRelay from './rtsp-relay.mjs'

const url = process.env.RTSP_URL || `rtsp://udev-camera-svc:8554/rps`

// HTTP & Websocket Server
const app = express()
expressWs(app)
const {proxy} = rtspRelay(app)
// the endpoint our RTSP uses
app.ws('/stream', proxy({
  url,
  verbose: true
}))


app.listen(process.env.PORT || 8080, () => {
  console.log('Listening')
})
