#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <exception>
#include <cstdio>
#include <cmath>
using namespace std;

// mlib
#include "console/progressbar.hpp"
using namespace mlib::console;

class Base64Encoder {
	const char base64[64] = {
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
		'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
		'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
		'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
	};
	// 解码表
	const int8_t esab64[256] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
	};

	const exception file_invalid = exception("Failed to open file");
	const exception file_out = exception("Failed to create output file");
	const exception file_empty = exception("Input file is empty");
	const exception decode_len = exception("Invalid length");
	const exception decode_chr = exception("Invalid character in input string/file");

	// 经测试这个 buf 大小基本能拉平文件 I/O 的耗时了
	static const size_t enc_ibuf_size = 1024 * 300; // 必须是 3 的倍数
	static const size_t dec_ibuf_size = 1024 * 400; // 必须是 4 的倍数
	static const size_t enc_obuf_size = enc_ibuf_size / 3 * 4;
	static const size_t dec_obuf_size = dec_ibuf_size / 4 * 3;

	unsigned char enc_ibuf[enc_ibuf_size] = { 0 };
	char enc_obuf[enc_obuf_size] = { 0 };
	char dec_ibuf[dec_ibuf_size] = { 0 };
	unsigned char dec_obuf[dec_obuf_size] = { 0 };

	string open_output_file(FILE** fout, string filepath, const string& mode, bool encode) {
		string path;
		// 去掉后缀名
		if (!encode) {
			filepath = filepath.substr(0, filepath.find_last_of("."));
		}
		// 尝试 Plan A
		if (!fopen_s(fout,
			(path = filepath + (encode ? "_enc.txt" : "_dec")).c_str(),
			"r")) {
			fclose(*fout);
			cout << "File " << path << " already exists. Do you want to override? [y/n]" << endl;
			string s;
			cin >> s;
			if (s == "Y" or s == "y") {
				// Plan A
				if (!fopen_s(fout, path.c_str(), mode.c_str())) {
					return path;
				}
				throw file_out;
			} else if (s == "N" or s == "n") {
				// Plan B
				for (int i = 1; i < 1024; i++) {
					if (!fopen_s(fout,
						(path = filepath + (encode ? "_enc_" + to_string(i) + ".txt" : "_dec_" + to_string(i))).c_str(),
						mode.c_str())) {
						return path;
					}
				}
				throw file_out;
			} else {
				throw exception("Invalid input");
			}
		} else {
			if (!fopen_s(fout, path.c_str(), mode.c_str())) {
				return path;
			}
			throw file_out;
		}
		// 尝试 Plan B
		for (int i = 1; i < 1024; i++) {
			if (!fopen_s(fout,
				(path = filepath + (encode ? "_enc_" + to_string(i) + ".txt" : "_dec_" + to_string(i))).c_str(),
				"r")) {
				fclose(*fout);
				continue;
			}
			if (!fopen_s(fout, path.c_str(), mode.c_str())) {
				return path;
			} else {
				throw file_out;
			}
		}
		throw file_out;
	}
	size_t get_file_size(FILE* file) {
		size_t cur = _ftelli64(file);
		_fseeki64(file, 0, SEEK_END);
		size_t size = _ftelli64(file);
		_fseeki64(file, cur, SEEK_SET);
		return size;
	}
	// 直接编码到 enc_obuf (不检查越界!)
	size_t encode_to_buf(const unsigned char* str, size_t size) {
		if (!size) return 0;
		char* cur = enc_obuf;
		uint32_t temp_bytes;
		for (size_t i = 0; i < size; i += 3) {
			temp_bytes = 0;
			temp_bytes |= str[i] << 16;
			if (i + 1 < size) temp_bytes |= str[i + 1] << 8;
			if (i + 2 < size) temp_bytes |= str[i + 2];
			*cur++ = base64[temp_bytes >> 18];
			*cur++ = base64[(temp_bytes >> 12) & 0x3f];
			if (i + 3 > size and size % 3 == 1) {
				*cur++ = '=';
				*cur++ = '=';
			} else if (i + 3 > size and size % 3 == 2) {
				*cur++ = base64[(temp_bytes >> 6) & 0x3f];
				*cur++ = '=';
			} else {
				*cur++ = base64[(temp_bytes >> 6) & 0x3f];
				*cur++ = base64[temp_bytes & 0x3f];
			}
		}
		return cur - enc_obuf;
	}
	// 直接解码到 dec_obuf (不检查越界!)
	size_t decode_to_buf(const char* str, size_t size) {
		if (size % 4 != 0) {
			throw decode_len;
		}
		uint32_t temp_bytes;
		unsigned char* cur = dec_obuf;
		for (size_t i = 0; i < size; i += 4) {
			temp_bytes = 0;
			temp_bytes |= dec(str[i]) << 18;
			temp_bytes |= dec(str[i + 1]) << 12;
			if (str[i + 2] == '=') {
				if (str[i + 3] != '=' or i + 4 != size) {
					throw decode_chr;
				}
				*cur++ = temp_bytes >> 16;
				if (dec(str[i + 1]) & 0x0f) {
					*cur++ = (temp_bytes >> 8) & 0xff;
				}
				return cur - dec_obuf;
			}
			temp_bytes |= dec(str[i + 2]) << 6;
			if (str[i + 3] == '=') {
				if (i + 4 != size) {
					throw decode_chr;
				}
				*cur++ = temp_bytes >> 16;
				*cur++ = (temp_bytes >> 8) & 0xff;
				if (dec(str[i + 1]) & 0x03) {
					*cur++ = temp_bytes & 0xff;
				}
				return cur - dec_obuf;
			}
			temp_bytes |= dec(str[i + 3]);
			*cur++ = temp_bytes >> 16;
			*cur++ = (temp_bytes >> 8) & 0xff;
			*cur++ = temp_bytes & 0xff;
		}
		return cur - dec_obuf;
	}
	// 解码单个字符
	unsigned char dec(unsigned char ch) {
		if (esab64[ch] == -1) {
			throw decode_chr;
		}
		return esab64[ch];
	}
