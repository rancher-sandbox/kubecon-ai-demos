import { useEffect, useState } from 'react'

import Player from './components/Player.js'
import TimedDialog from './components/Dialog.js'
import EventLog from './components/EventLog.js'
import './App.css'

import EventTranslator from './EventTranslator.js'

// TODO should be passed in from backend
const localVideoStreamURL = './cam.mp4'


function App() { 
    const [logs, setLog] = useState([])
    const [score, setScore] = useState({robot:'--', human:'--'})
    const [message, setMessage] = useState({duration:3000, text:'Hello'})
    const [gameState, setGameState] = useState('WAITING_TO_START')
    const [systemState, setSystemState] = useState('UNKNOWN')


    // Set up the eventing system and state machine
    useEffect(()=>{
        console.log('Setting up event stream')

        const socket = io("/events");
        const fsm = new EventTranslator(socket, {score, gameState, systemState})

        fsm.on('score', (msg)=>{setScore(msg)})
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
                    <video autoplay>
                        <source scr={localVideoStreamURL} type="video/mp4"/>
                    </video>
                </Player>


                <div className='vs'>VS</div>
                
                
                <Player name="Robot" headerColor="#fe7c3f" score={score.robot}>
                </Player>
            </div>
            <EventLog logs={logs}/>

            <TimedDialog duration={message.duration} message={message.text}/>
        </div>
    )
}


export default App