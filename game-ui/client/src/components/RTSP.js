import React, { useEffect, memo, useState } from 'react'
import { loadPlayer } from './rtsp-lib.js'


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
      audio: false
    })


    // console.log('UseEffect -- ', playerCamRef)
    // if (!playerCamRef || !playerCamRef.current) {console.log('No ref');return}
    // loadPlayer({
    //   url: src,
    //   canvas: playerCamRef.current,
    //   audio: false,
    //   onDisconnect: (lp) => {
    //     if(!!lp && typeof lp.destroy == 'function') {
    //       console.log('Destroying player -- ', lp)
    //       lp.destroy()
    //       setPlayerCamRef(React.createRef())
    //     }
    //   }
    // })

  }, [playerCamRef, src])

  return <canvas ref={playerCamRef} {...props} className="camera" />
}

export default memo(RTSP)