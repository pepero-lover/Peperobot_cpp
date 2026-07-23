#include "header/bitboard/init.h"
#include "header/engine/uci/uci_manager.h"

std::filesystem::path get_exe_dir(const char* argv0) {
    std::error_code ec;
    std::filesystem::path p = std::filesystem::absolute(argv0, ec);
    if (ec) return std::filesystem::current_path();
    return p.parent_path();
}

int main(int argc, char** argv) {
    init_all(get_exe_dir(argv[0]));

    uci_loop();
}
