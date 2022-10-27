import React, { useEffect, memo } from 'react'

let player = null

function RTSP({
    playerRef: playerCamRef = React.createRef(),
    src,
    ...props
}) {
  console.log('rendering rtsp element')
  if (!playerCamRef || !playerCamRef.current) console.warn('playerCamRef is null')

  useEffect(() => {
    console.log('Creating player')

    if (player) {
      console.log('Player is not null, destroying')
      player.source.abort()
      player.destroy()
    }

    console.log('Creating player for real')
    player = new window.JSMpeg.Player(src, {
      canvas: playerCamRef.current,
      audio: false,
      streaming: true // TODO
      // disableGl ?
    })

  }, [playerCamRef, src])

  return <canvas ref={playerCamRef} {...props} className="camera" />
}

export default memo(RTSP)