import asyncio
import argparse
import nats
from nats.errors import NoServersError, TimeoutError


async def main(nats_server_url, always_win):
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
    if always_win == "always_win":
        resp = await nc.request("get_computer_move", b'always_win')
    else:
        resp = await nc.request("get_computer_move")
    print("Response:", resp)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("nats_server_url")
    parser.add_argument("always_win")
    args = vars(parser.parse_args())
    asyncio.run(main(args["nats_server_url"], args["always_win"]))
