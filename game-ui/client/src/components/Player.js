
function PlayerFrame(props) { 

    return (
        <div className={`player-frame ${props.name}`}>
            <header style={{backgroundColor: props.headerColor}} className="player-header">
                <span className="move">Current Move: {props.move || 'N/A'}</span>
                <span className="detected">Current Detection: {props.detection || 'N/A'}</span>
                <span className="name">{props.name}</span>
                <span className="score">{props.score}</span>
            </header>
            {props.children}
        </div>
    )
}

export default PlayerFrame