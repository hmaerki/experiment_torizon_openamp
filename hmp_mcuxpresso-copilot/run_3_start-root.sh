modprobe rpmsg_ns || true
modprobe rpmsg_ctrl || true
modprobe rpmsg_char || true
modprobe rpmsg-client-sample

mkdir -p /root/firmware
echo /root/firmware > /sys/module/firmware_class/parameters/path
cp build/mcux-example-minimal-cm7/rpmsg_demo_MIMX8ML8_cm7.elf /root/firmware
echo stop > /sys/class/remoteproc/remoteproc0/state
echo rpmsg_demo_MIMX8ML8_cm7.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state
