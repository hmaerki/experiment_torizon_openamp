modprobe rpmsg_ns
modprobe rpmsg_ctrl
modprobe rpmsg_char
# rpmsg-client-sample speaks a different protocol and crashes this Zephyr
# sample when it sends a payload that is not an aligned 32-bit counter.
modprobe -r rpmsg-client-sample 2>/dev/null || true

mkdir -p /root/firmware
echo stop > /sys/class/remoteproc/remoteproc0/state
echo /root/firmware > /sys/module/firmware_class/parameters/path
cp ./build/zephyr/rpmsg_service.elf /root/firmware
echo rpmsg_service.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state
