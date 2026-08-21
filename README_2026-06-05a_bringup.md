# Bringup 2026-06-05

In this document:

* Run Easy Installer
* Flash a yocto image

## Links

* [Mallow](https://developer.toradex.com/hardware/verdin-som-family/carrier-boards/mallow-carrier-board/)
* [Connectors](https://www.toradex.com/products/carrier-board/mallow-carrier-board#features)
* https://docs.toradex.com/113763-mallow-v1.1-assembly-drawing.zip
* https://developer.toradex.com/easy-installer/toradex-easy-installer/download-tezi

* 0063 Verdin iMX8M Plus Quad 4GB IT, V1.1C, S/N 08910183

* X9 2xUSB-A: Plug a USB keyboard/mouse into the Mallow Carrier Board's USB 3.x Host (X9) connector(s).

* X8 USB-C: Plug a USB Type-C to Type-A cable into the DRP (X8) connector to enter Recovery Mode and load the Toradex Easy Installer.
* SW2: Reset button
* SW1: Power button

## ETH

00:14:2D:87:F5:67
192.168.1.170

* VSCode 'Torizon IDE Extension'
* verdin-imx8mp-08910183
* connect, torizon/torizon
* ssh torizon@verdin-imx8mp-08910183.local


### Prepare USB stick (NOT REQUIRED)

* PC: Plug USB (format as ext4, label `torizon`)

```bash
$ sudo rm -r /media/maerki/torizon/*
$ ssh octoprobe@octoprobe4.local cat /home/octoprobe/work_beampilot/experiment_torizon_openamp/yocto-workdir/build-torizon/deploy/images/verdin-imx8mp/torizon-docker-verdin-imx8mp-Tezi.tar | sudo tar -C /media/maerki/torizon/ --strip-components=1 -xf -
$ eject /media/maerki/torizon
```

## USB Recovery Mode

### Server: Server image to be installed

See also [TEZI Server](tezi-server/tezi_server_README.md)

```bash
$ cd yocto-workdir
$ uv run https://raw.githubusercontent.com/yocto-openamp/torizon_openamp/main/tezi-server/tezi_server.py
```

### Mallow: Boot into Recovery Mode

* Power off
* Connect ETH
* Connect PC with Mallow-USB-C
* Connect: X16 pin1(GND) with pin2(CTRL_RECOVERY_MICO)
  * https://docs.toradex.com/117250-mallow_carrier_board_recovery_mode.mp4
  * [mallow loading-tezi ](https://developer.toradex.com/easy-installer/toradex-easy-installer/loading-toradex-easy-installer/?module=verdin_imx8mp&carrier=mallow#3-loading-tezi)
* Power on (or SW2/RST)
  ```text
  usb 3-7: new high-speed USB device number 31 using xhci_hcd
  usb 3-7: New USB device found, idVendor=1fc9, idProduct=0146, bcdDevice= 0.02
  usb 3-7: New USB device strings: Mfr=1, Product=2, SerialNumber=0
  usb 3-7: Product: SE Blank 865  
  usb 3-7: Manufacturer: NXP       SemiConductor Inc 
  hid-generic 0003:1FC9:0146.000E: hiddev1,hidraw5: USB HID v1.10 Device [NXP       SemiConductor Inc  SE Blank 865  ] on usb-0000:00:14.0-7/input0
  ```
* Disconnect: X16 pin1(GND) with pin2(CTRL_RECOVERY_MICO)



### PC: Toradex Easy Installer

```bash
$ lsusb
ID 1fc9:0146 NXP Semiconductors SE Blank 865  

$ mkdir -p /tmp/torizon; cd /tmp/torizon
$ wget https://tezi.toradex.com/artifactory/tezi-oe-prod-frankfurt/scarthgap-7.x.y/release/12/verdin-imx8mp/tezi/tezi-run/oedeploy/Verdin-iMX8MP_ToradexEasyInstaller_7.6.1+build.12.zip
$ unzip Verd*.zip
$ cd Verd*
$ ./recovery-linux.sh 
Downloading Toradex Easy Installer...
uuu (Universal Update Utility) for nxp imx chips -- libuuu_1.5.233-0-g79ce7d2
Success 1    Failure 0                                                
3:7-08910183 7/ 7 [Done                                  ] FB: done                                                               
3:7-120D2800 1/ 1 [=================100%=================] SDPS: boot -f ../imx-boot-recoverytezi                                 
Successfully downloaded Toradex Easy Installer.
```

```text
[10257.211090] usb 3-7: USB disconnect, device number 56
[10257.832127] usb 3-7: new high-speed USB device number 57 using xhci_hcd
[10257.956525] usb 3-7: New USB device found, idVendor=1b67, idProduct=4000, bcdDevice= 2.23
[10257.956536] usb 3-7: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[10257.956539] usb 3-7: Product: USB download gadget
[10257.956542] usb 3-7: Manufacturer: Toradex
[10257.956545] usb 3-7: SerialNumber: 08910183
[10261.667791] usb 3-7: USB disconnect, device number 57
[10266.685941] usb 3-7: new high-speed USB device number 58 using xhci_hcd
[10266.811732] usb 3-7: New USB device found, idVendor=1b67, idProduct=403f, bcdDevice= 0.01
[10266.811746] usb 3-7: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[10266.811751] usb 3-7: Product: 0063 Verdin iMX8M Plus Quad 4GB IT
[10266.811754] usb 3-7: Manufacturer: Toradex
[10266.811758] usb 3-7: SerialNumber: 08910183
[10266.844046] cdc_ncm 3-7:1.0: MAC-Address: da:68:ed:78:e9:d1
[10266.844515] cdc_ncm 3-7:1.0 usb0: register 'cdc_ncm' at usb-0000:00:14.0-7, CDC NCM (NO ZLP), da:68:ed:78:e9:d1
[10266.890889] cdc_ncm 3-7:1.0 enxda68ed78e9d1: renamed from usb0
```

* gvncviewer 192.168.11.1 
  * On VNC/monitor: Toradex Easy Installer
    * Feeds: http://octoprobe4.local:8000/
    * Feeds: http://maerki-dell-5550.local:8000/
    * Select Torizon OS -> Install -> Yes -> I Accept

### Connect with ssh

```bash
ssh-copy-id torizon@verdin-imx8mp-08910183.local
# PW: torizon
```

### Allow ssh without password root@verdin-imx8mp-08910183.local

```bash
ssh torizon@verdin-imx8mp-08910183.local
sudo cp -r ~torizon/.ssh ~root
# logout
# Test ssh to root
ssh root@verdin-imx8mp-08910183.local
```