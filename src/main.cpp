/*
 * Copyright (c) 2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <password_store.h>
#include <command_line.h>
#include <file.h>
#include <stdio.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <libclip/clip.h>

PasswordStore *store = nullptr;
File *passFile = nullptr;

bool initialize_password_store() {

    if (passFile == nullptr) {
        passFile = new File(
            Path(getpwuid(getuid())->pw_dir).append(".pwdman")
        );
    }

    if (passFile->exists()) {
        printf(
            "Reading password file '%s'\n",
            passFile->info().path().get()
        );

        char password[PASS_MAX + 1];
        printf("\nPassword: ");
        get_password(password);

        store = new PasswordStore(password);

        try {
            InputFileSerializer(*passFile) >> *store;
        }
        catch (const Error &e) {
            printf("%s\n\n", e.what());
            return false;
        }

        return true;
    }
    else {
        printf(
            "Password file '%s' not found; initializing empty password store.\n",
            passFile->info().path().get()
        );

        char password[PASS_MAX + 1], confirm[PASS_MAX + 1];

        printf("\nPassword: ");
        get_password(password);
        printf("Confirm : ");
        get_password(confirm);

        while (strcmp(password, confirm)) {
            printf("Password mismatch. Try again\n");
            printf("\nPassword: ");
            get_password(password);
            printf("Confirm : ");
            get_password(confirm);
        }

        store = new PasswordStore(password);

        return true;
    }
}

void save_password_store() {
    passFile->close();
    passFile->open(File::READ_WRITE | File::CREATE | File::TRUNCATE, 0600);
    (OutputFileSerializer(*passFile) << *store).flush();
    passFile->close();
}

void print_cmd_help(const char *err = nullptr) {

    if (err) printf("%s\n", err);

    printf(
        "\n"
        "Usage:\n"
        "    (a)dd       <name> <password> : add/overwrite a stored password\n"
        "    (c)opy      <name>            : copy a stored password to clipboard\n"
        "    (g)et       <name>            : get a stored password\n"
        "    (l)ist                        : list all stored passwords\n"
        "    (r)emove    <name>            : remove a stored password\n"
        "    (w)rite                       : write changes to password file\n"
        "    (h)elp                        : show this help\n"
        "    (q)uit|exit                   : terminate\n"
        "\n"
    );
}

char * completion_generator(const char *text, int state) {
    static size_t i;
    static std::vector<std::string> suggestions;

    size_t len;

    if (state) {
        ++i;
    }
    else {
        i = 0;
        len = strlen(text);
        suggestions.clear();
    }

    if (len < 2) return nullptr;

    if (i == 0) {
        if (strrchr(text, '.')) {
            auto n = std::string(text);
            n = n.substr(0, n.rfind("."));
            auto n_unescaped = unescape(n);
            if (store->passwords().contains(n_unescaped)) {
                for (const auto &x: store->passwords()[n_unescaped]) {
                    auto s = n + '.' + x.k;
                    if (! rl_completion_quote_character) s = escape(s);
                    if (strncmp(s.c_str(), text, len) == 0) suggestions.push_back(s);
                }
            }
        }
        else {
            for (auto s : store->list()) {
                if (! rl_completion_quote_character) s = escape(s);
                if (strncmp(s.c_str(), text, len) == 0) suggestions.push_back(s);
            }
        }
    }

    return (i < suggestions.size()) ? strdup(suggestions[i].c_str()) : nullptr;
}

char ** completion_func(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, completion_generator);
}

int quote_detector(char *line, int index) {
    return (
        index > 0 &&
        line[index - 1] == '\\' &&
        !quote_detector(line, index - 1)
    );
}

void command_line() {
    rl_attempted_completion_function = completion_func;
    rl_completer_quote_characters = "\"'";
    rl_completer_word_break_characters = strdup(" ");
    rl_char_is_quoted_p = quote_detector;

    char *str;

    while (true) {
        str = readline("\n>> ");
        Command cmd = parse_command(str);
        free(str);

        switch (cmd.type) {
        case CommandType::ADD:
            if (cmd.path.element.empty()) cmd.path.element = "default";
            store->passwords()[cmd.path.name][cmd.path.element] = cmd.value;
        break;

        case CommandType::REMOVE:
            add_history(cmd.cmdStr.c_str());

            if (store->passwords().contains(cmd.path.name)) {
                if (cmd.path.element.empty()) {
                    store->passwords().erase(cmd.path.name);

                    printf("'%s' removed\n", cmd.path.name.c_str());
                }
                else if (store->passwords()[cmd.path.name].erase(cmd.path.element)) {
                    if (store->passwords()[cmd.path.name].empty()) {
                        store->passwords().erase(cmd.path.name);
                        printf("'%s' removed\n", cmd.path.name.c_str());
                    }
                    else {
                        printf("'%s.%s' removed\n", cmd.path.name.c_str(), cmd.path.element.c_str());
                    }

                }
                else {
                    printf("'%s.%s' not found\n", cmd.path.name.c_str(), cmd.path.element.c_str());
                }
            }
            else {
                printf("'%s' not found\n", cmd.path.name.c_str());
            }
        break;

        case CommandType::GET:
            add_history(cmd.cmdStr.c_str());

            if (store->passwords().contains(cmd.path.name)) {
                if (cmd.path.element.empty()) {
                    printf("%s: {\n", cmd.path.name.c_str());
                    if (store->passwords()[cmd.path.name].contains("default")) {
                        printf(
                            "    default: %s\n",
                            store->passwords()[cmd.path.name]["default"].c_str()
                        );
                    }
                    for (const auto &x : store->passwords()[cmd.path.name]) {
                        if (x.k != "default") {
                            printf("    %s: %s\n", x.k.c_str(), x.v.c_str());
                        }
                    }
                    printf("}\n");
                }
                else if (store->passwords()[cmd.path.name].contains(cmd.path.element)) {
                    printf(
                        "%s.%s: %s\n",
                        cmd.path.name.c_str(), cmd.path.element.c_str(),
                        store->passwords()[cmd.path.name][cmd.path.element].c_str()
                    );
                }
                else {
                    printf("'%s.%s' not found\n", cmd.path.name.c_str(), cmd.path.element.c_str());
                }
            }
            else {
                printf("'%s' not found\n", cmd.path.name.c_str());
            }
        break;

        case CommandType::COPY:
            add_history(cmd.cmdStr.c_str());

            if (cmd.path.element.empty()) cmd.path.element = "default";

            if (
                store->passwords().contains(cmd.path.name)
                && store->passwords()[cmd.path.name].contains(cmd.path.element)
            ) {
                if (clip::set_text(store->passwords()[cmd.path.name][cmd.path.element])) {
                    printf("Password '%s.%s' copied to clipboard\n", cmd.path.name.c_str(), cmd.path.element.c_str());
                }
                else {
                    printf("An error occurred while copying data to clipboard\n");
                }
            }
            else {
                printf("'%s.%s' not found\n", cmd.path.name.c_str(), cmd.path.element.c_str());
            }
        break;

        case CommandType::LIST:
            add_history(cmd.cmdStr.c_str());

            if (cmd.path.name.empty()) {
                auto l = store->list();

                if (l.empty()) {
                    printf("<Empty>\n");
                }
                else {
                    for (const auto &n : l) {
                        printf("%s\n", n.c_str());
                    }
                }
            }
            else {
                if (
                    store->passwords().contains(cmd.path.name)
                    && (cmd.path.element.empty() || store->passwords()[cmd.path.name].contains(cmd.path.element))
                ) {
                    if (cmd.path.element.empty()) {
                        if (store->passwords()[cmd.path.name].contains("default")) {
                            printf("default\n");
                        }
                        for (const auto &x : store->passwords()[cmd.path.name]) {
                            if (x.k != "default") printf("%s\n", x.k.c_str());
                        }
                    }
                    else {
                        printf("%s.%s\n", cmd.path.name.c_str(), cmd.path.element.c_str());
                    }
                }
            }
        break;

        case CommandType::HELP:
            add_history(cmd.cmdStr.c_str());

            print_cmd_help();
        break;

        case CommandType::WRITE:
            add_history(cmd.cmdStr.c_str());

            save_password_store();
        break;

        case CommandType::QUIT:
            printf("Bye!\n\n");
        break;

        case CommandType::WRITE_QUIT:
            save_password_store();
            printf("Bye!\n\n");
        break;

        default: break;
        }

        if (cmd.type == CommandType::QUIT || cmd.type == CommandType::WRITE_QUIT) {
            break;
        }
    }
}

int main(int argc, char **argv) {
    if (! initialize_password_store()) exit(1);
    command_line();
    exit(0);
}
