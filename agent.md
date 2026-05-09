# Agent.md

## References

- [Toradex Verdin i.MX 8M Plus](https://www.toradex.com/de/computer-on-modules/verdin-arm-family/nxp-imx-8m-plus)
- [Toradex Mallow carrier board](https://www.toradex.com/products/carrier-board/mallow-carrier-board)
- [torizon/meta-toradex-torizon](https://github.com/torizon/meta-toradex-torizon)

## Objective

- [ ] Prefer to use the torizon git repo over the yocto/zephir git repos.
- [ ] Build Yocto Linux image (inside Docker container).
- [ ] Build Zephyr firmware for Cortex-M7.
- [ ] Run Zephyr blinky demo on Cortex-M7.
- [ ] Establish Linux <-> Zephyr communication using OpenAMP.

## Execution Checklist

### Phase 1: Environment Setup

- [ ] Verify host has Docker installed and usable.
- [ ] Prepare Yocto build container and sources.
- [ ] Prepare Zephyr SDK/toolchain and workspace.

### Phase 2: Build Artifacts

- [ ] Build Yocto image for target module.
- [ ] Build Zephyr firmware for Cortex-M7.
- [ ] Confirm build outputs are generated with no blocking errors.

### Phase 3: Deploy and Boot

- [ ] Flash Yocto image to target.
- [ ] Boot Linux and verify system is healthy.
- [ ] Load and start Zephyr firmware on Cortex-M7 using remoteproc.
- [ ] Confirm Zephyr firmware starts successfully.

### Phase 4: OpenAMP Validation

- [ ] Start OpenAMP path after Linux and Cortex-M7 are both running.
- [ ] Verify bidirectional Linux <-> Cortex-M7 IPC messages.

### Phase 5: Demo Validation

- [ ] Verify Zephyr blinky is visible and stable.
- [ ] Verify OpenAMP demo traffic is stable.

## Optional Timing Demo (GPIO A -> GPIO B)

- [ ] Drive GPIO A pulse connected to Cortex-M7 input.
- [ ] Zephyr detects GPIO A edge and mirrors it to GPIO B.
- [ ] Measure delay between GPIO A and GPIO B on scope.
- [ ] Pass criterion: A-to-B propagation delay <= 5 ms.

## Final Success Criteria

- [ ] Yocto boots reliably on target.
- [ ] Zephyr runs on Cortex-M7 via remoteproc.
- [ ] OpenAMP communication works between Linux and Zephyr.
- [ ] Blinky demo runs.
- [ ] Optional GPIO timing demo meets <= 5 ms requirement.

