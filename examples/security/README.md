# A71CH Sample application
- Outputs A71CH Unique ID
- Generates some random data
- Prints out the uncompressed EC256 public key
- Checksum of models/testconv1-edgetpu.tflite
- Signed checksum of models/testconv1-edgetpu.tflite

## Example output from the application (this will be different on each device)

```
A71 Unique ID: 479070024791121002180013003063824812
A71 Random bytes: f644a375946aa7771fedad33570b2a05
A71 public key 0: 04b5dba65692068705adcc2aa090267bd33c3d9f3f3cc4f31492f3a967effa5bc85ca8ebe29c4d4e9feb49a771c221f350cfbc7fa745c5c6018e055b3421a9aca6
SHA256: 20735a163b0a4835e42157e88b60440b566db21a4accfac49bd002998e9e069f
Signature: 3045022046e163fdfa889ff781fd6df7ae248a8be46573a29eb4f6adedfa5cc765b9f447022100a80e0b8731971e0df56caddcaa4b42cea4c4c1f23c91eabf0208d88b67abb15a
```

## Getting a public key file for desktop use (replace key string with the string from your device)
```
bash examples/a71ch/pubkey.sh 04b5dba65692068705adcc2aa090267bd33c3d9f3f3cc4f31492f3a967effa5bc85ca8ebe29c4d4e9feb49a771c221f350cfbc7fa745c5c6018e055b3421a9aca6 > pubkey.pem
```

## Verify signature (replace signature with string from your device)
```
echo '3045022046e163fdfa889ff781fd6df7ae248a8be46573a29eb4f6adedfa5cc765b9f447022100a80e0b8731971e0df56caddcaa4b42cea4c4c1f23c91eabf0208d88b67abb15a' > testconv1-edgetpu.tflite.sig.hex

xxd -r -p testconv1-edgetpu.tflite.sig.hex testconv1-edgetpu.tflite.sig.bin

openssl dgst -sha256 -verify pubkey.pem -signature testconv1-edgetpu.tflite.sig.bin models/testconv1-edgetpu.tflite
```