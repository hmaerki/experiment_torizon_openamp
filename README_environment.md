# README_environment

Environment setup steps for Phase 1 in [agent.md](agent.md).

## Scope

This document covers:

- Host prerequisites (Linux workstation).
- Yocto build environment using Docker.
- FreeRTOS SDK/toolchain setup for R5F firmware builds (included in Torizon default configuration).
- Basic validation checks before starting artifact builds.

## 1) Host Prerequisites

Target host OS: Linux (Ubuntu 22.04 LTS recommended).

Install required host packages:

```bash
sudo apt update
sudo apt install -y \
  git curl wget unzip xz-utils file \
  build-essential cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler \
  python3 python3-pip python3-venv python3-setuptools python3-wheel \
  udev
```

Optional but recommended:

```bash
sudo apt install -y ripgrep jq
```

## 2) Install and Validate Docker

Install Docker Engine (Ubuntu):

```bash
sudo apt install -y ca-certificates gnupg
sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
sudo chmod a+r /etc/apt/keyrings/docker.gpg

echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

Allow running Docker without sudo:

```bash
sudo usermod -aG docker "$USER"
newgrp docker
```

Validation:

```bash
docker --version
docker run --rm hello-world
```

## 3) Prepare Torizon OS Yocto Workspace

Reference: [Build Torizon OS from Source](https://developer.toradex.com/knowledge-base/build-torizoncore)
Layer: https://github.com/torizon/meta-toradex-torizon
Manifest repo: `git://git.toradex.com/toradex-manifest.git`

Target hardware for this project:

- **Module**: Verdin AM62P → `MACHINE=verdin-am62p`
- **Distro**: `torizon` (downstream TI kernel, default for AM62P family)
- **Image**: `torizon-docker` (includes Docker engine, systemd, OTA)
- **Manifest branch**: `scarthgap-7.x.y`
- **Manifest file**: `torizon/default.xml`

### 3a) Install the `repo` tool

```bash
mkdir -p ~/bin
curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
chmod a+x ~/bin/repo
export PATH=~/bin:$PATH
# Persist:
echo 'export PATH=~/bin:$PATH' >> ~/.bashrc
```

### 3b) Create workspace and initialise manifest

```bash
mkdir -p ./yocto-workdir
cd ./yocto-workdir

git config --global user.email "email@example.com"
git config --global user.name "Your Name"

repo init \
  -u git://git.toradex.com/toradex-manifest.git \
  -b scarthgap-7.x.y \
  -m torizon/default.xml

repo sync --no-clone-bundle
```

> For reproducible builds use a tagged release branch instead of tracking `scarthgap-7.x.y`.

### 3c) Build using the official Torizon crops Docker container

Toradex provides a pre-configured build container (`torizon/crops`) that removes the need to manage Yocto host dependencies manually.

```bash
docker run --rm -it \
  --name=crops \
  -v "$(pwd)/yocto-workdir:/workdir/torizon" \
  --workdir=/workdir/torizon \
  -e MACHINE=verdin-am62p \
  -e IMAGE=torizon-docker \
  torizon/crops:scarthgap-7.x.y \
  startup-tdx.sh
```

The binaries may be found here:

```bash
yocto-workdir/build-torizon/deploy/images/verdin-am62p/torizon-docker-verdin-am62p-20260504064800.ota.tar.zst
240MByte

yocto-workdir/build-torizon/deploy/images/verdin-am62p/ti-dm/am62pxx/ipc_echo_testb_mcu1_0_release_strip.xer5f
268kByte
```

The .xer5f format is TI's ELF format for R5F cores. These are IPC echo test binaries — the FreeRTOS firmware used to demonstrate remoteproc loading and OpenAMP IPC between Linux and the R5F. The signed variant is for HS (High Security) devices.

Note that this is a pre-built IPC echo test demo, not a blinky. If you want a blinky on the R5F, that would need to be a separate build from TI's MCU+ SDK.

* Easy Installer image (most common “final binary”):
  
  * yocto-workdir/build-torizon/deploy/images/verdin-am62p/torizon-docker-verdin-am62p-Tezi.tar

* OTA package:
  
  * yocto-workdir/build-torizon/deploy/images/verdin-am62p/torizon-docker-verdin-am62p.ota.tar.zst

> **TI EULA**: The first run may prompt you to accept the TI BSP EULA.
> Accept it when prompted, or pre-accept by adding `ACCEPT_TI_EULA="1"` to `conf/local.conf` before building.

To open an interactive shell inside the container without starting a build (useful for inspecting the environment):

```bash
docker run --rm -it \
  --name=crops \
  -v "$(pwd)/yocto-workdir:/workdir/torizon" \
  --workdir=/workdir/torizon \
  torizon/crops:scarthgap-7.x.y \
  startup-tdx.sh
```

(Omit `-e IMAGE=...` to get a shell instead of starting a build.)

## 4) FreeRTOS Toolchain

FreeRTOS support for the R5F core is included in the Torizon default configuration — no separate SDK or toolchain installation is required. The FreeRTOS firmware is built as part of the Yocto/Torizon build in Phase 2.

No additional setup steps are needed beyond the Yocto workspace prepared in Section 3.

## 5) Toolchain and Build Sanity Checks

Run these checks before starting Phase 2 builds:

```bash
# Docker
docker --version
docker run --rm hello-world

# repo tool
repo version

# Yocto workspace
ls yocto-workdir/.repo
```

## 6) Phase 1 Completion Checklist

- [ ] Host dependencies installed.
- [ ] Docker installed and validated.
- [ ] User can run Docker commands without sudo.
- [ ] `repo` tool installed and on PATH.
- [ ] Torizon manifest repo initialised (`repo init`) and synced (`repo sync`).
- [ ] Sanity checks pass.

## Notes

- Keep Yocto builds in Docker for reproducibility.
- Keep Yocto downloads and sstate caches outside containers to reduce rebuild time.
- FreeRTOS firmware for R5F is built within the Torizon Yocto build system; no separate toolchain is needed.
