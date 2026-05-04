# Agent.md

## References

- https://www.toradex.com/computer-on-modules/verdin-arm-family/ti-am62p
- https://www.toradex.com/products/carrier-board/mallow-carrier-board
- https://github.com/torizon/meta-toradex-torizon

## Objective

- [ ] Prefer to use the torizon git repo; FreeRTOS support is included in the default Torizon configuration.
- [ ] Build Yocto Linux image (inside Docker container).
- [ ] Build FreeRTOS firmware for R5F.
- [ ] Run FreeRTOS blinky demo on R5F.
- [ ] Establish Linux <-> FreeRTOS communication using OpenAMP.

## Execution Checklist

### Phase 1: Environment Setup

- [ ] Verify host has Docker installed and usable.
- [ ] Prepare Yocto build container and sources.
- [ ] Prepare FreeRTOS SDK/toolchain and workspace.

### Phase 2: Build Artifacts

- [ ] Build Yocto image for target module.
- [ ] Build FreeRTOS firmware for R5F.
- [ ] Confirm build outputs are generated with no blocking errors.

### Phase 3: Deploy and Boot

- [ ] Flash Yocto image to target.
- [ ] Boot Linux and verify system is healthy.
- [ ] Load and start FreeRTOS firmware on R5F using remoteproc.
- [ ] Confirm FreeRTOS firmware starts successfully.

### Phase 4: OpenAMP Validation

- [ ] Start OpenAMP path after Linux and R5F are both running.
- [ ] Verify bidirectional Linux <-> R5F IPC messages.

### Phase 5: Demo Validation

- [ ] Verify FreeRTOS blinky is visible and stable.
- [ ] Verify OpenAMP demo traffic is stable.

## Optional Timing Demo (GPIO A -> GPIO B)

- [ ] Drive GPIO A pulse connected to R5F input.
- [ ] FreeRTOS detects GPIO A edge and mirrors it to GPIO B.
- [ ] Measure delay between GPIO A and GPIO B on scope.
- [ ] Pass criterion: A-to-B propagation delay <= 5 ms.

## Final Success Criteria

- [ ] Yocto boots reliably on target.
- [ ] FreeRTOS runs on R5F via remoteproc.
- [ ] OpenAMP communication works between Linux and FreeRTOS.
- [ ] Blinky demo runs.
- [ ] Optional GPIO timing demo meets <= 5 ms requirement.

