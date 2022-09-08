

function PlayerFrame(props) { 

    return (
        <div className="player-frame" style={{backgroundColor: props.color}}>
            <header>
                <span className="name">{props.name}</span>
                <span className="score">{props.score}</span>
            </header>
            {props.children}
        </div>
    )
}

export default PlayerFrame