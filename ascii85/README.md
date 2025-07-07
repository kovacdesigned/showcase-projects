A simple command-line tool to encode or decode data using ASCII85 encoding.

## Usage
    ./ascii85 [-e | -d] 

- e or no argument: encode data from stdin to ASCII85.
- d: decode ASCII85 data from stdin to raw binary.

Any other usage prints help and exits with failure.

## Examples
- Encode a file to ASCII85:
    ./ascii85 -e < input.bin > output.txt

- Decode an ASCII85-encoded file back to binary:
    ./ascii85 -d < output.txt > decoded.bin
