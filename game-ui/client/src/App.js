import { useEffect, useState } from 'react'

import Player from './components/Player'
import TimedDialog from './components/Dialog'
import './App.css'



function App() { 
    const [human_score, setHumanScore] = useState('-')
    const [robot_score, setRobotScore] = useState('-')

    const [gameState, setGameState] = useState('WAITING_TO_START')

    useEffect(()=>{
        
    })



    return (
        <div className="app">
            <header>
                <h1>Can you beat a robot in Rock Paper Scissors?</h1>
                <h2>Wave to start a count down then play against the computer!</h2>
            </header>

            <Player name="Human" headerColor="red" score="0"></Player>
            <div className='vs'>VS</div>
            <Player name="Robot" headerColor="blue" score="0"></Player>

            <TimedDialog duration="5000" message="blah"/>

            <div className='instructions'>Instructions go here</div>
        </div>
    )
}


export default App