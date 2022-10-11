import React, { useEffect, memo } from 'react'
import { loadPlayer } from 'rtsp-relay/browser'

function RTSP({
    playerRef: playerCamRef = React.createRef(),
    src,
    ...props
}) {
  if (!playerCamRef.current) console.warn('playerCamRef is null')

  useEffect(() => {
    loadPlayer({
      url: src,
      canvas: playerCamRef.current
    })

  }, [playerCamRef, src])

  return <canvas ref={playerCamRef} {...props} className="camera" />
}

export default memo(RTSP)