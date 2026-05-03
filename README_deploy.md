# Deploy and start

## Precondition

These binaries have been build according to [README_environment.md](README_environment.md):

* Yoco binary

  * yocto-workdir/build-torizon/deploy/images/verdin-imx8mm/torizon-docker-verdin-imx8mm.ota.tar.zst
  * yocto-workdir/build-torizon/deploy/images/verdin-imx8mm/torizon-docker-verdin-imx8mm-Tezi.tar

* Zephyr binary

  * $(pwd)/zephyr/zephyrproject/zephyr/build/zephyr/zephyr.elf

## Deploy both binaries and start the blinky app

### Step 1: Flash Torizon OS to the Verdin iMX8M Mini

Use the [Toradex Easy Installer (Tezi)](https://developer.toradex.com/software/toradex-easy-installer) to flash the Yocto image.

1. Put the module into recovery / Tezi mode (hold the recovery button on the carrier board while powering on, or follow the [Tezi boot mode guide](https://developer.toradex.com/software/toradex-easy-installer/tezi-easy-installer-boot-modes)).
2. On the host, start the Tezi USB/network interface so the installer can see the module:

   ```bash
   # Tezi runs on the module itself; connect via USB-to-serial or ethernet.
   # The image tarball to load is:
   ls yocto-workdir/build-torizon/deploy/images/verdin-imx8mm/torizon-docker-verdin-imx8mm-Tezi.tar
   ```

3. In the Tezi web UI (`http://<module-ip>:8080`) or USB mass-storage mode, select and install:

   ```
   yocto-workdir/build-torizon/deploy/images/verdin-imx8mm/torizon-docker-verdin-imx8mm-Tezi.tar
   ```

4. After flashing completes, reboot the module and confirm Linux boots:

   ```bash
   ssh torizon@<module-ip>
   uname -a
   systemctl is-system-running
   ```

### Step 2: Copy the Zephyr ELF to the target

From the host, copy the Cortex-M4 firmware to the Linux filesystem on the module:

```bash
scp zephyr/zephyrproject/zephyr/build/zephyr/zephyr.elf \
    torizon@<module-ip>:/lib/firmware/zephyr.elf
```

### Step 3: Load and start Zephyr on the Cortex-M4 via remoteproc

On the target (over SSH):

```bash
# Bind the remoteproc driver for the iMX8MM M4 core
echo -n "imx-rproc" | sudo tee /sys/bus/platform/drivers/imx-rproc/bind 2>/dev/null || true

# Point remoteproc at the firmware
echo zephyr.elf | sudo tee /sys/class/remoteproc/remoteproc0/firmware

# Start the Cortex-M4 core
echo start | sudo tee /sys/class/remoteproc/remoteproc0/state
```

Confirm it is running:

```bash
cat /sys/class/remoteproc/remoteproc0/state   # expected: running
dmesg | tail -20                               # look for "remoteproc0: remote processor imx-rproc is now up"
```

### Step 4: Verify the blinky demo

The Zephyr blinky sample toggles an LED on the Verdin module. Observe the LED physically, or confirm the firmware is alive via the remoteproc state:

```bash
cat /sys/class/remoteproc/remoteproc0/state   # must stay "running"
```

### Step 5: Stop the Cortex-M4 core (when done)

```bash
echo stop | sudo tee /sys/class/remoteproc/remoteproc0/state
```