public:
	string encode(const string& str) {
		if (str.empty()) {
			return "";
		}
		string res, temp_str(4, '\0');
		uint32_t temp_bytes;
		for (size_t i = 0; i < str.size(); i += 3) {
			temp_bytes = 0;
			temp_bytes |= str[i] << 16;
			if (i + 1 < str.size()) temp_bytes |= str[i + 1] << 8;
			if (i + 2 < str.size()) temp_bytes |= str[i + 2];
			temp_str[0] = base64[temp_bytes >> 18];
			temp_str[1] = base64[(temp_bytes >> 12) & 0x3f];
			temp_str[2] = base64[(temp_bytes >> 6) & 0x3f];
			temp_str[3] = base64[temp_bytes & 0x3f];
			if (i + 3 > str.size()) {
				if (str.size() % 3 == 1) {
					temp_str[2] = temp_str[3] = '=';
				} else if (str.size() % 3 == 2) {
					temp_str[3] = '=';
				}
			}
			res += temp_str;
		}
		return res;
	}
	string decode(const string& str) {
		if (str.size() % 4 != 0) {
			throw decode_len;
		}
		string res, temp_str(3, '\0');
		uint32_t temp_bytes;
		for (size_t i = 0; i < str.size(); i += 4) {
			temp_bytes = 0;
			temp_bytes |= dec(str[i]) << 18;
			temp_bytes |= dec(str[i + 1]) << 12;
			if (str[i + 2] == '=') {
				if (str[i + 3] != '=' or i + 4 != str.size()) {
					throw decode_chr;
				}
				temp_str[0] = temp_bytes >> 16;
				if (dec(str[i + 1]) & 0x0f) {
					temp_str.resize(2);
					temp_str[1] = (temp_bytes >> 8) & 0xff;
				} else {
					temp_str.resize(1);
				}
				return res + temp_str;
			}
			temp_bytes |= dec(str[i + 2]) << 6;
			if (str[i + 3] == '=') {
				if (i + 4 != str.size()) {
					throw decode_chr;
				}
				temp_str[0] = temp_bytes >> 16;
				temp_str[1] = (temp_bytes >> 8) & 0xff;
				if (dec(str[i + 2]) & 0x03) {
					temp_str[2] = temp_bytes & 0xff;
				} else {
					temp_str.resize(2);
				}
				return res + temp_str;
			}
			temp_bytes |= dec(str[i + 3]);
			temp_str[0] = temp_bytes >> 16;
			temp_str[1] = (temp_bytes >> 8) & 0xff;
			if (dec(str[i + 1]) & 0x03) {
				temp_str[2] = temp_bytes & 0xff;
			}
			res += temp_str;
		}
		return res;
	}

	void encode_file(const string& filepath) {
		FILE* fin, * fout;
		if (fopen_s(&fin, filepath.c_str(), "rb")) {
			throw file_invalid;
		}
		string path = open_output_file(&fout, filepath, "wb", true);
		cout << "Writing encoding result to " + path << endl;
		size_t file_size = get_file_size(fin);
		if (!file_size) {
			throw file_empty;
		}
		size_t segments = ceil(float(file_size) / enc_ibuf_size);
		size_t size_read, size_encoded;
		clock_t last_print = 0;
		ProgressBar pb("Encoding file", file_size);
		pb.work_of_bytes(true);
		for (int i = 0; i < segments; i++) {
			size_read = fread_s(enc_ibuf, sizeof(enc_ibuf), sizeof(char), enc_ibuf_size, fin);
			size_encoded = encode_to_buf(enc_ibuf, size_read);
			fwrite(enc_obuf, sizeof(char), size_encoded, fout);
			pb.foward(size_read);
			if (clock() - last_print > CLOCKS_PER_SEC / 2) {
				pb.print();
				last_print = clock();
			}
		}
		pb.print();
		fclose(fout);
	}
	void decode_file(const string& filepath) {
		FILE* fin, * fout;
		if (fopen_s(&fin, filepath.c_str(), "rb")) {
			throw file_invalid;
		}
		string path = open_output_file(&fout, filepath, "wb", false);
		cout << "Writing decoding result to " + path << endl;
		size_t file_size = get_file_size(fin);
		if (!file_size) {
			throw file_empty;
		}
		size_t segments = ceil(float(file_size) / dec_ibuf_size);
		size_t size_read, size_decoded;
		clock_t last_print = 0;
		ProgressBar pb("Decoding file", file_size);
		pb.work_of_bytes(true);
		for (int i = 0; i < segments; i++) {
			size_read = fread_s(dec_ibuf, sizeof(dec_ibuf), sizeof(char), dec_ibuf_size, fin);
			size_decoded = decode_to_buf(dec_ibuf, size_read);
			fwrite(dec_obuf, sizeof(char), size_decoded, fout);
			pb.foward(size_read);
			if (clock() - last_print > CLOCKS_PER_SEC / 2) {
				pb.print();
				last_print = clock();
			}
		}
		pb.print();
		fclose(fout);
	}
};