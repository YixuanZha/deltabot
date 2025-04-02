# Getting Started:

### Items Needed (Requirements):
- Main computer: [RADXA ROCK5B](https://radxa.com/products/rock5/5b/#documentation)
- NVME SSD (any size)
- MicroSD card (any size)
- USB Mouse & Keyboard
- Ethernet Cable & Connection
- HDMI Cable & Display
- USB-C cable with ≥30W Power Adapter

## Installing the System:
Refer to [RADXA official 'Getting Started' Page](https://docs.radxa.com/en/rock5/rock5b/getting-started).

Image Downloads: [Summary of resource downloads](https://docs.radxa.com/en/rock5/rock5b/download).

### In short, 
- Use a MicroSD card as a boot disk to flash the system’s SPI.
  - Download & Open [balenaEtcher](https://etcher.balena.io/#download-etcher) to burn the specific image you want to use (in this case Debian was used).
- Boot up your device and login - User: radxa | Password: radxa
- Copy the Image file from the MicroSD card into the NVME. Refer to [this page](https://docs.radxa.com/en/rock5/rock5b/getting-started/install-os/nvme).

If done successfully, power off the system, remove the MicroSD card, and the system should boot from the NVME drive.

### Troubleshooting tips:
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

## Configure the PWM pins:

The ROCK5B exposes multiple PWM-capable pins. First enable the PWM controller and channel via the sysfs interface.

### Step 1: Activate Pins Using `r-setup`

[`r-setup`]([https://github.com/radxa/rsetup](https://docs.radxa.com/en/rock5/rock5c/radxa-os/rsetup)) is a utility by Radxa to simplify GPIO, I2C, SPI, and PWM configuration. Use it to configure pins to their correct function.

For example, to configure **Pin 18 (PWM5_M2)** for PWM output:

### Step 2: Export & Set PWM

export the correct PWM channel using sysfs. For PWM5_M2, it is pwmchip5, channel 0:

`echo 0 | sudo tee /sys/class/pwm/pwmchip5/export`

`ls /sys/class/pwm/pwmchipX/pwmY` to list activated pins and check if export worked.

pwmchipX → The PWM controller (varies based on hardware)
pwmY → The PWM channel within that controller

Let's use Pin 18, ie. PWM5_M2 for example:

`echo 0 | tee /sys/class/pwm/pwmchip5/export`

You can manually set the PWM signal by

```
echo 20000000 | tee /sys/class/pwm/pwmchip5/pwm0/period
echo 1500000 | tee /sys/class/pwm/pwmchip5/pwm0/duty_cycle
echo 1 | tee /sys/class/pwm/pwmchip5/pwm0/enable
```

#### Note:
- Parallax Continuous Rotation Servo Motors require a pulse range of 1.3-1.5ns and a 20ms pause between every pulse:
• A pulse duration of 1.5 ms corresponds to a neutral position.
• A pulse duration below 1.5 ms induces clockwise rotation
• A pulse duration above 1.5 ms, induces counterclockwise rotation

### Step 3: C++ Integration
All the code scripts are found in [scripts](scripts)

Here is a simple example of how PWM configuration would work:
```

```

## Setting up the LIDAR:

## Setting Up the Camera:



