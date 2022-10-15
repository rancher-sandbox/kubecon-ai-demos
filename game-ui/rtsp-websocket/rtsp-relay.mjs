import { path } from '@ffmpeg-installer/ffmpeg'
import { spawn } from 'child_process'
import ews from 'express-ws'
import ps from 'ps-node'

const ffmpegPath = path

class InboundStreamWrapper {
  constructor() {
    this.clients = 0;
  }

  start({ url, additionalFlags = [], transport, windowsHide = true }) {
    if (this.verbose) console.log('[rtsp-relay] Creating brand new stream');

    // validate config
    const txpConfigInvalid = additionalFlags.indexOf('-rtsp_transport');
    // eslint-disable-next-line no-bitwise
    if (~txpConfigInvalid) {
      const val = additionalFlags[0o1 + txpConfigInvalid];
      console.warn(
        `[rtsp-relay] (!) Do not specify -rtsp_transport=${val} in additionalFlags, use the option \`transport: '${val}'\``,
      );
    }

    this.stream = spawn(
      ffmpegPath,
      [
        ...(transport ? ['-rtsp_transport', transport] : []), // this must come before `-i [url]`, see #82
        '-stimeout',
        '3000000',
        '-i',
        url,
        '-f', // force format
        'mpegts',
        '-codec:v', // specify video codec (MPEG1 required for jsmpeg)
        'mpeg1video',
        '-r',
        '30', // 30 fps. any lower and the client can't decode it
        ...additionalFlags,
        '-',
      ],
      { detached: false, windowsHide },
    );
    this.stream.stderr.on('data', () => {});
    this.stream.stderr.on('error', (e) => console.log('err:error', e));
    this.stream.stdout.on('error', (e) => console.log('out:error', e));
    this.stream.on('error', (err) => {
      if (this.verbose) {
        console.warn(`[rtsp-relay] Internal Error: ${err.message}`);
      }
    });

    this.stream.on('exit', (_code, signal) => {
      if (signal !== 'SIGTERM') {
        if (this.verbose) {
          console.warn(
            '[rtsp-relay] Stream died',
          );
        }
        this.stream = null;
      }
    });
  }

  get(options) {
    this.verbose = options.verbose;
    this.clients += 1;
    if (!this.stream) this.start(options);
    return this.stream;
  }

  decrement() {
    this.clients -= 1;
    return this.clients;
  }

  kill(clientsLeft) {
    if (!this.stream) return; // the stream is currently dead
    if (!clientsLeft) {
      if (this.verbose) {
        console.log('[rtsp-relay] no clients left; destroying stream');
      }
      this.stream.kill('SIGTERM');
      this.stream = null;
      // next time it is requested it will be recreated
      return;
    }

    if (this.verbose) {
      console.log(
        '[rtsp-relay] there are still some clients so not destroying stream',
      );
    }
  }
}

let wsInstance;
const rtspRelay = (app, server) => {
  if (!wsInstance) wsInstance = ews(app, server);
  const wsServer = wsInstance.getWss();

  const Inbound = {};

  return {

    killAll() {
      ps.lookup({ command: 'ffmpeg' }, (err, list) => {
        if (err) throw err;
        list
          .filter((p) => p.arguments.includes('mpeg1video'))
          .forEach(({ pid }) => ps.kill(pid));
      });
    },

    proxy({ url, verbose, ...options }) {
      if (!url) throw new Error('URL to rtsp stream is required');

      // TODO: node15 use ||=
      if (!Inbound[url]) Inbound[url] = new InboundStreamWrapper();

      async function handler(ws) {


        // these should be detected from the source stream
        const [width, height] = [0x0, 0x0];

        const streamHeader = Buffer.alloc(0x8);
        streamHeader.write('jsmp');
        streamHeader.writeUInt16BE(width, 0x4);
        streamHeader.writeUInt16BE(height, 0x6);
        ws.send(streamHeader, { binary: true });
        if (verbose) console.log('[rtsp-relay] New WebSocket Connection');


        ws.on('close', () => {
            const c = Inbound[url].decrement();
            if (verbose) {
                console.log(`[rtsp-relay] WebSocket Disconnected ${c} left`);
            }
            Inbound[url].kill(c);
        });

        while(ws.OPEN) {
            await new Promise((resolve)=>{
                console.warn('[rtsp-relay] Starting Stream')
                const streamIn = Inbound[url].get({ url, verbose, ...options })

                streamIn.stdout.on('data', (chunk)=>{ws.send(chunk)})

                streamIn.on('exit',()=>{
                    console.warn('[rtsp-relay] Restarting Stream in 3 seconds')
        
                    setTimeout(()=>{
                        resolve()
                    })
                })
            })
        }
      }
      return handler;
    }
  }
}

export default rtspRelay