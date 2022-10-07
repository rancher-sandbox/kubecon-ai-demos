
function EventLog(props) {

    return (
    <div className="event-log">
        <header>Event Log</header>
        <ul>
        {props.logs.map((line)=>((
            <li>{line}</li>
        )))}
        </ul>
    </div>
    )
}

export default EventLog