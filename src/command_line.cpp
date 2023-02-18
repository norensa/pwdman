#include <command_line.h>
#include <vector>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

const struct {
    CommandType type;
    CommandArgs args;
    std::vector<std::string> str;
} COMMAND[] = {
    {
        CommandType::INVALID,
        CommandArgs::NONE,
        { }
    },
    {
        CommandType::ADD,
        CommandArgs::PATH_VAL,
        { "add", "a" }
    },
    {
        CommandType::REMOVE,
        CommandArgs::PATH_ONLY,
        { "remove", "r" }
    },
    {
        CommandType::GET,
        CommandArgs::PATH_ONLY,
        { "get", "g" }
    },
    {
        CommandType::COPY,
        CommandArgs::PATH_ONLY,
        { "copy", "c" }
    },
    {
        CommandType::LIST,
        CommandArgs::OPT_PATH,
        { "list", "l" }
    },
    {
        CommandType::HELP,
        CommandArgs::NONE,
        { "help", "h" }
    },
    {
        CommandType::WRITE,
        CommandArgs::NONE,
        { "write", "w" }
    },
    {
        CommandType::QUIT,
        CommandArgs::NONE,
        { "quit", "q", "exit" }
    },
    {
        CommandType::WRITE_QUIT,
        CommandArgs::NONE,
        { "wq" }
    },
};

PasswordPath get_password_path(char *n) {
    PasswordPath p;
    char *ptr = strrchr(n, '.');
    if (ptr) {
        *ptr = '\0';
        p.name = n;
        p.element = ptr + 1;
    }
    else {
        p.name = n;
        p.element.clear();
    }
    return p;
}

char * get_token(char *&str, bool quote) {
    char *tok = nullptr, stop, *p;

    while (*str == ' ') ++str;

    tok = str;

    if (*str) {
        if (quote && (*str == '\'' || *str == '\"')) {
            ++tok;
            stop = *str;
            ++str;
            while (*str && *str != stop) ++str;
            if (*str) {
                *str = '\0';
                ++str;
            }
            else {
                printf("Invalid command; expected token %c\n", stop);
                return nullptr;
            }
        }
        else {
            do {
                if (*str && *str == '\\') {
                    p = str;
                    do *p = *(p + 1); while (*++p);
                    if (*str && *str == ' ') ++str;
                }
                while (*str && *str != ' ' &&  *str != '\\') ++str;
            } while (*str && *str == '\\');
            if (*str) {
                *str = '\0';
                ++str;
            }
        }
    }

    return tok;
}

Command parse_command(char *str) {
    char *token;
    Command cmd;

    // save command str
    cmd.cmdStr = str;

    // get command
    cmd.type = CommandType::INVALID;
    token = get_token(str, false);
    if (token == nullptr || *token == '\0') {
        return cmd;
    }
    for (auto &c : COMMAND) {
        for (auto &x : c.str) {
            if (strcasecmp(token, x.c_str()) == 0) {
                cmd.type = static_cast<CommandType>(c.type);
            }
        }
    }
    if (cmd.type == CommandType::INVALID) {
        printf("Invalid command '%s'\n", token);
        return cmd;
    }

    // get path, if found
    if ((token = get_token(str)) == nullptr) {
        cmd.type = CommandType::INVALID;
        return cmd;
    }
    if (*token) {
        cmd.path = get_password_path(token);
    }
    else {
        cmd.path.name.clear();
        cmd.path.element.clear();
    }

    // get value, if found
    if ((token = get_token(str)) == nullptr) {
        cmd.type = CommandType::INVALID;
        return cmd;
    }
    if (*token) {
        cmd.value = token;
    }
    else {
        cmd.value.clear();
    }

    // validate number of arguments
    if (
        *str
        || (
            COMMAND[static_cast<size_t>(cmd.type)].args == CommandArgs::PATH_VAL
            && (cmd.path.name.empty() || cmd.value.empty())
        )
        || (
            COMMAND[static_cast<size_t>(cmd.type)].args == CommandArgs::PATH_ONLY
            && (cmd.path.name.empty() || ! cmd.value.empty())
        )
        || (
            COMMAND[static_cast<size_t>(cmd.type)].args == CommandArgs::OPT_PATH
            && (! cmd.value.empty())
        )
        || (
            COMMAND[static_cast<size_t>(cmd.type)].args == CommandArgs::NONE
            && (! cmd.path.name.empty() || ! cmd.value.empty())
        )
    ) {
        printf(
            "Invalid number of arguments given to command '%s'. "
            "Type 'h' or 'help' for command help.\n",
            COMMAND[static_cast<size_t>(cmd.type)].str[0].c_str()
        );
        cmd.type = CommandType::INVALID;
    }

    return cmd;
}

void get_password(char *password) {
    static struct termios oldt, newt;
    int i = 0;
    int c;

    /*saving the old settings of STDIN_FILENO and copy settings for resetting*/
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    /*setting the approriate bit in the termios struct*/
    newt.c_lflag &= ~(ECHO);

    /*setting the new bits*/
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    /*reading the password from the console*/
    while ((c = getchar())!= '\n' && c != EOF && i < PASS_MAX) {
        password[i++] = c;
    }
    password[i] = '\0';
    printf("\n");

    /*resetting our old STDIN_FILENO*/
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

std::string escape(const char *str) {
    char *escaped = (char *) malloc(2 * strlen(str) + 1);
    char *str2 = escaped;

    while (*str) {
        if (*str == '\\') ++str;
        if (*str == ' ') *str2++ = '\\';
        *str2++ = *str++;
    }
    *str2 = '\0';

    std::string s(escaped);
    free(escaped);
    return s;
}

std::string unescape(const char *str) {
    char *unescaped = (char *) malloc(strlen(str) + 1);
    char *str2 = unescaped;

    while (*str) {
        if (*str == '\\') str++;
        *str2++ = *str++;
    }
    *str2 = '\0';

    std::string s(unescaped);
    free(unescaped);
    return s;
}
