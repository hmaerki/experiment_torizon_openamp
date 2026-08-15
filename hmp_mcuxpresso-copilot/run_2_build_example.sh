#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCES_DIR="$ROOT_DIR/sources"
MCUX_SDK_DIR="$SOURCES_DIR/mcux-sdk"
BUILD_DIR="$ROOT_DIR/build/mcux-example-minimal-cm7"
RPMSGLITE_DIR="$SOURCES_DIR/rpmsg-lite"

ZEPHYR_SDK_DIR="${ZEPHYR_SDK_DIR:-$HOME/zephyr-sdk-1.0.1}"
TOOLCHAIN_BIN_DIR="$ZEPHYR_SDK_DIR/gnu/arm-zephyr-eabi/bin"

CC="$TOOLCHAIN_BIN_DIR/arm-zephyr-eabi-gcc"
OBJCOPY="$TOOLCHAIN_BIN_DIR/arm-zephyr-eabi-objcopy"
SIZE="$TOOLCHAIN_BIN_DIR/arm-zephyr-eabi-size"

DEVICE="MIMX8ML8"
CPU_DEFINE="CPU_MIMX8ML8DVNLZ"
RPMSGLITE_ENV="bm"
RPMSGLITE_PLATFORM="imx8mp_m7"

DEVICE_DIR="$MCUX_SDK_DIR/devices/$DEVICE"
CMSIS_CORE_DIR="$MCUX_SDK_DIR/core/CMSIS/Core/Include"
DEVICE_DRIVERS_DIR="$DEVICE_DIR/drivers"
MU_DRIVERS_DIR="$MCUX_SDK_DIR/drivers/mu"
COMMON_DRIVERS_DIR="$MCUX_SDK_DIR/drivers/common"
RPMSGLITE_ENV_INCLUDE_DIR="$RPMSGLITE_DIR/lib/include/environment/$RPMSGLITE_ENV"
RPMSGLITE_PLATFORM_INCLUDE_DIR="$RPMSGLITE_DIR/lib/include/platform/$RPMSGLITE_PLATFORM"
RPMSGLITE_CONFIG_DIR="$RPMSGLITE_DIR/template_application"
RPMSGLITE_ENV_SOURCE_FILE="$RPMSGLITE_DIR/lib/rpmsg_lite/porting/environment/rpmsg_env_${RPMSGLITE_ENV}.c"
RPMSGLITE_PLATFORM_SOURCE_FILE="$RPMSGLITE_DIR/lib/rpmsg_lite/porting/platform/$RPMSGLITE_PLATFORM/rpmsg_platform.c"

STARTUP_FILE="$DEVICE_DIR/gcc/startup_${DEVICE}_cm7.S"
SYSTEM_FILE="$DEVICE_DIR/system_${DEVICE}_cm7.c"
LINKER_SCRIPT="$DEVICE_DIR/gcc/${DEVICE}xxxxx_cm7_ram.ld"

MAIN_C="$BUILD_DIR/main.c"
OUT_ELF="$BUILD_DIR/rpmsg_demo_${DEVICE}_cm7.elf"
OUT_BIN="$BUILD_DIR/rpmsg_demo_${DEVICE}_cm7.bin"
OUT_MAP="$BUILD_DIR/rpmsg_demo_${DEVICE}_cm7.map"

if [ ! -d "$MCUX_SDK_DIR" ]; then
  echo "MCUX SDK not found at $MCUX_SDK_DIR"
  exit 1
fi

if [ ! -d "$RPMSGLITE_DIR" ]; then
  echo "RPMsg-Lite source not found at $RPMSGLITE_DIR"
  echo "Run bootstrap first: ./run_0_bootstrap.sh"
  exit 1
fi

if [ ! -d "$ZEPHYR_SDK_DIR" ]; then
  echo "Zephyr SDK not found at $ZEPHYR_SDK_DIR"
  echo "Set ZEPHYR_SDK_DIR if your SDK is installed elsewhere."
  exit 1
fi

if [ ! -x "$CC" ]; then
  echo "Cross compiler not found: $CC"
  exit 1
fi

