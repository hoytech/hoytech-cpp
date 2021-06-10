#include <iostream>
#include <string_view>

#include "hoytech/assert_zerocopy.h"

int main() {
    std::string base = "hello world!";

    assert_copied("test", "1234");

    {
        std::string base2 = "hello world!";
        assert_copied(base, base2);
        assert_copied(base2, base);
        assert(hoytech::is_zerocopy(base, base2) == false);

        std::string other = "llo w";
        assert_copied(base, other);
        assert_copied(other, base);
        assert(hoytech::is_zerocopy(base, other) == false);
    }

    {
        std::string_view a(base);
        assert_zerocopy(base, a);
        assert_zerocopy(a, base);
        assert_zerocopy_substr(base, a);
        assert_zerocopy_substr(a, base);
        assert(hoytech::is_zerocopy(base, a) == true);
        assert(hoytech::is_zerocopy(a, base) == true);

        std::string_view b(base);
        assert_zerocopy(base, b);
        assert_zerocopy(b, base);
        assert_zerocopy(a, b);
        assert_zerocopy(b, a);
        assert_zerocopy_substr(a, b);
        assert_zerocopy_substr(b, a);
    }

    {
        std::string_view a(base.data() + 2, 3);
        assert(a == "llo");
        assert_zerocopy(a, base);
        assert_zerocopy(base, a);
        assert_zerocopy_substr(base, a);
        //assert_zerocopy_substr(a, base); // <-- this fails

        assert_copied(a, "llo");
        assert_copied("llo", a);

        assert_copied(a, std::string("llo"));
        assert_copied(std::string("llo"), a);

        std::string copy = base.substr(2, 3);
        assert(copy == "llo");
        assert_copied(copy, a);
        assert_copied(a, copy);
        assert_copied(base, copy);
        assert_copied(copy, base);

        assert_zerocopy(a, a.substr(1, 2));
        assert_zerocopy(a.substr(1, 2), a);
        assert_zerocopy(a.substr(0, 2), a.substr(1, 2));
        assert_zerocopy(a.substr(1, 2), a.substr(0, 2));
    }

    {
        std::string_view a(base.data() + 2, 5);
        assert(a == "llo w");

        std::string_view b(base.data() + 5, 5);
        assert(b == " worl");

        assert_zerocopy(a, b);
        assert_zerocopy(b, a);

        std::string_view c(base.data() + 7, 4);
        assert(c == "orld");

        assert_zerocopy(b, c);
        assert_zerocopy(c, b);
        assert_zerocopy(base, c);
        assert_zerocopy(c, base);

        assert_copied(a, c); // no overlap so we can't assume it's from same buffer
        assert_copied(c, a); // no overlap so we can't assume it's from same buffer
    }

    {
        // empty string edge cases
        assert_zerocopy(base, "");
        assert_zerocopy("", base);
        assert_zerocopy_substr(base, "");
        assert_zerocopy_substr("", "");
        //assert_zerocopy_substr("", base); <-- this fails
        assert_copied(base, "");
        assert_copied("", base);

        assert_zerocopy(base, std::string_view("blah", 0));
        assert_zerocopy(std::string_view("blah", 0), base);
        assert_copied(base, std::string_view("blah", 0));
        assert_copied(std::string_view("blah", 0), base);
    }

    if (0) { // These rely on compiler de-duplicating string literals (fails if compiled at -O0)
        assert_zerocopy("test", "test");
        assert_zerocopy("this is a test", "is a test");
    }

    std::cout << "ALL OK" << std::endl;

    return 0;
}
