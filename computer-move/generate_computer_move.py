import asyncio
import argparse
import nats
from nats.errors import NoServersError, TimeoutError
import logging
import random

rps_moves = {0: "rock", 1: "paper", 2: "scissors"}


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

    async def generate_move(msg):
        generated_computer_move = rps_moves[random.randint(0, 2)].encode("utf-8")
        await msg.respond(generated_computer_move)
        await nc.publish("computer_move", generated_computer_move)

    if nc.is_connected:
        sub = await nc.subscribe("get_computer_move", cb=generate_move)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("nats_server_url")
    args = vars(parser.parse_args())

    loop = asyncio.get_event_loop()
    loop.run_until_complete(main(args["nats_server_url"], loop))
    loop.run_forever()
    loop.close()
