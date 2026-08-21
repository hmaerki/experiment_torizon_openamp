# Second ETH

## Collected info

[Toradex, Second ETH](https://developer.toradex.com/software/linux-resources/connectivity/network-on-toradex-computer-on-modules/?utm_source=chatgpt.com#second-ethernet-on-toradex-carrier-boards)

The i.MX 8M Plus has a second Gigabit Ethernet controller exposed as RGMII. Toradex specifically documents using that interface with an external Ethernet PHY on the carrier board.

**Mallow does not provide that second PHY/RJ45 interface.**

The key fact is that Mallow's X17 is M.2 Key B with one PCIe lane, and Toradex confirms it can be used for PCIe/NVMe devices.

Don't search for a "one-notch M-key Ethernet card." Search for:

*M.2 B+M key PCIe x1 Ethernet I225* or *M.2 B key 2.5GbE I225*

## Links

* https://community.toradex.com/t/mallow-carrier-board-and-pcie-m-2-key-b/20530

## Cards

### Ableconn M2NW108BM — Intel I225, 2.5 GbE

https://ableconn.com/products_2.php?gid=172

2.5GbE

M.2 B+M-key card, PCIe ×1, with an RJ45 connector, based on the Intel I225. The manufacturer explicitly says it works in M.2 B-Key or M-Key sockets.

Risk: Single Print

### Ableconn/Lycom NW-107BM — 1 GbE

https://www.lycom.com.tw/NW-107BM.htm

M.2 B-M key, PCIe ×1 Ethernet module

### IOCrest IO-M2F225-GLAN — Intel I225, 2.5 GbE

M.2 B-Key/M-Key → 2.5G Ethernet, using Intel I225

### Advantech PCM-34R1TP-AE

https://www.integral-system.fr/products/1-port-gigabit-ethernet-intel-174-i225-2-5gb-s-ieee-1588-tsn-1x-rj45-b-m-key-3042-pciex1-pcm-34r1tp-ae

https://www.integral-system.fr/media/product/file/09196237-E1E4-4EEA-8DFB-8951616485BE.pdf

CHF77


### Delock M.2 Key B+M 2.5GbE Ethernet

https://www.digitec.ch/de/s1/product/delock-konverter-m2-key-bm-stecker-zu-1-x-rj45-25-gigabit-lan-port-horizontal-pcie-20-netzwerkkarte-32987734

CHF 42.59

**too long!**

### Ebay 2.5G M.2 B+M Key PCIE NVME Ethernet Card 2500M RJ45 LAN Card Intel I225 Chipset

https://www.ebay.com/itm/316249066533

USD 30

### DfRobot FIT1005

**Stock: 0** (Mouser&Digikey), (1 left https://www.dfrobot.com/search-fit1005.html)

https://www.mouser.ch/en/ProductDetail/DFRobot/FIT1005?qs=jcD%2FCkGBYeOGKfJ9FwXBAA%3D%3D

Ethernet Modules M.2 (B+M Key) to 2.5G Ethernet

CHF 20

### https://de.aliexpress.com/item/1005009824703376.html

CHF 45

M.2 B+M I226-T2 Industrial Grade 2.5G Gigabit Ethernet Card PCIE Dual Electrical Port Network Card

**probably too long**

### https://de.aliexpress.com/item/1005006567773303.html

CHF 25

M.2 to Dual Port 2.5G Ethernet NIC Network Card M.2 22*80mm Size B Key and M Key 2500 Mbps RTL8125B Chipset

**too long**

### [Delock 95272](https://www.delock.de/produkt/95272/merkmale.html)


Intel i225V
M.2 2242 B+M Stecker

2.5GB

CHF40 https://www.conrad.ch/de/p/delock-95272-m-2-controller-rj45-seriell-3379284.html

CHF47 https://www.digitec.ch/de/s1/product/delock-konverter-m2-key-bm-1-x-rj45-25-gigabit-lan-netzwerkkarte-31701823
==> SELECTED

CHF81 https://www.reichelt.com/ch/de/shop/produkt/netzwerkkarte_m_2_2_5_gigabit_ethernet_1x_rj45-335262

### [Delock 95274](https://www.delock.de/produkt/95274/merkmale.html)

https://www.digitec.ch/de/s1/product/delock-m2-key-bm-1-x-rj45-netzwerkkarte-42616779

Delock M.2 Key B+M 1 x RJ45

10GB

**too long**

### https://www.delock.de/produkt/62851/merkmale.html

M.2 2242, Realtek RTL8111

1G

