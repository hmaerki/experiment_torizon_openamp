Caveat: The meta-toradex-torizon reference may not include out-of-the-box FreeRTOS/remoteproc support for the AM62P R5F. TI's own Yocto layer (meta-ti) or the MCU+ SDK may be needed alongside it. It's worth verifying remoteproc firmware loading works with the Torizon image before committing to that path.

Would you like me to update the "yocto/freertos" wording and/or add a note about the TI MCU+ SDK dependency?
