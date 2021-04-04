#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include <fstream>
#include <market.hpp>
#include <ostream>

std::string filename;

int32_t main(int32_t argc, char* argv[]) {
    Catch::Session session;
    using namespace Catch::clara;
    auto cli = session.cli() | Opt(filename, "file name")["-g"]["--file_name"]("input file name");
    session.cli(cli);
    int32_t return_code = session.applyCommandLine(argc, argv);
    if (return_code != 0) {
        return return_code;
    }
    return session.run();
}

struct Order {
    char                 msg_type_{'\0'};
    lhft::book::OrderId  order_id_{0};
    lhft::book::Symbol   symbol_{1};
    bool                 is_buy_{false};
    lhft::book::Quantity quantity_{0};
    lhft::book::Price    price_{0};

    friend std::ostream& operator<<(std::ostream& os, const Order& order) {
        os << "msg_type: " << order.msg_type_ << " order_id: " << order.order_id_ << " symbol: " << order.symbol_
           << " is_buy: " << order.is_buy_ << " quantity: " << order.quantity_ << " price: " << order.price_;
        return os;
    }
};

static std::optional<Order> ReadLine(const std::string& line) {
    std::string        s;
    std::istringstream iss(line);

    // Reading msg_type
    std::getline(iss, s, ',');
    char msg_type = s[0];

    switch (msg_type) {
        case 'A':
        case 'X': {
            Order order;
            order.msg_type_ = msg_type;
            std::getline(iss, s, ',');
            order.order_id_ = std::stoi(s);
            std::getline(iss, s, ',');
            order.is_buy_ = s[0] == 'B';
            std::getline(iss, s, ',');
            order.quantity_ = std::stoi(s);
            std::getline(iss, s, ',');
            order.price_ = std::stoi(s);
            std::cout << order << '\n';
            return order;
        }
        default: {
            std::cout << "ERROR: Invalid msg type: " << msg_type << std::endl;
            return {};
        }
    }
}

TEST_CASE("lhft file test", "[unit]") {
    std::string   line;
    std::ifstream infile(filename.c_str(), std::ifstream::in);
    auto          market = std::make_unique<lhft::me::Market>();
    REQUIRE(market->AddBook(1));
    while (std::getline(infile, line)) {
        auto order = ReadLine(line);
        if (order) {
            switch (order->msg_type_) {
                case 'A':
                    market->OrderSubmit(std::make_shared<lhft::book::Order>(
                            order->order_id_, order->is_buy_, order->symbol_, order->quantity_, order->price_));
                    break;
                case 'X':
                    market->OrderCancel(order->order_id_);
                    break;
                default:
                    std::cout << "Invalid msg type " << order->msg_type_ << '\n';
                    break;
            }
        }
    }
    market->Log();
}