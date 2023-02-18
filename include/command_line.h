#pragma once

#include <string>
#include <stdint.h>

#define CMD_MAX 1024
#define PASS_MAX 1024

struct PasswordPath {
    std::string name;
    std::string element;
};

enum class CommandType : uint8_t {
    INVALID,
    ADD,
    REMOVE,
    GET,
    COPY,
    LIST,
    HELP,
    WRITE,
    QUIT,
    WRITE_QUIT,
    __CMD_MAX
};

enum class CommandArgs : uint8_t {
    PATH_VAL,
    PATH_ONLY,
    OPT_PATH,
    NONE,
};

struct Command {
    std::string cmdStr;
    CommandType type;
    PasswordPath path;
    std::string value;
};

PasswordPath get_password_path(char *n);

char * get_token(char *&str, bool quote = true);

Command parse_command(char *str);

void get_password(char *password);

std::string escape(const char *str);

inline std::string escape(const std::string str) {
    return escape(str.c_str());
}

std::string unescape(const char *str);

inline std::string unescape(const std::string str) {
    return unescape(str.c_str());
}
