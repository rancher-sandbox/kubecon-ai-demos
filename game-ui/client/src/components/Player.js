

function PlayerFrame(props) { 

    return (
        <div className="player-frame">
            <header style={{backgroundColor: props.headerColor}}>
                <span className="name">{props.name}</span>
                <span className="score">{props.score}</span>
            </header>
            {props.children}
        </div>
    )
}

export default PlayerFrame