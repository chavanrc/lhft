#include <market.hpp>

int main() {
    auto market = std::make_unique<lhft::me::Market>();
    market->AddBook(1);
    return 0;
}
