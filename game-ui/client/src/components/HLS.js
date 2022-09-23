import React, { useEffect } from 'react'
import Hls from 'hls.js'

// Adapted from https://github.com/devcshort/react-hls/blob/master/src/index.tsx

const hlsConfig = {
    maxLoadingDelay: 4,
    minAutoBitrate: 0,
    lowLatencyMode: true,
    enableWorker: false
}
const autoPlay = true

function HLS({
    playerRef = React.createRef(),
    src,
    ...props
}) {
    let hls = null

    useEffect(() => {
        function _initPlayer() {
          if (hls != null) {
            hls.destroy();
          }
    
          const newHls = new Hls(hlsConfig);
    
          if (playerRef.current != null) {
            newHls.attachMedia(playerRef.current);
          }
    
          newHls.on(Hls.Events.MEDIA_ATTACHED, () => {
            newHls.loadSource(src);
    
            newHls.on(Hls.Events.MANIFEST_PARSED, () => {
              if (autoPlay) {
                playerRef?.current
                  ?.play()
                  .catch(() =>
                    console.log(
                      'Unable to autoplay prior to user interaction with the dom.'
                    )
                  );
              }
            });
          });
    
          newHls.on(Hls.Events.ERROR, function (event, data) {
            if (data.fatal) {
              switch (data.type) {
                case Hls.ErrorTypes.NETWORK_ERROR:
                  newHls.startLoad();
                  break;
                case Hls.ErrorTypes.MEDIA_ERROR:
                  newHls.recoverMediaError();
                  break;
                default:
                  _initPlayer();
                  break;
              }
            }
          });
    
          hls = newHls
        }
    
        _initPlayer()
    
        return () => {
          if (hls != null) {
            hls.destroy();
          }
        }

    }, [playerRef, src]);

    return <video ref={playerRef} {...props} />
        
}

export default HLS