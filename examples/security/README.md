# A71CH Secure Element example

- Outputs A71CH Unique ID
- Generates some random data
- Prints out the uncompressed EC256 public key
- Checksum of models/testconv1-edgetpu.tflite
- Signed checksum of models/testconv1-edgetpu.tflite

To build and flash from coralmicro root:

```
bash build.sh
python3 scripts/flashtool.py -e security
```

## Example output from the application (this will be different on each device)

```
A71 Unique ID: 479070024791121002180012423063824812
A71 Random 8 Bytes: b4a3054723f73320
A71 Random 16 Bytes: 6ff97a2cf5e70fd71a867875515b6d57
A71 Random 32 Bytes: 28fda7f43956937043e56cdfda84321c61749e817f5b1982d5cb4ec574251c31
A71 Random 64 Bytes: 4d01a3d3c6fc1837efbd3f51644a5b280301fc510fb778ca3dde0c667e4490f292e82ea1d3b4006dcff68d7431e6cbd775bd90d15de99fcf324a874a4f3a32d7
A71 ECC public key 0: 04a35e5482132be6a6af3fd76885e062545801f331bb6f0c0e7cbb7b920e94719b62812925b11f7335e1c35b2d187f093dd38381287dfaea914ae248e4d21fe8ed
testconv1-edgetpu.tflite sha: 20735a163b0a4835e42157e88b60440b566db21a4accfac49bd002998e9e069f
Signature: 3046022100b1a1b761f642998228ad04972bc811fb25be47a1e08fa5db6bea1e6b25b995100221008c23b40e17ad57cbdc3fc4d9a2c3f8ac36f8d7e77b8a30b341c0d4b5ac7979fd
Ecc Verify: success
EccVerifyWithKey: success

See examples/security/README.md for instruction to verify.
```

## Getting a public key file for desktop use (replace key string with the string from your device)

```
bash examples/security/pubkey.sh 04b5dba65692068705adcc2aa090267bd33c3d9f3f3cc4f31492f3a967effa5bc85ca8ebe29c4d4e9feb49a771c221f350cfbc7fa745c5c6018e055b3421a9aca6 > pubkey.pem
```

## Verify signature (replace signature with string from your device)

```
echo '3045022046e163fdfa889ff781fd6df7ae248a8be46573a29eb4f6adedfa5cc765b9f447022100a80e0b8731971e0df56caddcaa4b42cea4c4c1f23c91eabf0208d88b67abb15a' > testconv1-edgetpu.tflite.sig.hex

xxd -r -p testconv1-edgetpu.tflite.sig.hex testconv1-edgetpu.tflite.sig.bin

openssl dgst -sha256 -verify pubkey.pem -signature testconv1-edgetpu.tflite.sig.bin models/testconv1-edgetpu.tflite
```
