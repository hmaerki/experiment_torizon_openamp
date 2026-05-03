# README_environment

Environment setup steps for Phase 1 in [agent.md](agent.md).

## Scope

This document covers:

- Host prerequisites (Linux workstation).
- Yocto build environment using Docker.
- Zephyr SDK/toolchain setup for Cortex-M4 firmware builds.
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

- **Module**: Verdin iMX8M Mini → `MACHINE=verdin-imx8mm`
- **Distro**: `torizon` (downstream NXP kernel, default for iMX8 family)
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
  -e MACHINE=verdin-imx8mm \
  -e IMAGE=torizon-docker \
  torizon/crops:scarthgap-7.x.y \
  startup-tdx.sh
```

* Easy Installer image (most common “final binary”):
  
  * yocto-workdir/build-torizon/deploy/images/verdin-imx8mm/torizon-docker-verdin-imx8mm-Tezi.tar

* OTA package (222 MBytes):
  
  * yocto-workdir/build-torizon/deploy/images/verdin-imx8mm/torizon-docker-verdin-imx8mm.ota.tar.zst
  * yocto-workdir/build-torizon/deploy/images/verdin-imx8mm/torizon-docker-verdin-imx8mm-Tezi_7.6.0-devel-20260502202613+build.0.tar

> **NXP EULA**: The first run will prompt you to accept the NXP/Freescale EULA.
> This is required for the iMX8M Mini BSP packages. Accept it when prompted, or
> pre-accept by adding `ACCEPT_FSL_EULA="1"` to `conf/local.conf` before building.

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

## 4) Prepare Zephyr Toolchain and Workspace

Create and activate a Python virtual environment for Zephyr tooling:

```bash
mkdir -p "$(pwd)/zephyr"
cd "$(pwd)/zephyr"
uv venv
source .venv/bin/activate
uv pip install west
```

Initialize Zephyr workspace:

```bash
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v3.7.0 $(pwd)/zephyr/zephyrproject
cd $(pwd)/zephyr/zephyrproject
west update
west zephyr-export
uv pip install -r zephyr/scripts/requirements.txt
```

Install Zephyr SDK (pick one method):

- Package manager / distro package, or

  ```bash
  # Download and install Zephyr SDK 0.16.8
  cd ~
  wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64.tar.xz
  tar xf zephyr-sdk-0.16.8_linux-x86_64.tar.xz
  cd zephyr-sdk-0.16.8
  ./setup.sh -t arm-zephyr-eabi   # Cortex-M4 only; use -t all for all toolchains

  export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk-0.16.8"
  echo 'export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk-0.16.8"' >> ~/.bashrc
  ```

- Official Zephyr SDK installer from docs.

Set SDK variables (example):

```bash
export ZEPHYR_SDK_INSTALL_DIR="$HOME/toolchains/zephyr-sdk"
```

Persist environment (optional):

```bash
echo 'export ZEPHYR_SDK_INSTALL_DIR="$HOME/toolchains/zephyr-sdk"' >> ~/.bashrc
```

## 5) Toolchain and Build Sanity Checks

Run these checks before starting Phase 2 builds:

```bash
# Docker
 docker --version
 docker run --rm hello-world

# Zephyr tooling
source "$(pwd)/zephyr/.venv/bin/activate"
west --version
cmake --version
ninja --version
python3 --version
```

Optional quick Zephyr test build (replace board as needed):

```bash
cd "$(pwd)/zephyr/zephyrproject/zephyr"
west build -b reel_board samples/basic/blinky
```

```bash
ls -lh $(pwd)/zephyr/zephyrproject/zephyr/build/zephyr/zephyr.elf
753k
```

## 6) Phase 1 Completion Checklist

- [ ] Host dependencies installed.
- [ ] Docker installed and validated.
- [ ] User can run Docker commands without sudo.
- [ ] `repo` tool installed and on PATH.
- [ ] Torizon manifest repo initialised (`repo init`) and synced (`repo sync`).
- [ ] Zephyr workspace initialized with `west`.
- [ ] Zephyr Python requirements installed.
- [ ] Zephyr SDK configured.
- [ ] Sanity checks pass.

## Notes

- Keep Yocto builds in Docker for reproducibility.
- Keep Yocto downloads and sstate caches outside containers to reduce rebuild time.
- Keep Zephyr tooling inside a dedicated Python virtual environment.
