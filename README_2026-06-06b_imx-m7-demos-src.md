# Run imx-m7-demos

## Binaries origin

They come from NXP’s prebuilt demo bundle, not from C source in this recipe.

Where defined:
1. Recipe: imx-m7-demos_2.16.000.bb
2. Actual fetch URL pattern is in include: imx-mcore-demos.inc
   - `SRC_URI = ${FSL_MIRROR}/${SOC}-${MCORE_TYPE}-demo-${PV}.bin`
3. Mirror base is configured in layer.conf
   - `IMX_MIRROR = https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/`

For your machine (`mx8mp`), the fetched artifact is:
- imx8mp-m7-demo-2.16.000.bin

Unpacked build workdir is:
- 2.16.000

If you want actual source code, you need the MCUXpresso SDK / demo sources separately; this Yocto recipe packages prebuilt `.elf`/`.bin` demo firmware.

## Source Code

Actual source location (for the RPMsg examples) is in NXP MCUXpresso SDK examples:

https://github.com/nxp-mcuxpresso/mcuxsdk-examples
multicore_examples/rpmsg_lite_str_echo_rtos/remote
multicore_examples/rpmsg_lite_pingpong_rtos_linux/remote
Board-specific path for i.MX8MP: boards/evkmimx8mp/...
Important detail:

The NXP demo bundle includes only binaries plus license/SCR metadata.
I extracted the bundle and found a commit hash file: 77ca62428022fa32fbf32d0cf2f876efb8805a78.
That hash does not appear as a public GitHub ref, so the exact binary snapshot likely comes from NXP’s internal/release pipeline, while public source is in mcuxsdk-examples.

```bash
torizon$ ls -l /lib/firmware/*.elf
/lib/firmware/imx8mp_m7_TCM_hello_world.elf
https://mcuxpresso.nxp.com/mcuxsdk/latest/html/examples/multicore_examples/hello_world/readme.html

/lib/firmware/imx8mp_m7_TCM_low_power_wakeword.elf
/lib/firmware/imx8mp_m7_TCM_rpmsg_lite_pingpong_rtos_linux_remote.elf
https://github.com/nxp-mcuxpresso/mcuxsdk-examples/blob/main/_boards/evkmimx8mp/multicore_examples/rpmsg_lite_pingpong_rtos_linux/remote/example_board_readme.md
https://github.com/nxp-mcuxpresso/mcuxsdk-examples/blob/main/multicore_examples/rpmsg_lite_pingpong_rtos_linux/remote/readme.md
https://mcuxpresso.nxp.com/mcuxsdk/latest/html/examples/multicore_examples/rpmsg_lite_pingpong_rtos_linux/index.html

/lib/firmware/imx8mp_m7_TCM_rpmsg_lite_str_echo_rtos.elf
https://github.com/nxp-mcuxpresso/mcuxsdk-examples/blob/main/_boards/evkmimx8mp/multicore_examples/rpmsg_lite_str_echo_rtos/remote/example_board_readme.md
https://github.com/nxp-mcuxpresso/mcuxsdk-examples/tree/main/multicore_examples/rpmsg_lite_str_echo_rtos/remote
https://mcuxpresso.nxp.com/mcuxsdk/latest/html/examples/multicore_examples/rpmsg_lite_str_echo_rtos/remote/readme.html

/lib/firmware/imx8mp_m7_TCM_sai_low_power_audio.elf
```

## rebuild from source

The matching MCUXpresso source lives here:

- `multicore_examples/rpmsg_lite_str_echo_rtos/remote`
- `multicore_examples/rpmsg_lite_pingpong_rtos_linux/remote`
- Board-specific Verdin i.MX8MP readmes: `_boards/evkmimx8mp/multicore_examples/rpmsg_lite_str_echo_rtos/remote/example_board_readme.md` and `_boards/evkmimx8mp/multicore_examples/rpmsg_lite_pingpong_rtos_linux/remote/example_board_readme.md`

Minimal workflow:

1. Clone `https://github.com/nxp-mcuxpresso/mcuxsdk-examples`.
2. Open the matching `example_board_readme.md` for `EVK-MIMX8MP`.
3. Build the `rpmsg_lite_str_echo_rtos` and `rpmsg_lite_pingpong_rtos_linux` remote examples.
4. Copy the resulting `.elf` files into `/lib/firmware` using the names already used above.

## Similar boards

https://www.nxp.com/design/design-center/development-boards-and-designs/MIMXRT1180-EVK

Uses Arm Cortex-M7 800 MHz and Arm Cortex-M33 300 MHz
