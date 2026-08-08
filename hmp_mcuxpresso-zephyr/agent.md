# Agent.md

## Context

On the Linux application core, the `rpmsg-client-sample` kernel
module is loaded:


```sh
modprobe rpmsg-client-sample
```

This is the source: hmp_mcuxpresso-zephyr/src/main.c / common.h.
The source was copied from https://github.com/torvalds/linux/blob/master/samples/rpmsg/rpmsg_client_sample.c / https://github.com/nxp-mcuxpresso/rpmsg-lite/blob/main/zephyr/samples/rpmsglite_pingpong/src/main.c
.

It is expected to get the output from hmp_mcuxpresso-zephyr/sources/rpmsg-lite/zephyr/samples/rpmsglite_pingpong/README.rst.

## Objective

Build a Zephyr ELF image for the Cortex-M7 core of the Toradex Verdin
iMX8M Plus using this application: hmp_mcuxpresso-zephyr/src/main.c

Linux: Master (or Application)
M7: Remote

Linux: RPMsg driver/client
M7: RPMsg endpoint/service

M7 overlay template: hmp_mcuxpresso-zephyr/sources/rpmsg-lite/zephyr/samples/rpmsglite_pingpong/remote/boards

## Configuration

* M7 board overlays  
  sources/rpmsg-lite/zephyr/  samples/rpmsglite_pingpong/remote/boards

* Target configuration  
  zephyr_app/prj_verdin_imx8mp.conf

* Zephyr source:  
  zephyr

* Zephyr SDK  
  ~/zephyr-sdk-1.0.1

* West  
~/torizon_openamp/hmp_mcuxpresso-zephyr/.venv/bin/west

* Python  
~/torizon_openamp/hmp_mcuxpresso-zephyr/.venv/bin/python

## Building

* Preparation  
```bash
cd ~/torizon_openamp/hmp_mcuxpresso-zephyr
./run_0_install_zephyr.sh
./run_1_install_mcuxpresso.sh
```

* Build  
```bash
cd ~/torizon_openamp/hmp_mcuxpresso-zephyr
./run_2_build.sh
```

## Deliverable

Produce a Cortex-M7 ELF file and report

* The build command used
* The ELF file location
* Any source, configuration, or overlay changes required
* How successful RPMsg communication with Linux was verified
