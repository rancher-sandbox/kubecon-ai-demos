import asyncio
import argparse
import nats
from nats.errors import NoServersError, TimeoutError
import logging
import traceback
import random

rps_moves = {0: "rock", 1: "paper", 2: "scissors"}
winning_moves = {"ROCK": "PAPER", "PAPER": "scissors", "SCISSORS": "rock"}
last_human_move = asyncio.Queue(maxsize=5)


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

    lock = asyncio.Lock()

    async def generate_move(msg):
        data = msg.data.decode()
        if data == "always_win":
            print("always win response received")
            async with lock:
                temp_last_human_move_list = []
                qsize = last_human_move.qsize()
                while qsize > 0:
                    temp_last_human_move_list.append(last_human_move.get_nowait())
                    qsize = qsize - 1
                if len(set(temp_last_human_move_list)) == 1:
                    generated_computer_move = winning_moves[
                        temp_last_human_move_list[0]
                    ]
                else:
                    generated_computer_move = rps_moves[random.randint(0, 2)]
        else:
            generated_computer_move = rps_moves[random.randint(0, 2)]
        generated_computer_move = generated_computer_move.encode("utf-8")
        await msg.respond(generated_computer_move)
        await nc.publish("computer_move", generated_computer_move)

    async def get_human_move(msg):
        data = msg.data.decode()
        async with lock:
            if last_human_move.qsize() == 5:
                last_human_move.get_nowait()
            last_human_move.put_nowait(data)

    if nc.is_connected:
        sub = await nc.subscribe("get_computer_move", cb=generate_move)
        human_move_sub = await nc.subscribe("human_move", cb=get_human_move)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("nats_server_url")
    args = vars(parser.parse_args())

    loop = asyncio.get_event_loop()
    loop.run_until_complete(main(args["nats_server_url"], loop))
    loop.run_forever()
    loop.close()
