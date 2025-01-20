# Getting Started:

### Gather Items Needed (Requirements):
- Main computer: [RADXA ROCK5B](https://radxa.com/products/rock5/5b/#documentation)
- NVME SSD (any size)
  - _Optional: screw in using ? screw_
- MicroSD card (any size)
- USB Mouse & Keyboard
- Ethernet Cable & Connection
- HDMI Cable & Display
- USB-C cable with ≥30W Power Adapter

### Install System:
Refer to [RADXA official 'Getting Started' Page](https://docs.radxa.com/en/rock5/rock5b/getting-started).

Image Downloads: [Summary of resource downloads](https://docs.radxa.com/en/rock5/rock5b/download).

#### In short, 
- Use MicroSD card as a boot disk to flash the system’s SPI.
  - Download & Open [balenaEtcher](https://etcher.balena.io/#download-etcher) to burn the specific image you want to use (in this case Debian was used).
- Boot up your device and login - User: radxa | Password: radxa
- Copy the Image file from the MicroSD card into the NVME. Refer to [this page](https://docs.radxa.com/en/rock5/rock5b/getting-started/install-os/nvme).

If done successfully, power off the system, remove the MicroSD card, and the system should boot from the NVME drive.

#### Troubleshooting tips:
- Run `lsblk` in the command line, and it should output something like this if steps were done correctly:
```
NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
mtdblock0    31:0    0    16M  0 disk 
zram0       254:0    0   3.9G  0 disk [SWAP]
nvme0n1     259:0    0 238.5G  0 disk 
├─nvme0n1p1 259:1    0    16M  0 part /config
├─nvme0n1p2 259:2    0   300M  0 part /boot/efi
└─nvme0n1p3 259:3    0 238.2G  0 part /
```
  - Names `mmcblkXpY` are for SD card partitions | `nvmeXbYpZ` for NVME SSD partitions.
  - The device with `/boot/efi` under the MOUNTPOINT column is your root filesystem and boot device.

- Make sure to use the correct path when copying the system image onto the NVME and replace `radxa@192.167.2.66` with the correct username and IP address for you ROCK 5B board.
  - You can find your IP address of your board by running `ip addr show` on the command line.
    - This will display the network interfaces and their IP addresses. Look `inet` followed by an address like `192.167.x.x` or `127.x.x.x`.
   
### Setting up the LIDAR:

### Setting Up the Camera

### Configure the PWM pins:



