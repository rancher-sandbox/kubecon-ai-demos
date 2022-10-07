import { useEffect, useState, memo } from 'react'
import './App.css'

import TimedDialog from './components/Dialog.js'
import EventLog from './components/EventLog.js'
import RTSP from './components/RTSP.js'

import Player from './components/Player.js'
import RobotPlay from './components/RobotPlay.js'


import EventTranslator from './EventTranslator.js'


function App() { 
    const [logs, setLog] = useState([])
    const [score, setScore] = useState({robot: 0, human: 0})
    const [message, setMessage] = useState({duration:0, text:'Wave to start a count down then play against the computer!'})
    const [gameState, setGameState] = useState('WAITING_TO_START')
    const [robotPlay, setRobotPlay] = useState('')

    // Set up the eventing system and state machine
    useEffect(async ()=>{
        console.log('Setting up event stream')


        const socket = io("/events");
        const fsm = new EventTranslator(socket)

        fsm.on('score', (msg)=>{setScore(msg)})
        fsm.on('robotPlay', (msg)=>{setRobotPlay(msg)})
        fsm.on('gameState', (msg)=>{setGameState(msg)})
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
                <Player name="human" headerColor="#2453ff" score={score.human}>
                    <RTSP src={`ws://${location.host}/stream`} />
                </Player>

                <div className='vs'>VS</div>
                
                <Player name="robot" headerColor="#fe7c3f" score={score.robot}>
                    <RobotPlay move={robotPlay} />
                </Player>
            </div>
            <EventLog logs={logs}/>

            <TimedDialog duration={message.duration} message={message.text}/>
        </div>
    )
}


export default App