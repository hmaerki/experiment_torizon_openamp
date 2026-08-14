#!/usr/bin/env python3

import argparse
import asyncio
import os
import pathlib
import sys
import time
import tty
import typing

from util_rpmsg import Rpmsg

CHANNEL: str = "rpmsg-client-sample-py"


async def forward_tty_to_stdout(device: pathlib.Path) -> None:
    print(f"{device}: waiting to open...")
    while True:
        try:
            file_descriptor = os.open(device, os.O_RDWR | os.O_NOCTTY)
            break
        except FileNotFoundError:
            await asyncio.sleep(0.1)
    print(f"{device}: ...open!")

    tty.setraw(file_descriptor)
    os.write(file_descriptor, b"LINUX: TTY_READY_RXYVXT")
    print("after: LINUX: TTY_READY")

    loop = asyncio.get_running_loop()
    finished = loop.create_future()

    def on_readable() -> None:
        try:
            data = os.read(file_descriptor, 4096)
            if not data:
                loop.remove_reader(file_descriptor)
                if not finished.done():
                    finished.set_result(None)
                return

            print(f"{data}")
            os.write(
                file_descriptor,
                data.replace(b"zephyr is sending", b"LINUX is sending"),
            )
        except OSError as error:
            loop.remove_reader(file_descriptor)
            if not finished.done():
                finished.set_exception(error)

    loop.add_reader(file_descriptor, on_readable)
    try:
        await finished
    finally:
        loop.remove_reader(file_descriptor)
        os.close(file_descriptor)


async def run_pingpong(
    channel_name: str,
    control_name: pathlib.Path | None,
    count: int,
    timeout: float,
) -> None:
    async with Rpmsg(channel_name, control_name, timeout) as rpmsg:
        assert rpmsg.channel is not None
        print(f"Channel {rpmsg.channel.name}")
        print(f"Created endpoint through {rpmsg.control}")
        print(f"Exchanging {count} messages through {rpmsg.endpoint}")

        tty_task = asyncio.create_task(
            forward_tty_to_stdout(pathlib.Path("/dev/ttyRPMSG30")),
            name="ttyRPMSG30-to-stdout",
        )

        received = 0
        await rpmsg.write(f"LINUX sending rpmsg {received}".encode())
        begin_s = time.monotonic()
        while received < count:
            payload = await rpmsg.read(512)
            if not payload:
                raise RuntimeError("RPMsg endpoint closed")
            received += 1
            print(f"incoming msg {received}: {payload!r}")
            if received < count:
                await rpmsg.write(f"LINUX sending rpmsg {received}".encode())

        duration_s = time.monotonic() - begin_s
        print(
            f"goodbye after {duration_s:0.1f}s! {1000.0 * duration_s / count:0.1f}ms per call."
        )
        await tty_task


def parse_args(argv: typing.Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=f"Userspace equivalent of Linux's '{CHANNEL}' driver."
    )
    parser.add_argument(
        "--channel",
        default=CHANNEL,
        help=f"RPMsg channel name (default: '{CHANNEL}')",
    )
    parser.add_argument(
        "--count",
        type=int,
        default=3,
        help="number of replies to receive (default: 100)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=10.0,
        help="discovery timeout in seconds (default: 10)",
    )
    return parser.parse_args(argv)


def main(argv: typing.Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        asyncio.run(
            run_pingpong(
                channel_name=args.channel,
                control_name=None,
                count=args.count,
                timeout=args.timeout,
            )
        )
    except KeyboardInterrupt:
        print("\nStopped", file=sys.stderr)
        return 130
    except (OSError, RuntimeError, TimeoutError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
