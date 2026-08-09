#!/usr/bin/env python3

import argparse
import fcntl
import os
from pathlib import Path
import struct
import sys
import time


RPMSG_NAME_SIZE = 32
RPMSG_ADDR_ANY = 0xFFFFFFFF
RPMSG_CTRL_CLASS = Path("/sys/class/rpmsg")
RPMSG_BUS_DEVICES = Path("/sys/bus/rpmsg/devices")


def _ioc(direction, ioctl_type, number, size):
    return (direction << 30) | (size << 16) | (ioctl_type << 8) | number


RPMSG_ENDPOINT_INFO = struct.Struct("32sII")
RPMSG_CREATE_EPT_IOCTL = _ioc(1, 0xB5, 1, RPMSG_ENDPOINT_INFO.size)


def read_int(path):
    return int(path.read_text().strip(), 0)


def wait_for_channel(name, timeout):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        for device in RPMSG_BUS_DEVICES.glob("*"):
            try:
                if (device / "name").read_text().strip() == name:
                    return device, read_int(device / "dst")
            except (FileNotFoundError, ValueError):
                continue
        time.sleep(0.1)
    raise TimeoutError(f"RPMsg channel {name!r} did not appear within {timeout:g} seconds")


def find_control_device(requested):
    if requested is not None:
        path = Path(requested)
        if not path.exists():
            raise FileNotFoundError(path)
        return path

    controls = sorted(Path("/dev").glob("rpmsg_ctrl*"))
    if len(controls) != 1:
        raise RuntimeError(
            f"expected one /dev/rpmsg_ctrl* device, found {len(controls)}; "
            "select one with --control"
        )
    return controls[0]


def endpoint_class_devices():
    return {
        path.name
        for path in RPMSG_CTRL_CLASS.glob("rpmsg*")
        if not path.name.startswith("rpmsg_ctrl")
    }


def wait_for_endpoint(previous, timeout):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        created = endpoint_class_devices() - previous
        if len(created) == 1:
            device = Path("/dev") / created.pop()
            if device.exists():
                return device
        elif len(created) > 1:
            raise RuntimeError(f"multiple RPMsg endpoints appeared: {sorted(created)}")
        time.sleep(0.05)
    raise TimeoutError("RPMsg endpoint character device was not created")


def run_client(channel_name, control_name, timeout):
    channel, destination = wait_for_channel(channel_name, timeout)
    control = find_control_device(control_name)
    previous_endpoints = endpoint_class_devices()
    endpoint_name = b"zephyr-rpmsg-service"
    endpoint_info = RPMSG_ENDPOINT_INFO.pack(
        endpoint_name.ljust(RPMSG_NAME_SIZE, b"\0"), RPMSG_ADDR_ANY, destination
    )

    print(f"Channel {channel.name}: destination 0x{destination:x}")
    print(f"Creating endpoint through {control}")
    with control.open("rb+", buffering=0) as control_file:
        fcntl.ioctl(control_file.fileno(), RPMSG_CREATE_EPT_IOCTL, endpoint_info)
        endpoint = wait_for_endpoint(previous_endpoints, timeout)
        print(f"Exchanging counters through {endpoint}")

        with endpoint.open("rb+", buffering=0) as endpoint_file:
            while True:
                payload = endpoint_file.read(512)
                if not payload:
                    raise RuntimeError("RPMsg endpoint closed")
                if len(payload) != 4:
                    raise RuntimeError(
                        f"expected a 4-byte counter from Zephyr, received {len(payload)} bytes"
                    )
                counter = struct.unpack("<I", payload)[0]
                print(f"received {counter}, replying {counter}")
                endpoint_file.write(payload)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Echo counters for Zephyr's samples/subsys/ipc/rpmsg_service endpoint."
    )
    parser.add_argument("--channel", default="demo", help="RPMsg channel name (default: demo)")
    parser.add_argument("--control", help="RPMsg control device (default: auto-detect)")
    parser.add_argument(
        "--timeout", type=float, default=10.0, help="discovery timeout in seconds (default: 10)"
    )
    return parser.parse_args()


def main():
    args = parse_args()
    try:
        run_client(args.channel, args.control, args.timeout)
    except KeyboardInterrupt:
        print("\nStopped", file=sys.stderr)
        return 130
    except (OSError, RuntimeError, TimeoutError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())