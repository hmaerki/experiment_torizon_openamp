# hmp_mcuxpresso-copilot

This directory is prepared for downloading and building an NXP RPMsg-Lite / MCUX-based sample for the i.MX 8M Plus M7 target.

## Directory layout

- sources/ : cloned source trees
  - sources/rpmsg-lite
  - sources/mcux-sdk
- build/ : build output
- downloads/ : download staging area
- docs/ : notes and build documentation
- scripts/ : helper scripts

## Bootstrap

Run:

```bash
cd hmp_mcuxpresso-copilot
bash run_0_bootstrap.sh
```

This will clone the public RPMsg-Lite and MCUX SDK repositories into the prepared structure.

## Run as root

```bash
cd hmp_mcuxpresso-copilot

sudo su
./run_3_start-root.sh
```

```text
[  323.668053] remoteproc remoteproc0: powering up imx-rproc
[  323.668308] remoteproc remoteproc0: Booting fw image rpmsg_demo_MIMX8ML8_cm7.elf, size 31956
[  324.185378] rproc-virtio rproc-virtio.2.auto: assigned reserved memory node vdevbuffer@55400000
[  324.295338] imx-rproc imx8mp-cm7: imx_rproc_kick: failed (0, err:-62)
[  324.301804] virtio_rpmsg_bus virtio0: rpmsg host is online
[  324.301890] rproc-virtio rproc-virtio.2.auto: registered virtio0 (type 7)
[  324.301902] remoteproc remoteproc0: remote processor imx-rproc is now up
```

```bash
ls  -1 /dev/re* /dev/rpmsg* /sys/bus/rpmsg/devices/
/dev/remoteproc0
/dev/rpmsg_ctrl0

/sys/bus/rpmsg/devices/:
virtio0.rpmsg_ctrl.0.0
virtio0.rpmsg_ns.53.53
```