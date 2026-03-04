#include <cstdio>
#include <cstring>

#include <thread>
#include <chrono>
#include <vector>
#include <functional>

#include "../hoytech/stream.h"


// --- Minimal test framework (no macros for exception checking) ---
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) void test_##name()

#define ASSERT(cond) do { \
    ++tests_run; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } else { \
        ++tests_passed; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_STR_EQ(a, b) ASSERT(0 == strcmp((a), (b)))

// Helper for exception testing
inline void assert_throws(const std::function<void()>& fn, const std::string& expected_msg) {
    ++tests_run;
    bool threw = false;
    try {
        fn();
    } catch (std::exception &e) {
        threw = true;
        if (std::string(e.what()).find(expected_msg) == std::string::npos) {
            fprintf(stderr, "FAIL: Expected exception containing '%s', got: %s\n", expected_msg.c_str(), e.what());
            return; // failure
        }
        ++tests_passed;
        return;
    } catch (...) {
        threw = true;
        fprintf(stderr, "FAIL: Unknown exception type thrown\n");
        return;
    }
    if (!threw) {
        fprintf(stderr, "FAIL: No exception thrown (expected: '%s')\n", expected_msg.c_str());
    }
}



// --- Test implementations ---

TEST(basic_read_write) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);
    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    {
        hoytech::StreamWriter writer(write_fd);
        writer.write("hello\nworld\n", 5000);
    }

    {
        hoytech::StreamReader reader(read_fd);
        std::string line1 = reader.read(1000);
        ASSERT_STR_EQ(line1.c_str(), "hello");
        std::string line2 = reader.read(1000);
        ASSERT_STR_EQ(line2.c_str(), "world");
    }

    close(pipefd[0]);
    close(pipefd[1]);
}

TEST(timeout_on_read) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);
    int read_fd = pipefd[0];

    {
        hoytech::StreamReader reader(read_fd);
        assert_throws([&]{ reader.read(100); }, "timed out");
    }
}

TEST(timeout_on_write) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);
    int write_fd = pipefd[1];

    std::string dummy(1'000'000, 'A');

    {
        hoytech::StreamWriter writer(write_fd);
        assert_throws([&]{ writer.write(dummy, 100); }, "timed out");
    }
}

TEST(max_record_size) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);

    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    {
        hoytech::StreamWriter writer(write_fd);
        writer.write("abc\ndef");
    }

    {
        hoytech::StreamReader reader(read_fd);
        reader.setMaxRecordSize(2);
        assert_throws([&]{ reader.read(); }, "maxRecordSize");
    }
}

TEST(separator_change) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);

    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    {
        hoytech::StreamWriter writer(write_fd);
        writer.write("a|b|c|");
    }

    {
        hoytech::StreamReader reader(read_fd);
        reader.setSep('|');
        ASSERT_STR_EQ(reader.read(1000).c_str(), "a");
        ASSERT_STR_EQ(reader.read(1000).c_str(), "b");
        ASSERT_STR_EQ(reader.read(1000).c_str(), "c");
    }
}

TEST(pipe_closed_read) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);
    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    {
        hoytech::StreamWriter writer(write_fd);
        writer.write("hello\n--INCOMPLETE--");
    }

    // Force close pipe
    close(write_fd);

    {
        hoytech::StreamReader reader(read_fd);
        ASSERT_STR_EQ(reader.read(1000).c_str(), "hello");
        assert_throws([&]{ reader.read(1000); }, "pipe closed");
    }
}

TEST(incomplete_line_with_timeout) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);

    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    hoytech::StreamWriter writer(write_fd);
    writer.write("partial", 5000);

    hoytech::StreamReader reader(read_fd);
    assert_throws([&]{ reader.read(50); }, "timed out");

    // Now send newline
    writer.write("\n");
    ASSERT_STR_EQ(reader.read(1000).c_str(), "partial");
}

TEST(multiline_buffering) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);

    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    {
        hoytech::StreamWriter writer(write_fd);
        writer.write("first\nsecond\nthird\n");
    }

    {
        hoytech::StreamReader reader(read_fd);
        ASSERT_STR_EQ(reader.read(1000).c_str(), "first");
        ASSERT_STR_EQ(reader.read(1000).c_str(), "second");
        ASSERT_STR_EQ(reader.read(1000).c_str(), "third");
    }
}

// Helper to simulate delayed writes
static void writer_task(int fd, std::vector<std::pair<std::string, int>> steps) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    for (auto& [s, delay] : steps) {
        ::write(fd, s.c_str(), s.size());
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

TEST(multithreaded_write_read) {
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0);

    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    std::vector<std::thread> threads;

    threads.emplace_back([&](){
        writer_task(write_fd, {{"abc\ndef\n", 10}, {"ghi\n", 20}});
    });

    {
        hoytech::StreamReader reader(read_fd);
        ASSERT_STR_EQ(reader.read(1000).c_str(), "abc");
        ASSERT_STR_EQ(reader.read(1000).c_str(), "def");
        ASSERT_STR_EQ(reader.read(1000).c_str(), "ghi");
    }

    for (auto& t : threads) t.join();
    close(read_fd);
    close(write_fd);
}

// --- Main ---
int main() {
    fprintf(stderr, "Running tests...\n");

    test_basic_read_write();
    test_timeout_on_read();
    test_timeout_on_write();
    test_max_record_size();
    test_separator_change();
    test_pipe_closed_read();
    test_incomplete_line_with_timeout();
    test_multiline_buffering();
    test_multithreaded_write_read();

    fprintf(stderr, "\n%2d/%2d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
