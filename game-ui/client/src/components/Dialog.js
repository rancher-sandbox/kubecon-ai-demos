import { useEffect, useState } from 'react'

function TimedDialog(props) {

    const [open, setOpen] = useState(true);

    // Only set timeout if duration is set
    // This allows for long term messages like "Please start game by ..."
    useEffect(()=>{
        if (props.duration>0){
            setTimeout(()=>{
                setOpen(false)
            }, props.duration)
        }
        setOpen(true)
    }, [props.message])

    return (
    <dialog open={open}>
        {props.message}
    </dialog>
    )
}

export default TimedDialog