# OpenAmp

yocto-workdir/layers/meta-ti/meta-ti-bsp/recipes-bsp/zephyr-fw/zephyr-openamp-rsc-table-fw.bb

yocto-workdir/layers/meta-arm/meta-arm-bsp/recipes-bsp/trusted-firmware-m/files/corstone1000/0008-Platform-CS1000-Replace-OpenAMP-with-RSE_COMMS.patch


https://corstone1000.docs.arm.com/en/latest/change-log.html
  
  * RSE Comms replaces OpenAMP

* Runtime Security Engine (RSE)
* https://docs.kernel.org/staging/remoteproc.html
* https://wiki.st.com/stm32mpu/wiki/Linux_RPMsg_framework_overview

For this build, the i.MX8MP OpenAMP and RPMsg support comes from the kernel and BSP metadata, not from a separate OpenAMP layer.


The board selects the Toradex NXP kernel in verdin-imx8mp.conf.
That machine file sets PREFERRED_PROVIDER_virtual/kernel to linux-toradex and lists the Verdin DTBs that are built. It does not select a dedicated rpmsg DTB variant the way some NXP EVK machine files do.

The Linux-side remoteproc and RPMsg features are enabled by the built kernel metadata in base.cfg:545.
That fragment enables:

```C
CONFIG_REMOTEPROC
CONFIG_REMOTEPROC_CDEV
CONFIG_IMX_REMOTEPROC
CONFIG_RPMSG_VIRTIO
CONFIG_RPMSG_CHAR
CONFIG_RPMSG_CTRL
```

The actual i.MX remoteproc driver is in imx_rproc.c:1339.
That file explicitly matches the compatible string fsl,imx8mp-cm7, so this is the kernel driver that backs the M7 remoteproc path on i.MX8MP.

The RPMsg implementation files are in the kernel source here:

```
imx_rpmsg.c
imx_rpmsg_tty.c
rpmsg_char.c
rpmsg_ctrl.c
```

The build produced the corresponding kernel module packages under the linux-toradex workdir, for example:
```
kernel-module-rpmsg-char-6.6.129-7.6.0-devel
kernel-module-rpmsg-ctrl-6.6.129-7.6.0-devel
kernel-module-imx-rpmsg-tty-6.6.129-7.6.0-devel
```

The NXP-style device-tree pattern for enabling M7/RPMsg is visible in the EVK DTS files, for example imx8mp-evk-rpmsg.dts:1.
That file adds the reserved memory, vrings, resource table, and the imx8mp-cm7 node that the imx_rproc driver binds to.