# USB Mass Storage
This application allows mounting the on-board Dev Board Micro LittleFS file system on a Linux host system for read and write access.

Only Linux hosts are supported.

## Flash the app
After building, flash with

```bash
python3 scripts/flashtool.py --app usb_drive
```

When the app starts the Coral Dev Board Micro will enumerate as a USB Mass Storage Block Device, e.g:
```
$ dmesg

[1210529.489967] usb-storage 1-2.3.1:1.3: USB Mass Storage device detected
[1210529.491267] scsi host2: usb-storage 1-2.3.1:1.3
[1210530.515600] scsi 2:0:0:0: Direct-Access     CORAL    MASS STORAGE     0001 PQ: 0 ANSI: 4
[1210530.516376] sd 2:0:0:0: Attached scsi generic sg1 type 0
[1210530.516557] sd 2:0:0:0: [sdc] 32000 2048-byte logical blocks: (65.5 MB/62.5 MiB)
[1210530.517401] sd 2:0:0:0: [sdc] Write Protect is off
[1210530.517668] sd 2:0:0:0: [sdc] Mode Sense: 00 00 00 00
[1210530.525223] sd 2:0:0:0: [sdc] Attached SCSI removable disk

```
Note the assigned block device name, /dev/**sdc** in this example. The assigned device name can also be found with the help of the `lsblk` utility:
```
$ lsblk 

NAME                MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINTS
sda                   8:0    1     0B  0 disk  
sdc                   8:32   1  62.5M  0 disk  
nvme0n1             259:0    0 238.5G  0 disk 
```

## Build littlefs-fuse
To mount the file system writable a fork of littlefs-fuse is provided. `libfuse-dev` must be installed on the host system:

``` bash
sudo apt-get install libfuse-dev
```

 Build with:
```bash
make -C third_party/littlefs-fuse
```

## Mount the file system
First create a mount point in the host system, which initially is just a regular empty folder.
```bash
mkdir -p /path/to/mount/point
```
Then mount the file system using the built `lfs` binary, assigned block device name (`/dev/sdc` in this example) and `/path/to/mount/point`.

```bash
sudo third_party/littlefs-fuse/lfs -o allow_other --read_size=2048 \
     --cache_size=2048 --lookahead_size=2048 --prog_size=2048 \
     --block_cycles=250 --block_count=512 --block_size=131072 \
     /dev/sdc /path/to/mount/point
```

File system should now be mounted for both read and write access.
```bash
$ ls -l /path/to/mount/point

-rwxrwxrwx 0 root root 166308 Dec 31  1969 default.elf*
-rwxrwxrwx 0 root root     10 Dec 31  1969 usb_ip_address*
```

Unmount the file system when done:
```bash
sudo umount /path/to/mount/point
```
