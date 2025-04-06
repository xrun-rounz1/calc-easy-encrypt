#include <iostream>
#include <string>
#include <iomanip>
#include <exception>
#include <string_view>
#include <limits>
#include <sqlite3.h>

#define PARAMS_VERSION 1

#define GETKEY_SQL                                                     \
    "select sum("                                                      \
    "    cast(strftime('%Y', uploaded) as integer) +"               \
    "        ("                                                        \
    "            cast(strftime('%m', uploaded) as integer) * 100 +" \
    "            cast(strftime('%d', uploaded) as integer)"         \
    "        )) as k"                                                  \
    " from video"                                                      \
    " where id in (6529016, 9621047, 12050471);"


int get_key() {
    sqlite3 *db = nullptr;
    int result = -1;
    if(sqlite3_open_v2("./video.db", &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        return -1;
    }
    sqlite3_stmt *stmt = nullptr;
    int ret = sqlite3_prepare_v2(db, GETKEY_SQL, -1, &stmt, nullptr);
    if (ret == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        } else {
            result = -1;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return result;
}

int get_utf8_encode_size(unsigned int u) {
    if (u > 0xffff && u <= 0x1fffff) {
        return 4;
    } else if(u > 0x07ff) {
        return 3;
    } else if(u > 0x7f) {
        return 2;
    } else {
        return 1;
    }
    return 0;
}

int utf8_encode(unsigned int u, std::string::iterator it, std::string::iterator end) {
    if (u > 0xffff && u <= 0x1fffff && std::distance(it, end) >= 4) {
        *(it + 3) = 0x80 | u & 0x3f;
        *(it + 2) = 0x80 | (u >> 6) & 0x3f;
        *(it + 1) = 0x80 | (u >> 12) & 0x3f;
        *(it + 0) = 0xf0 | (u >> 16) & 0x07;
        return 4;
    } else if (u > 0x07ff && std::distance(it, end) >= 3) {
        *(it + 2) = 0x80 | u & 0x3f;
        *(it + 1) = 0x80 | (u >> 6) & 0x3f;
        *(it + 0) = 0xe0 | (u >> 12) & 0x0f;
        return 3;
    } else if (u > 0x7f && std::distance(it, end) >= 2) {
        *(it + 1) = 0x80 | u & 0x3f;
        *(it + 0) = 0xc0 | (u >> 6) & 0x1f;
        return 2;
    } else if (std::distance(it, end) >= 1) {
        *(it + 0) = u;
        return 1;
    }
    return 0;
}

unsigned int parse_params(int argc, char *argv[]) {
    unsigned int params_flag = 0;
    for(int i = 1; i < argc; i++) {
        std::string_view sv{argv[i]};
        if(sv == "--version"){
            params_flag |= PARAMS_VERSION;
        } else {
            throw std::runtime_error(std::string("invalid parameter: ") + sv.data());
        }
    }
    return params_flag;
}

void print_version() {
    std::cout << "Vocal: こめはっぱ\n";
    std::cout << "         https://x.com/Komehappa_Shake\n";
    std::cout << "         https://www.youtube.com/@komehappa\n";
    std::cout << "Mix: 隆也(たかや)\n";
    std::cout << "         https://x.com/T5SC_GD\n";
    std::cout << "Illust: さくら ゆん\n";
    std::cout << "         https://x.com/yunsakura_\n";
    std::cout << "Movie: 玄月 ゆん\n";
    std::cout << "         https://x.com/kurozuki1608\n";
    std::cout << "\n";
    std::cout << "Original: Calc. ジミーサムP\n";
    std::cout << "         https://www.nicovideo.jp/watch/sm12050471\n";
    std::cout << "         https://www.nicovideo.jp/user/7032706/mylist/8538985\n";
    std::cout << "\n";
    std::cout << "calc-easy-decrypt version 1.0.0 (2025-04-06)\n";
    std::cout << "Developed by Mobu Ronzu(@ro_rounz1)\n";
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    // 引数パース
    unsigned int params = 0;
    try {
        params = parse_params(argc, argv);
    } catch(std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    if(params & PARAMS_VERSION) {
        // クレジット出力
        print_version();
        return 0;
    }

    // ======================================
    // decrypt
    int key = get_key();
    if(key == -1) {
        std::cerr << "failed generate key" << std::endl;
        return 1;
    }

    int byte;
    int point = 0;
    char charactor;
    std::string bytestr, origstr;

    while((byte = std::cin.get()) != EOF) {
        charactor = std::char_traits<char>::to_char_type(byte);
        if (charactor != 0x20 && charactor != '\n') {
            bytestr += charactor;
            continue;
        }
        if(!bytestr.empty()) {
            unsigned int codepoint;
            try {
                long value = std::stol(bytestr, nullptr, 16);
                if(value > std::numeric_limits<unsigned int>::max()) {
                    throw std::out_of_range("Integer overflow");
                }
                codepoint = static_cast<unsigned int>(value);
            } catch(std::invalid_argument) {
                std::cerr << "bad value" << std::endl;
                return 1;
            } catch(std::out_of_range) {
                std::cerr << "Integer overflow..." << std::endl;
                return 1;
            }
            codepoint = codepoint ^ key;
            int size = get_utf8_encode_size(codepoint);
            origstr.resize(origstr.size() + size);
            utf8_encode(codepoint, origstr.begin() + point, origstr.end());
            point += size;
        }
        if (charactor == '\n' || bytestr.empty()) {
            origstr += charactor;
            point++;
        }
        bytestr = "";
    }
    std::cout << origstr;
    return 0;
}
