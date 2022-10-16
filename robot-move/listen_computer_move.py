import asyncio
import argparse
from asyncio.windows_events import NULL
import nats
from nats.errors import NoServersError, TimeoutError
import logging
import random

rps_moves = {0: "rock", 1: "paper", 2: "scissors"}
servo = {"pinky": 0x18, "ring": 0x18, "middle": 0x18, "pointer": 0x18, "thumb": 0x18, "wrist": 0x18, }
command = {"activate": 0x06}
cmd_val = {"one": b'\x01', "zero": b'\x00'}
agent = None

async def main(nats_server_url, loop):
    async def disconnected_cb():
        print("Got disconnected!")

    async def reconnected_cb():
        print("Got reconnected to NATS...")

    async def error_cb(e):
        print(f"There was an error: {e}")

    async def closed_cb():
        print("Connection is closed")

    nc = await nats.connect(
        nats_server_url,
        error_cb=error_cb,
        reconnected_cb=reconnected_cb,
        disconnected_cb=disconnected_cb,
        closed_cb=closed_cb,
    )

    async def connect():
        try:
            print("Connecting to ftdi device")
            i2c = I2cController()
            await i2c.configure('ftdi://ftdi:232h:1/1')
            agent = i2c.get_port(0x47)
            CONNECTED=True
        except:
            print("Connection failed")
            CONNECTED=False

    async def flip_switch():
        cur = await agent.read_from(command["activate"])
        return cmd_val["one"] if cur == cmd_val["zero"] else cmd_val["zero"]

    async def move_robot(msg):
        if(not CONNECTED):
            try:
                await connect()
                print("Computer move registered: ", msg.data)
                # Move the robot hand
                # Register indexes
                match msg.data:
                    case "rock":
                        await agent.write_to(servo["pinky"], b'\x09\x60')
                        await agent.write_to(servo["ring"], b'\x09\x60')
                        await agent.write_to(servo["middle"], b'\x09\x60')
                        await agent.write_to(servo["pointer"], b'\x09\x60')
                        await agent.write_to(servo["thumb"], b'\x09\x60')
                        await agent.write_to(command["activate"], flip_switch() )

                    case "paper":
                        await agent.write_to(servo["pinky"], b'\x05\x64')
                        await agent.write_to(servo["ring"], b'\x05\x64')
                        await agent.write_to(servo["middle"], b'\x05\x64')
                        await agent.write_to(servo["pointer"], b'\x05\x64')
                        await agent.write_to(servo["thumb"], b'\x05\x64')
                        await agent.write_to(command["activate"], flip_switch() )

                    case "scissors":
                        await agent.write_to(0xFF, b'\xFF')
                        await agent.write_to(servo["pinky"], b'\x05\x64')
                        await agent.write_to(servo["ring"], b'\x05\x64')
                        await agent.write_to(servo["middle"], b'\x09\x60')
                        await agent.write_to(servo["pointer"], b'\x09\x60')
                        await agent.write_to(servo["thumb"], b'\x05\x64')
                        await agent.write_to(command["activate"], flip_switch() )

                    case _:
                        await agent.write_to(0xFF, b'\xFF')

                    if msg.reply:
                        await msg.respond(msg.reply, msg.data)
                        
            except:
                print("Gesture changed but device not connected")
                if msg.reply:
                        await msg.respond(msg.reply, "disconnected")


    if nc.is_connected:
        sub = await nc.subscribe("computer_move", cb=move_robot)


if __name__ == "__main__":

    CONNECTED=False

    parser = argparse.ArgumentParser()
    parser.add_argument("nats_server_url")
    args = vars(parser.parse_args())

    loop = asyncio.get_event_loop()
    loop.run_until_complete(main(args["nats_server_url"], loop))
    loop.run_forever()
    loop.close()
