import asyncio

import nats
from nats.errors import NoServersError, TimeoutError


async def main():
    async def disconnected_cb():
        print("Got disconnected!")

    async def reconnected_cb():
        print("Got reconnected to NATS...")

    async def error_cb(e):
        print(f"There was an error: {e}")

    async def closed_cb():
        print("Connection is closed")

    nc = await nats.connect(
        "nats://localhost:4222",
        error_cb=error_cb,
        reconnected_cb=reconnected_cb,
        disconnected_cb=disconnected_cb,
        closed_cb=closed_cb,
    )

    resp = await nc.request("get_computer_move")
    print("Response:", resp)


if __name__ == "__main__":
    asyncio.run(main())
