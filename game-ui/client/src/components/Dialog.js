import { useEffect, useState } from 'react'


function TimedDialog(props) {

    const [open, setOpen] = useState(true);
    const [lastTimeout, setLastTimeout] = useState();

    // Only set timeout if duration is set
    // This allows for long term messages like "Please start game by ..."
    useEffect(()=>{
        clearTimeout(lastTimeout)
        if (props.duration>0){
            const newTimeout = setTimeout(()=>{
                setOpen(false)
            }, props.duration)
            setLastTimeout(newTimeout)
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