if [ ! -x "$OBJCOPY" ]; then
  echo "objcopy not found: $OBJCOPY"
  exit 1
fi

if [ ! -x "$SIZE" ]; then
  echo "size tool not found: $SIZE"
  exit 1
fi

if [ ! -d "$CMSIS_CORE_DIR" ]; then
  echo "CMSIS Core headers not found at $CMSIS_CORE_DIR"
  echo "Run bootstrap first: ./run_0_bootstrap.sh"
  exit 1
fi

for f in "$STARTUP_FILE" "$SYSTEM_FILE" "$LINKER_SCRIPT" "$DEVICE_DIR/fsl_device_registers.h"; do
  if [ ! -f "$f" ]; then
    echo "Required file not found: $f"
    exit 1
  fi
done

for d in "$DEVICE_DRIVERS_DIR" "$MU_DRIVERS_DIR" "$COMMON_DRIVERS_DIR" "$RPMSGLITE_ENV_INCLUDE_DIR" "$RPMSGLITE_PLATFORM_INCLUDE_DIR" "$RPMSGLITE_CONFIG_DIR"; do
  if [ ! -d "$d" ]; then
    echo "Required directory not found: $d"
    exit 1
  fi
done

for f in "$RPMSGLITE_CONFIG_DIR/rpmsg_config.h" "$RPMSGLITE_ENV_SOURCE_FILE" "$RPMSGLITE_PLATFORM_SOURCE_FILE"; do
  if [ ! -f "$f" ]; then
    echo "Required RPMsg-Lite file not found: $f"
    exit 1
  fi
done

mkdir -p "$BUILD_DIR"

cat > "$MAIN_C" <<'EOF'
#include "fsl_device_registers.h"
#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include <stdint.h>

/* Linux remoteproc resource table definitions. */
#define RSC_VDEV (3U)
#define VIRTIO_ID_RPMSG (7U)
#define VIRTIO_RPMSG_F_NS (0U)
#define FW_RSC_ADDR_ANY (0xFFFFFFFFU)

struct resource_table_header
{
  uint32_t ver;
  uint32_t num;
  uint32_t reserved[2];
  uint32_t offset[1];
};

struct fw_rsc_vdev
{
  uint32_t type;
  uint32_t id;
  uint32_t notifyid;
  uint32_t dfeatures;
  uint32_t gfeatures;
  uint32_t config_len;
  uint8_t status;
  uint8_t num_of_vrings;
  uint8_t reserved[2];
};

struct fw_rsc_vdev_vring
{
  uint32_t da;
  uint32_t align;
  uint32_t num;
  uint32_t notifyid;
  uint32_t reserved;
};

struct rpmsg_resource_table
{
  struct resource_table_header header;
  struct fw_rsc_vdev vdev;
  struct fw_rsc_vdev_vring vring0;
  struct fw_rsc_vdev_vring vring1;
};

/* Keep this in a dedicated section that the linker script preserves. */
__attribute__((section(".resource_table"), used))
static const struct rpmsg_resource_table resources = {
  .header = {
    .ver = 1,
    .num = 1,
    .reserved = {0, 0},
    .offset = {sizeof(struct resource_table_header)},
  },
  .vdev = {
    .type = RSC_VDEV,
    .id = VIRTIO_ID_RPMSG,
    .notifyid = 0,
    .dfeatures = (1U << VIRTIO_RPMSG_F_NS),
    .gfeatures = 0,
    .config_len = 0,
    .status = 0,
    .num_of_vrings = 2,
    .reserved = {0, 0},
  },
  .vring0 = {
    .da = FW_RSC_ADDR_ANY,
    .align = 0x1000,
    .num = 16,
    .notifyid = 1,
    .reserved = 0,
  },
  .vring1 = {
    .da = FW_RSC_ADDR_ANY,
    .align = 0x1000,
    .num = 16,
    .notifyid = 2,
    .reserved = 0,
  },
};

/* Keep sample shared memory in noinit so the section can be reused by remoteproc workflows. */
#define SHMEM_SIZE (0x6000U)

