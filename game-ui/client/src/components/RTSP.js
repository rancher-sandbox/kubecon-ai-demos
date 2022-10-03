import React, { useEffect } from 'react'

import { loadPlayer } from 'rtsp-relay/browser'

function RTSP({
    playerRef = React.createRef(),
    src,
    ...props
}) {

    useEffect(() => {
      loadPlayer({
        url: `ws://${location.host}/stream`,
        canvas: playerRef.current,
      
        // optional
        onDisconnect: () => console.log('Connection lost!'),
      })

    }, [playerRef, src]);

    return <canvas ref={playerRef} {...props} />
        
}

export default RTSP