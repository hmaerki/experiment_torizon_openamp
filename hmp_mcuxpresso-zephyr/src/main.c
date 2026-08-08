/*
 * Copyright (c) 2020 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <inttypes.h>

#include <zephyr/device.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <metal/io.h>
#include <metal/sys.h>
#include <openamp/open_amp.h>

#include <addr_translation.h>
#include <resource_table.h>

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

K_THREAD_STACK_DEFINE(manager_stack, APP_TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(client_stack, APP_TASK_STACK_SIZE);

static struct k_thread manager_thread;
static struct k_thread client_thread;
static K_SEM_DEFINE(ipm_sem, 0, 1);
static K_SEM_DEFINE(client_ready_sem, 0, 1);

static const struct device *const ipm = DEVICE_DT_GET(DT_CHOSEN(zephyr_ipc));
static metal_phys_addr_t shm_physmap = SHM_START_ADDR;
static metal_phys_addr_t resource_table_physmap;
static struct metal_io_region shm_io_data;
static struct metal_io_region resource_table_io_data;
static struct rpmsg_virtio_device rvdev;
static struct rpmsg_device *rpdev;
static struct rpmsg_endpoint endpoint;
static void *resource_table;

static void ipm_callback(const struct device *dev, void *context,
             uint32_t id, volatile void *data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(context);
    ARG_UNUSED(id);
    ARG_UNUSED(data);
    k_sem_give(&ipm_sem);
  	LOG_INF("ipm_callback()");
}

static int endpoint_callback(struct rpmsg_endpoint *ept, void *data,
                 size_t len, uint32_t src, void *priv)
{
    ARG_UNUSED(src);
    ARG_UNUSED(priv);

    LOG_INF("Received %u bytes", (unsigned int)len);
    return rpmsg_send(ept, data, len);
}

static void new_service_callback(struct rpmsg_device *rdev, const char *name,
                 uint32_t src)
{
    ARG_UNUSED(rdev);
    LOG_WRN("Unexpected name service announcement: %s at 0x%x", name, src);
}

static int mailbox_notify(void *priv, uint32_t id)
{
    ARG_UNUSED(priv);
  	LOG_INF("mailbox_notify()");
    return IPM_SEND(ipm, 0, id, &id, sizeof(id));
}

static int platform_init(void)
{
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
    metal_io_init(&shm_io_data, (void *)SHM_START_ADDR, &shm_physmap,
              SHM_SIZE, -1, 0, addr_translation_get_ops(shm_physmap));

  	LOG_INF("platform_init(): rsc_table_get");
    rsc_table_get(&resource_table, &resource_table_size);
    LOG_INF("platform_init(): resource_table_size=%d", resource_table_size);

    struct fw_resource_table *ptr = resource_table;
    LOG_INF("0x%08" PRIxPTR " resource-table header",
      (uintptr_t)&ptr->hdr);
    LOG_INF("  version=%" PRIu32 ", entries=%" PRIu32,
      ptr->hdr.ver, ptr->hdr.num);
    LOG_INF("0x%08" PRIxPTR " resource-table offsets",
      (uintptr_t)&ptr->offset[0]);
    LOG_INF("  offset[0]=0x%08" PRIx32, ptr->offset[0]);
    LOG_INF("0x%08" PRIxPTR " virtio device",
      (uintptr_t)&ptr->vdev);
    LOG_INF("0x%08" PRIxPTR " virtio status: 0x%02x",
      (uintptr_t)&ptr->vdev.status, (unsigned int)ptr->vdev.status);
    LOG_INF("0x%08" PRIxPTR " vring0: da=0x%08" PRIx32
      ", notifyid=%" PRIu32,
      (uintptr_t)&ptr->vring0, ptr->vring0.da, ptr->vring0.notifyid);
    LOG_INF("0x%08" PRIxPTR " vring1: da=0x%08" PRIx32
      ", notifyid=%" PRIu32,
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

static struct rpmsg_device *create_rpmsg_device(void)
{
    struct fw_rsc_vdev_vring *vring;
    struct virtio_device *vdev;
    int ret;

  	LOG_INF("create_rpmsg_device(): rproc_virtio_create_vdev");
    vdev = rproc_virtio_create_vdev(VIRTIO_DEV_DEVICE, VDEV_ID,
                    rsc_table_to_vdev(resource_table),
                    &resource_table_io_data, NULL,
                    mailbox_notify, NULL);
    if (vdev == NULL) {
        LOG_ERR("Failed to create virtio device");
        return NULL;
    }

  	LOG_INF("create_rpmsg_device(): rproc_virtio_wait_remote_ready");
    rproc_virtio_wait_remote_ready(vdev);

  	LOG_INF("create_rpmsg_device(): rsc_table_get_vring0");
    vring = rsc_table_get_vring0(resource_table);
  	LOG_INF("create_rpmsg_device(): rproc_virtio_init_vring");
    ret = rproc_virtio_init_vring(vdev, 0, vring->notifyid,
                      (void *)vring->da, &resource_table_io_data,
                      vring->num, vring->align);
    if (ret != 0) {
        LOG_ERR("Failed to initialize vring 0: %d", ret);
        goto fail;
    }

  	LOG_INF("create_rpmsg_device(): rsc_table_get_vring1");
    vring = rsc_table_get_vring1(resource_table);
  	LOG_INF("create_rpmsg_device(): rproc_virtio_init_vring");
    ret = rproc_virtio_init_vring(vdev, 1, vring->notifyid,
                      (void *)vring->da, &resource_table_io_data,
                      vring->num, vring->align);
    if (ret != 0) {
        LOG_ERR("Failed to initialize vring 1: %d", ret);
        goto fail;
    }

  	LOG_INF("create_rpmsg_device(): rpmsg_init_vdev");
    ret = rpmsg_init_vdev(&rvdev, vdev, new_service_callback,
                  &shm_io_data, NULL);
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

static void manager(void *arg1, void *arg2, void *arg3)
{
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
        k_sem_take(&ipm_sem, K_FOREVER);
    	LOG_INF("manager(): rproc_virtio_notified");
        rproc_virtio_notified(rvdev.vdev, VRING1_ID);
    }
}

static void client(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    k_sem_take(&client_ready_sem, K_FOREVER);

    LOG_INF("client(): rpmsg_create_ept");
    int ret = rpmsg_create_ept(&endpoint, rpdev, "rpmsg-client-sample",
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   endpoint_callback, NULL);
    if (ret != 0) {
        LOG_ERR("Could not create endpoint: %d", ret);
        return;
    }

    LOG_INF("Linux rpmsg-client-sample endpoint is ready");
}

int main(void)
{
    LOG_INF("Starting Verdin iMX8MP OpenAMP remote");
    printk("Starting Verdin iMX8MP OpenAMP remote!\n");

    k_thread_create(&manager_thread, manager_stack, APP_TASK_STACK_SIZE,
            manager, NULL, NULL, NULL, K_PRIO_COOP(8), 0, K_NO_WAIT);
    k_thread_create(&client_thread, client_stack, APP_TASK_STACK_SIZE,
            client, NULL, NULL, NULL, K_PRIO_COOP(7), 0, K_NO_WAIT);

    return 0;
}
#if 0
/*
This file was copied from:
hmp_mcuxpresso-zephyr/sources/rpmsg-lite/zephyr/samples/rpmsglite_pingpong/src/main.c
https://github.com/nxp-mcuxpresso/rpmsg-lite/blob/main/zephyr/samples/rpmsglite_pingpong/src/main.c
*/

