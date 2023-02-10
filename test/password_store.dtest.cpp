/*
 * Copyright (c) 2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <password_store.h>
#include <file.h>

unit("password_store", "put")
.body([] {
    PasswordStore s("password");

    s.put("mypass", "pass");
    assert(s.get("mypass") == "pass");
});

unit("password_store", "get")
.body([] {
    PasswordStore s("password");

    s.put("mypass", "pass");
    assert(s.get("mypass") == "pass");

    try {
        s.get("whatever");
        fail("Got non-existent password");
    }
    catch (...) { }
});

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
        s.put("mypass", "pass");

        (OutputFileSerializer(File("password_store.test")) << s).flush();
    }

    {
        PasswordStore s("password");
        InputFileSerializer(File("password_store.test")) >> s;
        assert(s.get("mypass") == "pass");
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

    s.put("mypass2", "pass");
    s.put("mypass1", "pass");
    s.put("mypass", "pass");

    for (auto &p : s.list()) {
        std::cout << p << std::endl;
    }
});
