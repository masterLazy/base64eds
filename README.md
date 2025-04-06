# ✨ Base64EDS

### Base64 Encoder-Decoder-Scanner | Base64 编码器-解码器-扫描器

支持下列功能：

- 编码和解码字符串
- 编码和解码文件
- 扫描文件中的 base64 编码片段

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

## 🔥 性能

Base64EDS 在文件编解码上做了**极致优化**，编码约 **500MiB/s**，解码约 **700MiB/s**🔥。下面是在 32GiB 文件上的测试：

```
D:\>base64eds -e test.bin -f
Writing encoding result to test.bin.b64
Encoding file 100%|███████| 32.00GiB/32.00GiB [01:03<00:00, 513.86MiB/s]
D:\>base64eds -d test.bin.b64 -f
File test.bin already exists. Do you want to override? [y/n]
n
Writing decoding result to test.bin_1
Decoding file 100%|███████| 42.67GiB/42.67GiB [00:58<00:00, 745.91MiB/s]
```

（机器参数：Intel i7-6700@3.40GHz，32GB RAM）

## ⚙️ 如何编译

把 `mlib/include/` 添加到包含目录，添加 `main.cpp` 到源文件，然后编译即可。（开启 `O2` 优化以达到最佳性能）


## 🗂 第三方代码声明

本项目包含以下第三方代码，其版权归属原作者。

- [cssopts](https://github.com/jarro2783/cxxopts)，许可证文件在 `cxxopts_license.txt`。