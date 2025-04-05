# Base64 Encoder-Decoder-Scaner

```
Encoding and decoding base64 strings or files, scanning base64 segments in files.
Usage:
  base64eds [OPTION...]

 Decode/Encode options:
  -e, --encode [INPUT]  Encode [INPUT] (string or file)
  -d, --decode [INPUT]  Decode [INPUT] (string or file)
  -f, --file            Treat [INPUT] as file path, and also output to a
                        file.

 Scan File options:
  -s, --scan [FILEPATH]  Scan files for Base64 patterns

 Other options:
  -x, --hex      Print result on console as hexadecimal
  -h, --help     Show this help message
  -v, --version  Show version information
```