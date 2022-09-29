import { useEffect, useState } from 'react'

import Player from './components/Player.js'
import TimedDialog from './components/Dialog.js'
import EventLog from './components/EventLog.js'
import HLS from './components/HLS.js'
import RobotPlay from './components/RobotPlay.js'
import './App.css'

import EventTranslator from './EventTranslator.js'


function App() { 
    const [logs, setLog] = useState([])
    const [score, setScore] = useState({robot: 0, human: 0})
    const [message, setMessage] = useState({duration:0, text:'Wave to start a count down then play against the computer!'})
    const [gameState, setGameState] = useState('WAITING_TO_START')
    const [localVideoStreamURL, setLocalVideoStreamURL] = useState(window.location.protocol+'//'+window.location.hostname+'/rps/master.m3u8')
    const [robotPlay, setRobotPlay] = useState('Nothing yet')


    // Set up the eventing system and state machine
    useEffect(async ()=>{
        console.log('Setting up event stream')

        // try {
        //     const sock = await connect({servers:[window.location.host]})
        //     console.log(sock)
        // } catch (err){
        //     console.error(err)
        // }

        const socket = io("/events");
        const fsm = new EventTranslator(socket, {score, gameState, setLocalVideoStreamURL})

        fsm.on('score', (msg)=>{setScore(msg)})
        fsm.on('robotPlay', (msg)=>{setRobotPlay(msg)})
        fsm.on('gameState', (msg)=>{setGameState(msg)})
        fsm.on('systemState', (msg)=>{setSystemState(msg)})
        fsm.on('prompt', (msg)=>{setMessage(msg)})

        fsm.on('log', (line)=>{
            setLog((previousLogs)=>[...previousLogs, line])
        })

        fsm.init()
        
    },[])

    return (
        <div className="app">
            <header>
                <h1>Can you beat a robot in Rock Paper Scissors?</h1>
                <h2>Wave to start a count down then play against the computer!</h2>
            </header>

            <div class="center">
                <Player name="Human" headerColor="#2453ff" score={score.human}>
                    <HLS src={localVideoStreamURL} />
                </Player>

                <div className='vs'>VS</div>
                
                <Player name="Robot" headerColor="#fe7c3f" score={score.robot}>
                    <RobotPlay move={robotPlay} />
                </Player>
            </div>
            <EventLog logs={logs}/>

            <TimedDialog duration={message.duration} message={message.text}/>
        </div>
    )
}


export default App