/*
 * Copyright (c) 2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <password_store.h>
#include <file.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

#define PASS_MAX 1024
#define CMD_MAX 1024

PasswordStore *store = nullptr;
File *passFile = nullptr;
std::vector<std::string> storedPasswordNames;

void getPassword(char *password) {
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
        getPassword(password);

        store = new PasswordStore(password);

        try {
            InputFileSerializer(*passFile) >> *store;
        }
        catch (const Error &e) {
            printf("%s\n\n", e.what());
            return false;
        }

        storedPasswordNames = store->list();
        return true;
    }
    else {
        printf(
            "Password file '%s' not found; initializing empty password store.\n",
            passFile->info().path().get()
        );

        char password[PASS_MAX + 1], confirm[PASS_MAX + 1];

        printf("\nPassword: ");
        getPassword(password);
        printf("Confirm : ");
        getPassword(confirm);

        while (strcmp(password, confirm)) {
            printf("Password mismatch. Try again\n");
            printf("\nPassword: ");
            getPassword(password);
            printf("Confirm : ");
            getPassword(confirm);
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
        "    (r)emove    <name>            : remove a stored password\n"
        "    (g)et       <name>            : get a stored passwords\n"
        "    (l)ist                        : list all stored passwords\n"
        "    (s)ave                        : save changes\n"
        "    (h)elp                        : show this help\n"
        "    (q)uit|exit                   : terminate\n"
        "\n"
    );
}

char * comp_generator(const char *text, int state) {
    static size_t i, len;

    if (!state) {
        i = 0;
        len = strlen(text);
    }
    else {
        ++i;
    }

    if (len < 2) return nullptr;

    for (; i < storedPasswordNames.size(); ++i) {
        auto s = storedPasswordNames[i].c_str();
        if (strncmp(s, text, len) == 0) {
            return strdup(s);
        }
    }

    return nullptr;
}

char ** comp_func(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, comp_generator);
}

void command_line() {
    rl_attempted_completion_function = comp_func;

    char *cmd = nullptr, *tokenizedCmd = nullptr, *p, *n, *k;

    while (true) {
        if (cmd) free(cmd);
        if (tokenizedCmd) free(tokenizedCmd);
        cmd = readline("\n>> ");

        tokenizedCmd = strdup(cmd);
        p = strtok(tokenizedCmd, " ");
        if (strcasecmp(p, "a") == 0 || strcasecmp(p, "add") == 0) {
            n = strtok(nullptr, " ");
            k = strtok(nullptr, " ");

            if (n == nullptr) {
                printf("Name argument missing for command 'add'\n");
                continue;
            }
            if (k == nullptr) {
                printf("Password argument missing for command 'add'\n");
                continue;
            }
            if (strtok(nullptr, " ") != nullptr) {
                printf("Unexpected arguments given to command 'add'\n");
                continue;
            }

            store->put(n, k);
            storedPasswordNames = store->list();
        }
        else if (strcasecmp(p, "r") == 0 || strcasecmp(p, "remove") == 0) {
            n = strtok(nullptr, " ");

            if (n == nullptr) {
                printf("Name argument missing for command 'remove'\n");
                continue;
            }
            if (strtok(nullptr, " ") != nullptr) {
                printf("Unexpected arguments given to command 'remove'\n");
                continue;
            }

            add_history(cmd);

            if (store->remove(n)) {
                printf("'%s' removed\n", n);
                storedPasswordNames = store->list();
            }
            else {
                printf("'%s' not found\n", n);
            }
        }
        else if (strcasecmp(p, "g") == 0 || strcasecmp(p, "get") == 0) {
            n = strtok(nullptr, " ");

            if (n == nullptr) {
                printf("Name argument missing for command 'get'\n");
                continue;
            }
            if (strtok(nullptr, " ") != nullptr) {
                printf("Unexpected arguments given to command 'get'\n");
                continue;
            }

            add_history(cmd);

            try {
                printf("%s: %s\n", n, store->get(n).c_str());
            }
            catch (const ElementNotFoundError &e) {
                printf("'%s' not found\n", n);
            }
        }
        else if (strcasecmp(p, "l") == 0 || strcasecmp(p, "list") == 0) {
            if (strtok(nullptr, " ") != nullptr) {
                printf("Unexpected arguments given to command 'list'\n");
                continue;
            }

            add_history(cmd);

            if (storedPasswordNames.empty()) {
                printf("<Empty>\n");
            }
            else {
                for (const auto &n : storedPasswordNames) {
                    printf("%s\n", n.c_str());
                }
            }
        }
        else if (strcasecmp(p, "s") == 0 || strcasecmp(p, "save") == 0) {
            save_password_store();
            add_history(cmd);
        }
        else if (strcasecmp(p, "h") == 0 || strcasecmp(p, "help") == 0) {
            print_cmd_help();
            add_history(cmd);
        }
        else if (strcasecmp(p, "q") == 0 || strcasecmp(p, "quit") == 0 || strcasecmp(p, "exit") == 0) {
            printf("Bye!\n\n");
            break;
        }
        else {
            printf("Unknown command '%s'\n", p);
        }
    }

    if (cmd) free(cmd);
    if (tokenizedCmd) free(tokenizedCmd);
}

int main(int argc, char **argv) {
    if (! initialize_password_store()) exit(1);
    command_line();
    exit(0);
}
