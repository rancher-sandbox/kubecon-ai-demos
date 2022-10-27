import { useEffect, useState } from 'react'
import './App.css'

import TimedDialog from './components/Dialog.js'
import EventLog from './components/EventLog.js'
import RTSP from './components/RTSP.js'

import Player from './components/Player.js'
import RobotPlay from './components/RobotPlay.js'


import EventTranslator from './EventTranslator.js'

let waitText = 'Show "rock", "paper", "scissors" in order to start!'

function App() { 
    const [logs, setLog] = useState([])
    const [config, setConfig] = useState({events: '/events', camera: '', showLog: true})
    const [score, setScore] = useState({robot: 0, human: 0})
    const [message, setMessage] = useState({duration:0, text: waitText})

    const [robotPlay, setRobotPlay] = useState('')
    const [humanPlay, setHumanPlay] = useState('')


    // Set up the eventing system and state machine
    useEffect(async ()=>{
        console.log('Setting up event stream')
        const response = await fetch('/config.json')
        const newConfig = await response.json()
        console.log(newConfig)

        setConfig((prevConfig)=>(Object.assign({},prevConfig,newConfig)))

        const socket = io(newConfig.events)
        const fsm = new EventTranslator(socket, newConfig)

        fsm.on('score', (msg)=>{setScore(msg)})
        fsm.on('robotPlay', (msg)=>{setRobotPlay(msg)})
        fsm.on('detection', (msg)=>{
            setHumanPlay(msg)
            if(!msg) {
                setMessage({duration:0, text: waitText})
            }
        })
        fsm.on('prompt', (msg)=>{setMessage(msg)})

        fsm.on('log', (line)=>{
            setLog((previousLogs)=>[...previousLogs, line])
        })

        fsm.init()
    },[])

    const Stream = (!!config.camera) ? (<RTSP src={config.camera} />):null;

    return (
        <div className="app">
            <header>
                <h1>Can you beat a robot in Rock Paper Scissors?</h1>
            </header>

            <div class="center">
                <Player name="human" headerColor="#2453ff" score={score.human} move={humanPlay.toLowerCase()}>
                    {Stream}
                </Player>

                <div className='vs'>VS</div>
                
                <Player name="robot" headerColor="#fe7c3f" score={score.robot} move={robotPlay.toLowerCase()}>
                    <RobotPlay move={robotPlay} />
                </Player>
            </div>
            <EventLog logs={logs} show={config.showLog}/>

            <TimedDialog duration={message.duration} message={message.text}/>
        </div>
    )
}


export default App