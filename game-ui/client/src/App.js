import { useEffect, useState } from 'react'

import Player from './components/Player'
import TimedDialog from './components/Dialog'
import EventLog from './components/EventLog'
import './App.css'

try {
    const socket = io("/admin");
} catch(err){
    console.log(err)
}


function App() { 
    const [human_score, setHumanScore] = useState('-')
    const [robot_score, setRobotScore] = useState('-')

    const [gameState, setGameState] = useState('WAITING_TO_START')


    const [logs, setLog] = useState([])
    console.log('logs', logs)

    const appendLog = (line)=>{
        console.log('oldLogs:', logs)
        setLog((previousLogs)=>[...previousLogs, line])
        console.log('newLogs:', logs)
    }

    useEffect(()=>{
        console.log('setting up event stream')

        const socket = io("/events");
        socket.on('connect',()=>{
            console.log('event stream connected')
            appendLog('CONNECTED')
        })
        socket.on('disconnect',()=>{
            console.log('event stream disconnected')
            appendLog('DISCONNECTED')
        })

        socket.on('msg', (msg)=>{
            console.log('MSG', msg)
            appendLog(msg)
        })
    },[])



    return (
        <div className="app">
            <header>
                <h1>Can you beat a robot in Rock Paper Scissors?</h1>
                <h2>Wave to start a count down then play against the computer!</h2>
            </header>

            <Player name="Human" headerColor="red" score="0"></Player>
            <div className='vs'>VS</div>
            <Player name="Robot" headerColor="blue" score="0"></Player>
            <EventLog logs={logs}/>

            <TimedDialog duration="5000" message="blah"/>

            <div className='instructions'>Instructions go here</div>
        </div>
    )
}


export default App