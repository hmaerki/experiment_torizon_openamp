#include <stddef.h>
#include <string.h>

#include <zephyr/kernel.h>

#include <resource_table.h>

#define IMX8MP_VRING0_ADDRESS 0x55000000U
#define IMX8MP_VRING1_ADDRESS 0x55008000U
#define IMX8MP_RSC_TABLE_ADDRESS DT_REG_ADDR(DT_CHOSEN(zephyr_ipc_rsc_table))
#define IMX8MP_RSC_TABLE_SIZE DT_REG_SIZE(DT_CHOSEN(zephyr_ipc_rsc_table))

#define __resource Z_GENERIC_SECTION(.resource_table)

static struct fw_resource_table __resource resource_table = {
	.hdr = {
		.ver = 1,
		.num = RSC_TABLE_NUM_ENTRY,
	},
	.offset = {
		offsetof(struct fw_resource_table, vdev),
	},
	.vdev = {
		RSC_VDEV,
		VIRTIO_ID_RPMSG,
		0,
		RPMSG_IPU_C0_FEATURES,
		0,
		0,
		0,
		VRING_COUNT,
		{0, 0},
	},
	.vring0 = {
		IMX8MP_VRING0_ADDRESS,
		CONFIG_OPENAMP_VRING_ALIGNMENT,
		CONFIG_OPENAMP_RSC_TABLE_NUM_RPMSG_BUFF,
		VRING0_ID,
		0,
	},
	.vring1 = {
		IMX8MP_VRING1_ADDRESS,
		CONFIG_OPENAMP_VRING_ALIGNMENT,
		CONFIG_OPENAMP_RSC_TABLE_NUM_RPMSG_BUFF,
		VRING1_ID,
		0,
	},
};

BUILD_ASSERT(sizeof(resource_table) <= IMX8MP_RSC_TABLE_SIZE);

void rsc_table_get(void **table_ptr, int *length)
{
	*length = sizeof(resource_table);
	*table_ptr = (void *)IMX8MP_RSC_TABLE_ADDRESS;
	memcpy(*table_ptr, &resource_table, *length);
}

struct fw_rsc_vdev *rsc_table_to_vdev(void *rsc_table)
{
	return &((struct fw_resource_table *)rsc_table)->vdev;
}

struct fw_rsc_vdev_vring *rsc_table_get_vring0(void *rsc_table)
{
	return &((struct fw_resource_table *)rsc_table)->vring0;
}

struct fw_rsc_vdev_vring *rsc_table_get_vring1(void *rsc_table)
{
	return &((struct fw_resource_table *)rsc_table)->vring1;
}