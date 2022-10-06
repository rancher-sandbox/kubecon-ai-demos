
function EventLog(props) {

    const logs = props.logs.map((line)=>((
        <li>{line}</li>
    )))

    return (
    <ul className="event-log">
        <header>Event Log</header>
        {logs}
    </ul>
    )
}

export default EventLog