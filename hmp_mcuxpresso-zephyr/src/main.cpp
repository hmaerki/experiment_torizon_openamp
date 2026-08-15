/*
clang-format -i -style='{IndentWidth: 4, TabWidth: 4, UseTab: Never}'
hmp_mcuxpresso-zephyr/src/main.c
*/

#include <inttypes.h>
#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <metal/io.h>
#include <metal/sys.h>
#include <openamp/open_amp.h>

#include <resource_table.h>

extern "C" {
#include <addr_translation.h>
}

// Use of the kernel module with the same name
// #define channel "rpmsg-client-sample"
// Use with rpmsg_lite_sample.py
#define RPMSG_CHAR_CHANNEL_NAME "rpmsg-client-sample-py"

LOG_MODULE_REGISTER(rpmsg_client_sample);

#if !DT_HAS_CHOSEN(zephyr_ipc_shm)
#error "Application requires zephyr,ipc_shm"
#endif

#if CONFIG_IPM_MAX_DATA_SIZE > 0
#define IPM_SEND(dev, wait, id, data, size) ipm_send(dev, wait, id, data, size)
#else
#define IPM_SEND(dev, wait, id, data, size) ipm_send(dev, wait, id, NULL, 0)
#endif

#define SHM_NODE DT_CHOSEN(zephyr_ipc_shm)
#define SHM_START_ADDR DT_REG_ADDR(SHM_NODE)
#define SHM_SIZE DT_REG_SIZE(SHM_NODE)
#define APP_TASK_STACK_SIZE 2048

static K_SEM_DEFINE(ipm_sem, 0, 1);
static K_SEM_DEFINE(client_ready_sem, 0, 1);

static const struct device *const ipm = DEVICE_DT_GET(DT_CHOSEN(zephyr_ipc));
static metal_phys_addr_t shm_physmap = SHM_START_ADDR;
static metal_phys_addr_t resource_table_physmap;
static struct metal_io_region shm_io_data;
static struct metal_io_region resource_table_io_data;
static struct rpmsg_virtio_device rvdev;
static struct rpmsg_device *rpdev;
static struct rpmsg_endpoint char_endpoint;
static void *resource_table;

static void ipm_callback(const struct device *dev, void *context, uint32_t id,
                         volatile void *data) {
    auto *table = static_cast<struct fw_resource_table *>(resource_table);

    ARG_UNUSED(dev);
    ARG_UNUSED(context);
    ARG_UNUSED(id);
    ARG_UNUSED(data);
    k_sem_give(&ipm_sem);
    if (table != NULL) {
        LOG_INF("ipm_callback(): status=0x%02x, vring0.da=0x%08" PRIx32
                ", vring1.da=0x%08" PRIx32,
                (unsigned int)table->vdev.status, table->vring0.da,
                table->vring1.da);
    }
}

static int char_endpoint_callback(struct rpmsg_endpoint *ept, void *data,
                                  size_t len, uint32_t src, void *priv) {
    ARG_UNUSED(src);
    ARG_UNUSED(priv);

    // The * means: take the precision from the next argument.
    LOG_INF("RPMsg char received %u bytes: '%.*s'", (unsigned int)len,
            (unsigned int)len, (char *)data);
    return rpmsg_send(ept, data, len);
}

static void new_service_callback(struct rpmsg_device *rdev, const char *name,
                                 uint32_t src) {
    ARG_UNUSED(rdev);
    LOG_WRN("Unexpected name service announcement: %s at 0x%x", name, src);
}

static int mailbox_notify(void *priv, uint32_t id) {
    uint32_t message = id << 16;

    ARG_UNUSED(priv);
    LOG_INF("mailbox_notify(): vring=%" PRIu32 ", channel=%d", id,
            CONFIG_OPENAMP_RSC_TABLE_IPM_TX_ID);
    return IPM_SEND(ipm, 0, CONFIG_OPENAMP_RSC_TABLE_IPM_TX_ID, &message,
                    sizeof(message));
}

static int platform_init(void) {
    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    int resource_table_size;
    int ret;

    LOG_INF("platform_init(): metal_init");
    ret = metal_init(&metal_params);
    if (ret != 0) {
        LOG_ERR("metal_init failed: %d", ret);
        return ret;
    }

    LOG_INF("platform_init(): metal_io_init");
    metal_io_init(&shm_io_data, (void *)SHM_START_ADDR, &shm_physmap, SHM_SIZE,
                  -1, 0, addr_translation_get_ops(shm_physmap));

    LOG_INF("platform_init(): rsc_table_get");
    rsc_table_get(&resource_table, &resource_table_size);
    LOG_INF("platform_init(): resource_table_size=%d", resource_table_size);

    auto *ptr = static_cast<struct fw_resource_table *>(resource_table);
    LOG_INF("0x%08" PRIxPTR " resource-table header", (uintptr_t)&ptr->hdr);
    LOG_INF("  version=%" PRIu32 ", entries=%" PRIu32, ptr->hdr.ver,
            ptr->hdr.num);
    LOG_INF("0x%08" PRIxPTR " resource-table offsets",
            (uintptr_t)&ptr->offset[0]);
    LOG_INF("  offset[0]=0x%08" PRIx32, ptr->offset[0]);
    LOG_INF("0x%08" PRIxPTR " virtio device", (uintptr_t)&ptr->vdev);
    LOG_INF("0x%08" PRIxPTR " virtio status: 0x%02x",
            (uintptr_t)&ptr->vdev.status, (unsigned int)ptr->vdev.status);
    LOG_INF("0x%08" PRIxPTR " vring0: da=0x%08" PRIx32 ", notifyid=%" PRIu32,
            (uintptr_t)&ptr->vring0, ptr->vring0.da, ptr->vring0.notifyid);
    LOG_INF("0x%08" PRIxPTR " vring1: da=0x%08" PRIx32 ", notifyid=%" PRIu32,
            (uintptr_t)&ptr->vring1, ptr->vring1.da, ptr->vring1.notifyid);

    resource_table_physmap = (uintptr_t)resource_table;
    LOG_INF("platform_init(): metal_io_init");
    metal_io_init(&resource_table_io_data, resource_table,
                  &resource_table_physmap, resource_table_size, -1, 0, NULL);

    if (!device_is_ready(ipm)) {
        LOG_ERR("IPM device is not ready");
        return -ENODEV;
    }
    LOG_INF("platform_init(): device_is_ready");

    LOG_INF("platform_init(): ipm_register_callback");
    ipm_register_callback(ipm, ipm_callback, NULL);
    LOG_INF("platform_init(): ipm_set_enabled");
    ret = ipm_set_enabled(ipm, 1);
    if (ret != 0) {
        LOG_ERR("ipm_set_enabled failed: %d", ret);
    }

    LOG_INF("platform_init(): ipm_set_enabled");
    return ret;
}

