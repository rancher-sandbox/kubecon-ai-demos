import express from 'express'
import expressWs from 'express-ws'
import rtspRelay from 'rtsp-relay'

const url = process.env.RTSP_URL || `rtsp://udev-camera-svc:8554/rps`

// HTTP & Websocket Server
const app = express()
expressWs(app)

// the endpoint our RTSP uses
app.ws('/stream',  rtspRelay(app)({
  url,
  verbose: true
}))


app.listen(process.env.PORT || 8080, () => {
  console.log('Listening')
})
