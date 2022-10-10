const EventEmitter = require( 'events' )

const beats = (move1, move2) => (
    (move1 == 'paper' && move2 == 'rock') ||
    (move1 == 'rock' && move2 == 'scissors') ||
    (move1 == 'scissors' && move2 == 'paper')) 

// Translates raw events from MQTT/NATS into state events for App.js to propagate 
class EventTranslator extends EventEmitter {

    constructor(events) {
        super()
        this.events = events
        this.scores = {robot: 0, human: 0}
    }

    init(){
        this.events.on('msg', (msg)=>{
            const {topic, message} = JSON.parse(msg)
            const topicArr = topic.split(/[/.]/ig)

            this.onMessage(topicArr, message)
        })

        this.events.on('connect',()=>{
            this.emit('log', 'CONNECTED')
        })

        this.events.on('disconnect',()=>{
            this.emit('log', 'DISCONNECTED')
        })
    }

    onMessage(topicArr, message){
        console.log('MSG -- ', topicArr, ' -- ', message)
        this.emit('log', message)


        switch(topicArr[0]) {
            case 'game':
                this.onGameMessage(topicArr, message)
                break;
            case 'round':
                this.onRoundMessage(topicArr, message)
                break;
            case 'system':
                this.onSystemMessage(topicArr, message)
                break;
        }
    }


    // 'WAITING_TO_START'
    // 'GAME_STARTING' -- round total
    // 'GAME_END' -- winner, score, etc...
    onGameMessage(topicArr, message){
        console.log('Game message')
        switch(topicArr[1]) {
            case 'start':
                console.log('Start')
                this.sendPrompt('Game Starting', 1000)
                break;

            case 'end':
                console.log('End')
                const score = JSON.parse(message)
                const winlose = score.human > score.robot ? 'Won!': 'Lost :('
                this.sendPrompt(`You ${winlose} -- The final score was You:${score.human} Robot:${score.robot}`, 2000)
                break;
        }
    }

    // 'ROUND_STARTING' -- round #, round total
    // 'COUNTDOWN' -- number
    // 'ROUND_END' -- plays, winner, score
    onRoundMessage(topicArr, message){
        switch(topicArr[1]) {
            case 'start':
                this.sendPrompt('Round Starting', 1000)
                break;

            case 'countdown':
                this.sendPrompt(message, 750)
                break;

            case 'end':
                const {robotPlay, humanPlay} = JSON.parse(message)

                if (beats(humanPlay,robotPlay)) {
                    this.scores.human = this.scores.human + 1
                } else {
                    this.scores.robot = this.scores.robot + 1
                }

                this.emit('robotPlay', robotPlay)
                this.emit('score', this.scores)

                // TODO emit prompt of "blah beats blah"

                break;
        }
    }
    
    // 'HEALTHY'
    // 'INITIALIZING' -- [component initializing]
    // ''
    onSystemMessage(topicArr, message){
        switch(topicArr[1]) {
            case 'videoSrc':
                this.emit('videoSrc', message)
                break;
        }
    }

    sendPrompt(text, duration=0) {
        console.log('Sending Prompt')
        this.emit('prompt', {text, duration})
    }
}


export default EventTranslator