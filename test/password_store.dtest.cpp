/*
 * Copyright (c) 2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <password_store.h>
#include <file.h>

unit("password_store", "serialization")
.onInit([] {
    File("password_store.test").open(File::CREATE | File::TRUNCATE);
})
.onComplete([] {
    File("password_store.test").remove();
})
.body([] {
    {
        PasswordStore s("password");
        s.passwords().put("mypass", HashMap<std::string, std::string>({ { "default", "pass" } }));

        (OutputFileSerializer(File("password_store.test")) << s).flush();
    }

    {
        PasswordStore s("password");
        InputFileSerializer(File("password_store.test")) >> s;
        assert(s.passwords().get("mypass").get("default") == "pass");
    }

    {
        PasswordStore s("password1");
        try {
            InputFileSerializer(File("password_store.test")) >> s;
            fail("Decrypted using invalid password");
        }
        catch (...) { }
    }
});

unit("password_store", "list")
.body([] {
    PasswordStore s("password");

    s.passwords().put("mypass2", HashMap<std::string, std::string>({ { "default", "pass" } }));
    s.passwords().put("mypass1", HashMap<std::string, std::string>({ { "default", "pass" } }));
    s.passwords().put("mypass", HashMap<std::string, std::string>({ { "default", "pass" } }));
    s.passwords().put("mypass3", HashMap<std::string, std::string>({ { "default", "pass" } }));

    for (auto &p : s.list()) {
        std::cout << p << std::endl;
    }
});