static struct rpmsg_device *create_rpmsg_device(void) {
    struct fw_rsc_vdev_vring *vring;
    struct virtio_device *vdev;
    int ret;

    LOG_INF("create_rpmsg_device(): rproc_virtio_create_vdev");
    vdev = rproc_virtio_create_vdev(
        VIRTIO_DEV_DEVICE, VDEV_ID, rsc_table_to_vdev(resource_table),
        &resource_table_io_data, NULL, mailbox_notify, NULL);
    if (vdev == NULL) {
        LOG_ERR("Failed to create virtio device");
        return NULL;
    }

    LOG_INF("create_rpmsg_device(): rproc_virtio_wait_remote_ready");
    rproc_virtio_wait_remote_ready(vdev);

    LOG_INF("create_rpmsg_device(): rsc_table_get_vring0");
    vring = rsc_table_get_vring0(resource_table);
    LOG_INF("create_rpmsg_device(): rproc_virtio_init_vring");
    ret = rproc_virtio_init_vring(vdev, 0, vring->notifyid, (void *)vring->da,
                                  &shm_io_data, vring->num, vring->align);
    if (ret != 0) {
        LOG_ERR("Failed to initialize vring 0: %d", ret);
        goto fail;
    }

    LOG_INF("create_rpmsg_device(): rsc_table_get_vring1");
    vring = rsc_table_get_vring1(resource_table);
    LOG_INF("create_rpmsg_device(): rproc_virtio_init_vring");
    ret = rproc_virtio_init_vring(vdev, 1, vring->notifyid, (void *)vring->da,
                                  &shm_io_data, vring->num, vring->align);
    if (ret != 0) {
        LOG_ERR("Failed to initialize vring 1: %d", ret);
        goto fail;
    }

    LOG_INF("create_rpmsg_device(): rpmsg_init_vdev");
    ret =
        rpmsg_init_vdev(&rvdev, vdev, new_service_callback, &shm_io_data, NULL);
    if (ret != 0) {
        LOG_ERR("rpmsg_init_vdev failed: %d", ret);
        goto fail;
    }

    LOG_INF("create_rpmsg_device(): rpmsg_virtio_get_rpmsg_device");
    return rpmsg_virtio_get_rpmsg_device(&rvdev);

fail:
    rproc_virtio_remove_vdev(vdev);
    return NULL;
}

static void manager(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF("manager(): platform_init");
    if (platform_init() != 0) {
        return;
    }

    LOG_INF("manager(): create_rpmsg_device");
    rpdev = create_rpmsg_device();
    if (rpdev == NULL) {
        return;
    }

    LOG_INF("manager(): k_sem_give");
    k_sem_give(&client_ready_sem);
    while (true) {
        // This awakens after ipm_callback()
        k_sem_take(&ipm_sem, K_FOREVER);
        // proc_virtio_notified() can process received RPMsg data from VRING1_ID.
        LOG_INF("manager(): rproc_virtio_notified");
        rproc_virtio_notified(rvdev.vdev, VRING1_ID);
    }
}

static void client(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    k_sem_take(&client_ready_sem, K_FOREVER);

    LOG_INF("client(): create '%s' endpoint", RPMSG_CHAR_CHANNEL_NAME);
    int ret = rpmsg_create_ept(&char_endpoint, rpdev, RPMSG_CHAR_CHANNEL_NAME,
                               RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                               char_endpoint_callback, NULL);
    if (ret != 0) {
        LOG_ERR("Could not create RPMsg char endpoint: %d", ret);
        return;
    }

    LOG_INF("Linux RPMsg char endpoint is ready");
}

K_THREAD_DEFINE(manager_thread, APP_TASK_STACK_SIZE, manager, NULL, NULL, NULL,
                K_PRIO_COOP(8), 0, 0);
K_THREAD_DEFINE(client_thread, APP_TASK_STACK_SIZE, client, NULL, NULL, NULL,
                K_PRIO_COOP(7), 0, 0);

int main(void) {
    LOG_INF("Starting Verdin iMX8MP OpenAMP remote");
    printk("Starting Verdin iMX8MP OpenAMP remote!\n");

    return 0;
}