/*
 * Copyright 2023-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/drivers/ipm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/init.h>

#include "common.h"
#include "rpmsg_lite.h"

#if defined(CONFIG_BOARD_MIMXRT685_EVK) || defined(CONFIG_BOARD_MIMXRT700_EVK)
#include "dsp.h"
#endif

#define REMOTE_EPT_ADDR               (30U)
#define LOCAL_EPT_ADDR                (40U)
#define APP_RPMSG_READY_EVENT_DATA    (1U)
#define APP_RPMSG_EP_READY_EVENT_DATA (2U)

#define SHM_MEM_ADDR DT_REG_ADDR(DT_CHOSEN(zephyr_ipc_shm))
#define SHM_MEM_SIZE DT_REG_SIZE(DT_CHOSEN(zephyr_ipc_shm))

#define APP_THREAD_STACK_SIZE (1024)
K_THREAD_STACK_DEFINE(thread_stack, APP_THREAD_STACK_SIZE);
static struct k_thread thread_data;

struct rpmsg_lite_instance *gp_rpmsg_dev_inst;
struct rpmsg_lite_endpoint *gp_rpmsg_ept;
struct rpmsg_lite_instance g_rpmsg_ctxt;
struct rpmsg_lite_ept_static_context g_ept_context;
volatile int32_t g_has_received;

uint32_t *shared_memory = (uint32_t *)SHM_MEM_ADDR;

K_EVENT_DEFINE(wait_event);

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

static THE_MESSAGE volatile g_msg = {0};

/* This is the read callback, note we are in a task context when this callback
is invoked, so kernel primitives can be used freely */
static int32_t rpmsg_ept_read_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    int32_t *has_received = priv;

    if (payload_len <= sizeof(THE_MESSAGE))
    {
        (void)memcpy((void *)&g_msg, payload, payload_len);
        *has_received = 1;
    }
    (void)printk("Primary core received a msg\r\n");
    (void)printk("Message: Size=%x, DATA = %i\r\n", payload_len, g_msg.DATA);
    return RL_RELEASE;
}

