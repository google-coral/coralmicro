Built from source code in `third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/bootloader_examples/flashloader`.
elftosb -f imx -V -c flashloader.bd -o ivt_flashloader.bin flashloader.srec
