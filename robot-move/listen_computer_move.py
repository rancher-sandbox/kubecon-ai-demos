import asyncio
import argparse
import nats
from nats.errors import NoServersError, TimeoutError
from pyftdi.ftdi import Ftdi
from pyftdi.i2c import I2cController
from time import sleep


rps_moves = {0: "rock", 1: "paper", 2: "scissors"}
servo = {"pinky": 0x18, "ring": 0x18, "middle": 0x18, "pointer": 0x18, "thumb": 0x18, "wrist": 0x18, }
command = {"activate": 0x06, "run_count": 0x05}
cmd_val = {"one": b'\x01', "zero": b'\x00', "three": b'\x03'}
agent = None
cur = 0x00
fsw = 0x00
sleepy = .5
CONNECTED=False

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

    def connect():
        global agent
        global CONNECTED
        try:
            print("Connecting to ftdi device")
            i2c = I2cController()
            i2c.configure('ftdi://ftdi:232h:1/1')
            agent = i2c.get_port(0x47)
            print("Agent configured: ", type(agent))
            CONNECTED=True
        except Exception as e:
            print("Connection failed", e)
            CONNECTED=False

    def flip_switch():
            global cur
            global fsw
            cur = cmd_val["one"] if fsw == cmd_val["zero"] else cmd_val["zero"]
            print("Activate switch value: ", cur)
            return cur


    async def move_robot(msg):
        global agent
        global CONNECTED
        global fsw
        global sleepy

        print("Computer move registered: ", msg.data)
        if(not CONNECTED):
            try:
                connect()
            except Exception as e:
                print("Gesture changed but device not connected", e)
                if msg.reply:
                    await msg.respond(msg.reply, "disconnected")
                return # bail out 

        try:
            # Move the robot hand
            # Register indexes
            match msg.data:
                case b'rock':
                    print("configure and activate gesture rock ...")
                    agent.write_to(servo["pinky"], b'\x09\x60')
                    sleep(sleepy)
                    agent.write_to(servo["ring"], b'\x09\x60')
                    sleep(sleepy)
                    agent.write_to(servo["middle"], b'\x09\x60')
                    sleep(sleepy)
                    agent.write_to(servo["pointer"], b'\x09\x60')
                    sleep(sleepy)
                    agent.write_to(servo["thumb"], b'\x09\x60')
                    sleep(sleepy)
                    agent.write_to(command["run_count"], cmd_val["one"])
                    sleep(sleepy)
                    fsw = flip_switch()
                    agent.write_to(command["activate"], fsw)
                    print("rock processed.")

                case b'paper':
                    print("configure and activate gesture paper ...")
                    agent.write_to(servo["pinky"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(servo["ring"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(servo["middle"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(servo["pointer"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(servo["thumb"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(command["run_count"], cmd_val["one"])
                    sleep(sleepy)
                    fsw = flip_switch()
                    agent.write_to(command["activate"], fsw)
                    print("paper processed.")

                case b'scissors':
                    print("configure and activate gesture scissors ...")
                    agent.write_to(servo["pinky"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(servo["ring"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(servo["middle"], b'\x09\x60')
                    sleep(sleepy)
                    agent.write_to(servo["pointer"], b'\x09\x60')
                    sleep(sleepy)
                    agent.write_to(servo["thumb"], b'\x05\x64')
                    sleep(sleepy)
                    agent.write_to(command["run_count"], cmd_val["one"])
                    sleep(sleepy)
                    fsw = flip_switch()
                    agent.write_to(command["activate"], fsw)
                    print("scissors processed.")

                case _:
                    agent.write_to(0xFE, b'\xFF')
                    print("default case processed.")

        except Exception as e:
            print("Could not process gesture:", e)

        if msg.reply:
            await msg.respond(msg.reply, msg.data)
                        

    if nc.is_connected:
        sub = await nc.subscribe("computer_move", cb=move_robot)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("nats_server_url")
    args = vars(parser.parse_args())

    loop = asyncio.get_event_loop()
    loop.run_until_complete(main(args["nats_server_url"], loop))
    loop.run_forever()
    loop.close()
