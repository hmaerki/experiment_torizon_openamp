hmp_mcuxpresso-zephyr/sources/rpmsg-lite/zephyr/samples/rpmsglite_pingpong/remote/src/main.c

hmp_mcuxpresso-zephyr/prj_verdin_imx8mp.conf



```bash
cd hmp_mcuxpresso-zephyr

sudo su
./run_3_start-root.sh
```

```bash
dmsg --follow

[12709.966234] remoteproc remoteproc0: stopped remote processor imx-rproc
[12709.999102] remoteproc remoteproc0: powering up imx-rproc
[12710.001452] remoteproc remoteproc0: Booting fw image rpmsg_lite_sample.elf, size 1067016
[12710.517312] rproc-virtio rproc-virtio.2.auto: assigned reserved memory node vdevbuffer@55400000
[12710.518570] virtio_rpmsg_bus virtio0: rpmsg host is online
[12710.518608] rproc-virtio rproc-virtio.2.auto: registered virtio0 (type 7)
[12710.518616] remoteproc remoteproc0: remote processor imx-rproc is now up
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
[00:00:00.053,000] <inf> rpmsg_client_sample: platform_init(): resource_table=0xb7a8
[00:00:00.061,000] <inf> rpmsg_client_sample: platform_init(): resource_table.hdr.ver=0x1
[00:00:00.070,000] <inf> rpmsg_client_sample: platform_init(): &resource_table.hdr.ver=0xb7a8
[00:00:00.079,000] <inf> rpmsg_client_sample: platform_init(): resource_table.hdr.num=0x1
[00:00:00.088,000] <inf> rpmsg_client_sample: platform_init(): &resource_table.hdr.num=0xb7ac
[00:00:00.097,000] <inf> rpmsg_client_sample: platform_init(): &resource_table.offset=0xb7b8
[00:00:00.105,000] <inf> rpmsg_client_sample: platform_init(): &resource_table.offset=0xb7b8
[00:00:00.114,000] <inf> rpmsg_client_sample: platform_init(): resource_table.offset[RSC_TABLE_NUM_ENTRY]=1
[00:00:00.124,000] <inf> rpmsg_client_sample: platform_init(): &resource_table.vdev=0xb7bc
[00:00:00.133,000] <inf> rpmsg_client_sample: platform_init(): &resource_table.vring0=0xb7d8
[00:00:00.142,000] <inf> rpmsg_client_sample: platform_init(): &resource_table.vring1=0xb7ec
[00:00:00.151,000] <inf> rpmsg_client_sample: platform_init(): metal_io_init
[00:00:00.158,000] <inf> rpmsg_client_sample: platform_init(): device_is_ready
[00:00:00.166,000] <inf> rpmsg_client_sample: platform_init(): ipm_register_callback
[00:00:00.174,000] <inf> rpmsg_client_sample: platform_init(): ipm_set_enabled
[00:00:00.182,000] <inf> rpmsg_client_sample: platform_init(): ipm_set_enabled
[00:00:00.189,000] <inf> rpmsg_client_sample: manager(): create_rpmsg_device
[00:00:00.197,000] <inf> rpmsg_client_sample: create_rpmsg_device(): rproc_virtio_create_vdev
[00:00:00.206,000] <inf> rpmsg_client_sample: create_rpmsg_device(): rproc_virtio_wait_remote_ready
[00:00:00.520,000] <inf> rpmsg_client_sample: ipm_callback()
*** Booting Zephyr OS build v4.4.0-10840-geaa480916f44 ***
[00:00:00.004,000] <inf> rpmsg_client_sample: Starting Verdin iMX8MP OpenAMP remote
Starting Verdin iMX8MP OpenAMP remote!
[00:00:00.016,000] <inf> rpmsg_client_sample: manager(): platform_init
[00:00:00.023,000] <inf> rpmsg_client_sample: platform_init(): metal_init
[00:00:00.030,000] <inf> rpmsg_client_sample: platform_init(): metal_io_init
[00:00:00.038,000] <inf> rpmsg_client_sample: platform_init(): rsc_table_get
[00:00:00.045,000] <inf> rpmsg_client_sample: platform_init(): resource_table_size=88
[00:00:00.053,000] <inf> rpmsg_client_sample: 0x0000b680 resource-table header
[00:00:00.061,000] <inf> rpmsg_client_sample:   version=1, entries=1
[00:00:00.068,000] <inf> rpmsg_client_sample: 0x0000b690 resource-table offsets
[00:00:00.075,000] <inf> rpmsg_client_sample:   offset[0]=0x00000014
[00:00:00.082,000] <inf> rpmsg_client_sample: 0x0000b694 virtio device
[00:00:00.089,000] <inf> rpmsg_client_sample: 0x0000b6ac virtio status: 0x00
[00:00:00.097,000] <inf> rpmsg_client_sample: 0x0000b6b0 vring0: da=0xffffffff, notifyid=1
[00:00:00.105,000] <inf> rpmsg_client_sample: 0x0000b6c4 vring1: da=0xffffffff, notifyid=0
[00:00:00.114,000] <inf> rpmsg_client_sample: platform_init(): metal_io_init
[00:00:00.121,000] <inf> rpmsg_client_sample: platform_init(): device_is_ready
[00:00:00.129,000] <inf> rpmsg_client_sample: platform_init(): ipm_register_callback
[00:00:00.137,000] <inf> rpmsg_client_sample: platform_init(): ipm_set_enabled
[00:00:00.145,000] <inf> rpmsg_client_sample: platform_init(): ipm_set_enabled
[00:00:00.153,000] <inf> rpmsg_client_sample: manager(): create_rpmsg_device
[00:00:00.160,000] <inf> rpmsg_client_sample: create_rpmsg_device(): rproc_virtio_create_vdev
[00:00:00.169,000] <inf> rpmsg_client_sample: create_rpmsg_device(): rproc_virtio_wait_remote_ready
[00:00:00.516,000] <inf> rpmsg_client_sample: ipm_callback()
```

```bash
ls  -1 /dev/re* /dev/rpmsg* /sys/bus/rpmsg/devices/
/dev/remoteproc0
/dev/rpmsg_ctrl0

/sys/bus/rpmsg/devices/:
virtio0.rpmsg_ctrl.0.0
virtio0.rpmsg_ns.53.53
```