static void application_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

   	LOG_INF("application_thread()");

   	LOG_INF("rpmsg_lite_send().rpmsg_lite_master_init");
    gp_rpmsg_dev_inst =
        rpmsg_lite_master_init(shared_memory, SHM_MEM_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS, &g_rpmsg_ctxt);

   	LOG_INF("rpmsg_lite_send().rpmsg_lite_create_ept");
    gp_rpmsg_ept = rpmsg_lite_create_ept(gp_rpmsg_dev_inst, LOCAL_EPT_ADDR, rpmsg_ept_read_cb, (void *)&g_has_received,
                                         &g_ept_context);

    /* Temporary wait a bit here until remote core will process initialization and is ready to receive data */
   	LOG_INF("rpmsg_lite_send().k_event_wait_all");
    (void)k_event_wait_all(&wait_event, 5, false, K_MSEC(50));

    /* Send the first message to the remoteproc */
    g_msg.DATA = 0U;
   	LOG_INF("rpmsg_lite_send().rpmsg_lite_send %d", g_msg.DATA);
    (void)rpmsg_lite_send(gp_rpmsg_dev_inst, gp_rpmsg_ept, REMOTE_EPT_ADDR, (char *)&g_msg, sizeof(THE_MESSAGE),
                          RL_DONT_BLOCK);

    while (g_msg.DATA <= 100U)
    {
        if (1 == g_has_received)
        {
            g_has_received = 0;
            g_msg.DATA++;
           	LOG_INF("rpmsg_lite_send().rpmsg_lite_send %d", g_msg.DATA);
            (void)rpmsg_lite_send(gp_rpmsg_dev_inst, gp_rpmsg_ept, REMOTE_EPT_ADDR, (char *)&g_msg, sizeof(THE_MESSAGE),
                                  RL_DONT_BLOCK);
        }
    }

   	LOG_INF("rpmsg_lite_send().rpmsg_lite_destroy_ept %d", g_msg.DATA);
    (void)rpmsg_lite_destroy_ept(gp_rpmsg_dev_inst, gp_rpmsg_ept);
    gp_rpmsg_ept = ((void *)0);
    (void)rpmsg_lite_deinit(gp_rpmsg_dev_inst);

    /* Print the ending banner */
    (void)printk("\r\nRPMsg demo ends\r\n");
   	LOG_INF("nRPMsg demo ends");
}

int main(void)
{
#if defined(CONFIG_BOARD_MIMXRT685_EVK) || defined(CONFIG_BOARD_MIMXRT700_EVK)
    dsp_start();
    k_sleep(K_MSEC(500));
#endif

	LOG_INF("LOG_INF: Starting application thread on Main Core!");
    printk("Starting application thread on Main Core!\n");
    k_thread_create(&thread_data, thread_stack, APP_THREAD_STACK_SIZE, application_thread, NULL, NULL, NULL,
                    K_PRIO_COOP(7), 0, K_NO_WAIT);

    return 0;
}
#endif
