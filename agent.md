# Agent.md

* https://www.toradex.com/computer-on-modules/verdin-arm-family/nxp-imx-8m-mini-nano

* https://www.toradex.com/products/carrier-board/mallow-carrier-board

## Goal

Build

* Yocto

  * Using docker container

* Zephyr for the Cortex-M4

Demo Application

* Blinky application on Zephyr

* Communication between Yocto and Zephyr using OpenAMP.

* Optionally:
  * Apply a pulse on GPIO A connected to the Cortex-M4.
  * Zephyr detects this pulse and applies the same pulse on a second GPIO B.
  * Using a scope, measure the delay between GPIO A and GPIO B.
  * Acceptance criterion: A-to-B propagation delay <= 5 ms.

Demo Run

* Flash Yocto

* Load Zephyr onto the Cortex-M4 using remoteproc

* Use OpenAMP for Linux <-> Cortex-M4 IPC once both cores are running

* Start app on Zephyr