__attribute__((section(".noinit.$rpmsg_sh_mem"))) static unsigned char rpmsg_shmem[SHMEM_SIZE];
static struct rpmsg_lite_instance rpmsg_ctxt;
static struct rpmsg_lite_ept_static_context ept_ctxt;
static struct rpmsg_lite_endpoint *ept;

static int32_t rx_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
  (void)payload;
  (void)payload_len;
  (void)src;
  (void)priv;
  return RL_RELEASE;
}

int main(void)
{
  struct rpmsg_lite_instance *inst;

    SystemCoreClockUpdate();
  env_init();

  inst = rpmsg_lite_master_init((void *)rpmsg_shmem, SHMEM_SIZE,
                  RL_PLATFORM_IMX8MP_M7_USER_LINK_ID,
                  RL_NO_FLAGS, &rpmsg_ctxt);
  if (inst == NULL)
  {
    for (;;)
    {
      __asm volatile("nop");
    }
  }

  ept = rpmsg_lite_create_ept(inst, RL_ADDR_ANY, rx_cb, NULL, &ept_ctxt);
  if (ept == NULL)
  {
    for (;;)
    {
      __asm volatile("nop");
    }
  }

  (void)rpmsg_ns_announce(inst, ept, "rpmsg-lite-sample", RL_NS_CREATE);

    for (;;)
    {
        __asm volatile("nop");
    }
}
EOF

CFLAGS=(
  -mcpu=cortex-m7
  -mthumb
  -O2
  -ffunction-sections
  -fdata-sections
  -DNDEBUG
  -D"$CPU_DEFINE"
  -I"$DEVICE_DIR"
  -I"$CMSIS_CORE_DIR"
  -I"$DEVICE_DRIVERS_DIR"
  -I"$MU_DRIVERS_DIR"
  -I"$COMMON_DRIVERS_DIR"
  -I"$RPMSGLITE_DIR/lib/include"
  -I"$RPMSGLITE_ENV_INCLUDE_DIR"
  -I"$RPMSGLITE_PLATFORM_INCLUDE_DIR"
  -I"$RPMSGLITE_CONFIG_DIR"
)

ASFLAGS=(
  "${CFLAGS[@]}"
  -D__START=main
)

echo "Compiling RPMsg-Lite sample sources"
SOURCES=(
  "$MAIN_C"
  "$SYSTEM_FILE"
  "$STARTUP_FILE"
  "$COMMON_DRIVERS_DIR/fsl_common.c"
  "$MU_DRIVERS_DIR/fsl_mu.c"
  "$DEVICE_DRIVERS_DIR/fsl_clock.c"
  "$RPMSGLITE_DIR/lib/common/llist.c"
  "$RPMSGLITE_DIR/lib/virtio/virtqueue.c"
  "$RPMSGLITE_DIR/lib/rpmsg_lite/rpmsg_lite.c"
  "$RPMSGLITE_DIR/lib/rpmsg_lite/rpmsg_ns.c"
  "$RPMSGLITE_DIR/lib/rpmsg_lite/rpmsg_queue.c"
  "$RPMSGLITE_ENV_SOURCE_FILE"
  "$RPMSGLITE_PLATFORM_SOURCE_FILE"
)

OBJS=()
for src in "${SOURCES[@]}"; do
  obj="$BUILD_DIR/$(basename "${src%.*}").o"
  if [[ "$src" == *.S ]]; then
    "$CC" "${ASFLAGS[@]}" -c "$src" -o "$obj"
  else
    "$CC" "${CFLAGS[@]}" -c "$src" -o "$obj"
  fi
  OBJS+=("$obj")
done

echo "Linking ELF"
"$CC" \
  "${OBJS[@]}" \
  -T "$LINKER_SCRIPT" \
  -Wl,--gc-sections \
  -Wl,-Map="$OUT_MAP" \
  -nostartfiles \
  -o "$OUT_ELF"

echo "Generating BIN"
"$OBJCOPY" -O binary "$OUT_ELF" "$OUT_BIN"

echo "Size report"
"$SIZE" "$OUT_ELF"

echo "Build finished."
echo "  ELF: $OUT_ELF"
echo "  BIN: $OUT_BIN"
echo "  MAP: $OUT_MAP"