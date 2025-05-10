// #define MLOG_LEVEL MLOG_LEVEL_ERROR

#include "allay/mlog/mlog.hpp"

namespace {

void test(const std::string &name) {
    mlog::debug(name) << " 1 hello, debug-" << name << '\n';
    mlog::info(name) << " 2 hello, info-" << name << '\n';
    mlog::warn(name) << " 3 hello, warn-" << name << '\n';
    mlog::error(name) << " 4 hello, error-" << name << '\n';
}

void prepare() {
    mlog::create_logger("A")
        .set_format(mlog::Format::LEVEL_SIGNATURE_TIME)
        .link_file_default()
        .lock();

    mlog::create_logger("B").link_file_app("b.log").lock();

    mlog::create_logger("C")
        .link_file_trunc("c.log")
        .enable_file_and_cout()
        .lock();

    mlog::create_logger("d").link_file_trunc("d.log");
    mlog::create_logger("e").link_file_trunc("e.log");
    mlog::create_logger("f").link_file_trunc("f.log");
}

void test2() {
    int i = 0;
    while (i < 200) {
        MLOG_IF_FIRST_N(10) { MLOG_INFO("d") << "i = " << i << '\n'; }
        MLOG_IF_EVERY_N(20) { MLOG_WARN("e") << "i = " << i << '\n'; }
        MLOG_IF_MORETHAN_N(180) { MLOG_ERROR("f") << "i = " << i << '\n'; }

        i++;
    }
}

}  // namespace

int main(int argc, char *argv[]) {
    mlog::init(PREFIX + std::string("/.mlog/"));
    mlog::set_level_info();

    prepare();

    mlog::show_detail();

    test("cout");
    test("A");
    test("B");
    test("C");

    test2();

    return 0;
}
