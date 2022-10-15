

//TODO: change to be images of the last play

function RobotPlay(props) {

    if(!props.move) {
        return (
            <div className="robotDiv">
                Robot hasn't made a move yet!
            </div>
        )
    } else {
        const imageSrc = `/images/${props.move.toLowerCase()}.jpg`
    
        return (
            <div className="robotDiv">
                <img src={imageSrc} className="robotImg"/>
            </div>
        )

    }
}

export default RobotPlay