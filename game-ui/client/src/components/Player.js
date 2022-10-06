
function PlayerFrame(props) { 

    return (
        <div className={`player-frame ${props.name}`}>
            <header style={{backgroundColor: props.headerColor}}>
                <span className="name">{props.name}</span>
                <span className="score">{props.score}</span>
            </header>
            {props.children}
        </div>
    )
}

export default PlayerFrame