/*
* Base64 Encoder-Decoder-Scaner
* by masterLazy, 2025
*/
#include <iostream>
#include <vector>
#include <string>
#include <exception>
using namespace std;

#include "base64.hpp"

#include "cxxopts.hpp"

Base64 base64;
const string program_verson = "3.0";
const auto arg_invalid = cxxopts::exceptions::exception("Invalid arguments");
const auto arg_unmatch = cxxopts::exceptions::exception("No matching arguments");

void print_as_hex(const string& str) {
	for (unsigned char i : str) {
		printf_s("0x%02x ", i);
	}
	cout << endl;
}

int main(int argc, char* argv[]) {
	cxxopts::Options options("base64eds",
		"Encoding and decoding base64 strings or files, scanning base64 segments in files.");
	options.add_options("Decode/Encode")
		("e,encode", "Encode [INPUT] (string or file)", cxxopts::value<string>(), "[INPUT]")
		("d,decode", "Decode [INPUT] (string or file)", cxxopts::value<string>(), "[INPUT]")
		("f,file", "Treat [INPUT] as file path, and also output to a file.", cxxopts::value<bool>()->default_value("false"));
	options.add_options("Scan File")
		("s,scan", "Scan files for Base64 patterns", cxxopts::value<string>(), "[FILEPATH]");
	options.add_options("Other")
		("x,hex", "Print result on console as hexadecimal", cxxopts::value<bool>()->default_value("false"))
		("h,help", "Show this help message", cxxopts::value<bool>()->default_value("false"))
		("v,version", "Show version information", cxxopts::value<bool>()->default_value("false"));
	if (argc == 1) {
		cout << options.help() << endl;
		return 0;
	}
	try {
		auto res = options.parse(argc, argv);
		// 排除非法情况
		if (// 重复
			res.count("encode") > 1 or res.count("decode") > 1 or
			res.count("file") > 1 or res.count("scan") > 1 or
			// 互斥
			res.count("encode") and res.count("decode") or
			res.count("encode") and res.count("scan") or
			res.count("decode") and res.count("scan") or
			// 没选模式
			not(res.count("encode") or res.count("decode") or res.count("scan")) and res.count("file")) {
			throw arg_invalid;
		}
		// 非法但是可以忽略的情况
		if (res.count("scan") and res.count("file")) {
			cout << "WARNING: Ignoring invalid argument \"-file\"" << endl;
		}
		if (res.count("encode") or res.count("decode") or res.count("scan")) {
			if (res.count("help")) {
				cout << "WARNING: Ignoring invalid argument \"-help\"" << endl;
			}
			if (res.count("version")) {
				cout << "WARNING: Ignoring invalid argument \"-version\"" << endl;
			}
		}
		if ((res.count("file") or res.count("scan")) and res.count("hex")) {
			cout << "WARNING: Ignoring invalid argument \"-hex\"" << endl;
		}
		// 命令分支
		if (res.count("encode")) { // 编码
			if (res["file"].as<bool>()) {
				base64.encode_file(res["encode"].as<string>());
			} else {
				if (res["hex"].as<bool>()) {
					print_as_hex(base64.encode(res["encode"].as<string>()));
				} else {
					cout << base64.encode(res["encode"].as<string>()) << endl;
				}
			}
		} else if (res.count("decode")) { // 解码
			if (res["file"].as<bool>()) {
				base64.decode_file(res["decode"].as<string>());
			} else {
				if (res["hex"].as<bool>()) {
					print_as_hex(base64.decode(res["decode"].as<string>()));
				} else {
					cout << base64.decode(res["decode"].as<string>()) << endl;
				}
			}
		} else if (res.count("scan")) { // 扫描
			base64.scan_file(res["scan"].as<string>());
		} else if (res.count("help")) { // 帮助
			cout << options.help() << endl;
		} else if (res.count("version")) { // 版本
			cout << "Version " + program_verson << endl;
		} else {
			throw arg_unmatch;
		}
	} catch (const cxxopts::exceptions::exception& e) {
		cerr << "ERROR: " << e.what() << endl << options.help() << endl;
		return 1;
	} catch (const exception& e) {
		cerr << "ERROR: " << e.what() << endl;
		return 1;
	}
	return 0;
}