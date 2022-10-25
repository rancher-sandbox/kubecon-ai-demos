const EventEmitter = require( 'events' )


const beats = (move1, move2) => (
    (move1 == 'PAPER' && move2 == 'ROCK') ||
    (move1 == 'ROCK' && move2 == 'SCISSORS') ||
    (move1 == 'SCISSORS' && move2 == 'PAPER')) 

// Translates raw events from MQTT/NATS into state events for App.js to propagate 
class EventTranslator extends EventEmitter {

    constructor(events, config={}) {
        super()
        this.events = events
        this.scores = {robot: 0, human: 0}
        this.PLAY_UNTIL = config.PLAY_UNTIL || 3
    }

    init(){
        this.events.on('msg', (msg)=>{
            const {topic, message} = JSON.parse(msg)
            const topicArr = topic.split(/[/.]/ig) // allow for both NATS and MQTT

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
            case 'detection':
                this.onDetectionMessage(message)
                break;
        }
    }


    // Unused in current revision
    onGameMessage(topicArr, message){
        console.log('Game message')
        switch(topicArr[1]) {
            case 'start':
                console.log('Start')
                this.sendPrompt('Game Starting', 1000)
                break;

            case 'end':
                console.log('Game End')
                const winlose = this.scores.human > this.scores.robot ? 'Won!': 'Lost :('
                this.sendPrompt(`You ${winlose} -- The final score was You:${score.human} Robot:${score.robot}`, 5000)
                this.resetScore()
                break;
        }
    }

    onRoundMessage(topicArr, message){
        switch(topicArr[1]) {
            case 'start':
                this.sendPrompt('Round Starting', 900)
                break;

            case 'countdown':
                this.onCountdown(message)
                break;

            case 'end':
                this.onRoundEnd(message)
                break;
        }
    }



    onDetectionMessage(message){
        this.emit('detection', message)
    }

    onCountdown(message) {
        this.sendPrompt(message, 750)
    }

    onRoundEnd(message) {
        const {robotPlay, humanPlay} = JSON.parse(message)

        if (humanPlay == robotPlay) { // Tie
            this.sendPrompt(`Tie`, 2000)
        } else if (beats(humanPlay,robotPlay)) { // Human Wins
            this.scores.human = this.scores.human + 1
            this.sendPrompt(`You win`, 2000)
        } else { // Robot Wins
            this.scores.robot = this.scores.robot + 1
            this.sendPrompt(`Robot Wins`, 2000)
        }

        this.emit('robotPlay', robotPlay)
        this.emit('score', this.scores)
    }

    sendPrompt(text, duration=0) {
        console.log('Sending Prompt')
        this.emit('prompt', {text, duration})
    }

    resetScore() {
        this.scores = {robot: 0, human: 0}
        this.emit('score', this.scores)
    }


    isGameOver() {
        if (!PLAY_UNTIL) return false
        return ((this.scores.human >=this.PLAY_UNTIL) || (this.scores.robot >= this.PLAY_UNTIL))
    }

}


export default EventTranslator