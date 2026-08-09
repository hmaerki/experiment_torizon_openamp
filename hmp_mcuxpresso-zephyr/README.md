hmp_mcuxpresso-zephyr/sources/rpmsg-lite/zephyr/samples/rpmsglite_pingpong/remote/src/main.c

hmp_mcuxpresso-zephyr/prj_verdin_imx8mp.conf



```bash
cd hmp_mcuxpresso-zephyr

sudo su
./run_3_start-root.sh
```

```bash
dmsg --follow

[ 1775.515759] remoteproc remoteproc0: stopped remote processor imx-rproc
[ 1775.542033] remoteproc remoteproc0: powering up imx-rproc
[ 1775.544481] remoteproc remoteproc0: Booting fw image rpmsg_lite_sample.elf, size 1067036
[ 1776.058711] rproc-virtio rproc-virtio.2.auto: assigned reserved memory node vdevbuffer@55400000
[ 1776.062113] virtio_rpmsg_bus virtio0: rpmsg host is online
[ 1776.062212] rproc-virtio rproc-virtio.2.auto: registered virtio0 (type 7)
[ 1776.062221] remoteproc remoteproc0: remote processor imx-rproc is now up
```


```bash
tio --baudrate=115200 /dev/ttyUSB0

*** Booting Zephyr OS build v4.4.0-10840-geaa480916f44 ***
[00:00:00.004,000] <inf> rpmsg_client_sample: Starting Verdin iMX8MP OpenAMP remote
Starting Verdin iMX8MP OpenAMP remote!
[00:00:00.016,000] <inf> rpmsg_client_sample: manager(): platform_init
[00:00:00.023,000] <inf> rpmsg_client_sample: platform_init(): metal_init
[00:00:00.030,000] <inf> rpmsg_client_sample: platform_init(): metal_io_init
[00:00:00.038,000] <inf> rpmsg_client_sample: platform_init(): rsc_table_get
[00:00:00.045,000] <inf> rpmsg_client_sample: platform_init(): resource_table_size=88
[00:00:00.053,000] <inf> rpmsg_client_sample: 0x0000b678 resource-table header
[00:00:00.061,000] <inf> rpmsg_client_sample:   version=1, entries=1
[00:00:00.068,000] <inf> rpmsg_client_sample: 0x0000b688 resource-table offsets
[00:00:00.075,000] <inf> rpmsg_client_sample:   offset[0]=0x00000014
[00:00:00.082,000] <inf> rpmsg_client_sample: 0x0000b68c virtio device
[00:00:00.089,000] <inf> rpmsg_client_sample: 0x0000b6a4 virtio status: 0x00
[00:00:00.097,000] <inf> rpmsg_client_sample: 0x0000b6a8 vring0: da=0x55000000, notifyid=0
[00:00:00.105,000] <inf> rpmsg_client_sample: 0x0000b6bc vring1: da=0x55008000, notifyid=1
[00:00:00.114,000] <inf> rpmsg_client_sample: platform_init(): metal_io_init
[00:00:00.121,000] <inf> rpmsg_client_sample: platform_init(): device_is_ready
[00:00:00.129,000] <inf> rpmsg_client_sample: platform_init(): ipm_register_callback
[00:00:00.137,000] <inf> rpmsg_client_sample: platform_init(): ipm_set_enabled
[00:00:00.145,000] <inf> rpmsg_client_sample: platform_init(): ipm_set_enabled
[00:00:00.153,000] <inf> rpmsg_client_sample: manager(): create_rpmsg_device
[00:00:00.160,000] <inf> rpmsg_client_sample: create_rpmsg_device(): rproc_virtio_create_vdev
[00:00:00.169,000] <inf> rpmsg_client_sample: create_rpmsg_device(): rproc_virtio_wait_remote_ready
[00:00:00.515,000] <inf> rpmsg_client_sample: ipm_callback()
```

```bash
ls  -1 /dev/re* /dev/rpmsg* /sys/bus/rpmsg/devices/
/dev/remoteproc0
/dev/rpmsg_ctrl0

/sys/bus/rpmsg/devices/:
virtio0.rpmsg_ctrl.0.0
virtio0.rpmsg_ns.53.53
```

## Compare this binary against working TCM binary

```bash
export A=build/zephyr/rpmsg_lite_sample.elf
export B=/lib/firmware/imx8mp_m7_TCM_rpmsg_lite_pingpong_rtos_linux_remote.elf
readelf -SW $A | grep resource_table
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [31] .resource_table   PROGBITS        0000b680 00b798 000058 00  WA  0   0  8
readelf -SW $B | grep resource_table
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 2] .resource_table   PROGBITS        00000400 001400 000058 00   A  0   0  1

readelf -x .resource_table $A
Hex dump of section '.resource_table':
  0x0000b680 01000000 01000000 00000000 00000000 ................
  0x0000b690 14000000 03000000 07000000 00000000 ................
  0x0000b6a0 01000000 00000000 00000000 00020000 ................
  0x0000b6b0 ffffffff 10000000 08000000 01000000 ................
  0x0000b6c0 00000000 ffffffff 10000000 08000000 ................
  0x0000b6d0 00000000 00000000                   ........

readelf -x .resource_table $B
Hex dump of section '.resource_table':
  0x00000400 01000000 01000000 00000000 00000000 ................
  0x00000410 14000000 03000000 07000000 00000000 ................
  0x00000420 01000000 00000000 00000000 00020000 ................
  0x00000430 00000055 00100000 00010000 00000000 ...U............
  0x00000440 00000000 00800055 00100000 00010000 .......U........
  0x00000450 01000000 00000000                   ........

./decode_resource_table.py $A
./decode_resource_table.py $A

