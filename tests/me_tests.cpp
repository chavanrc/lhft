#include <catch2/catch.hpp>
#include <iostream>
#include <market.hpp>

TEST_CASE("add market test", "[unit]") {
    auto market = std::make_unique<lhft::me::Market>();
    REQUIRE(market->AddBook(1));
}

TEST_CASE("add and remove market test", "[unit]") {
    auto market = std::make_unique<lhft::me::Market>();
    REQUIRE(market->AddBook(1));
    REQUIRE(market->RemoveBook(1));
}

TEST_CASE("add order test", "[unit]") {
    auto                 market   = std::make_unique<lhft::me::Market>();
    lhft::book::Symbol   symbol   = 1;
    lhft::book::OrderId  order_id = 1;
    bool                 is_buy   = true;
    lhft::book::Quantity quantity = 1;
    lhft::book::Price    price    = 1;
    auto                 order    = std::make_shared<lhft::book::Order>(order_id, is_buy, symbol, quantity, price);
    REQUIRE(market->AddBook(symbol));
    REQUIRE(market->OrderSubmit(order));
    REQUIRE(market->RemoveBook(symbol));
}

TEST_CASE("fill order book test", "[unit]") {
    auto                 market   = std::make_unique<lhft::me::Market>();
    lhft::book::Symbol   symbol   = 1;
    lhft::book::OrderId  order_id = 1;
    bool                 is_buy   = true;
    lhft::book::Quantity quantity = 1;
    lhft::book::Price    price    = 1;
    REQUIRE(market->AddBook(symbol));

    std::random_device                     random_device;
    std::mt19937                           random_engine(random_device());
    std::uniform_int_distribution<int32_t> distribution_1_10(1, 5);

    // Buy
    for (int32_t i = 0; i < 10; i++) {
        auto random_number = distribution_1_10(random_engine);
        for (int32_t j = 0; j < random_number; j++) {
            quantity   = distribution_1_10(random_engine);
            auto order = std::make_shared<lhft::book::Order>(order_id++, is_buy, symbol, quantity, price);
            REQUIRE(market->OrderSubmit(order));
        }
        ++price;
    }
    market->Log();

    // Sell
    for (int32_t i = 0; i < 10; i++) {
        auto random_number = distribution_1_10(random_engine);
        for (int32_t j = 0; j < random_number; j++) {
            quantity   = distribution_1_10(random_engine);
            auto order = std::make_shared<lhft::book::Order>(order_id++, !is_buy, symbol, quantity, price);
            REQUIRE(market->OrderSubmit(order));
        }
        ++price;
    }
    market->Log();
    REQUIRE(market->RemoveBook(symbol));
}

TEST_CASE("cancel order test", "[unit]") {
    auto                 market   = std::make_unique<lhft::me::Market>();
    lhft::book::Symbol   symbol   = 1;
    lhft::book::OrderId  order_id = 1;
    bool                 is_buy   = true;
    lhft::book::Quantity quantity = 1;
    lhft::book::Price    price    = 1;
    auto                 order    = std::make_shared<lhft::book::Order>(order_id, is_buy, symbol, quantity, price);
    REQUIRE(market->AddBook(symbol));
    REQUIRE(market->OrderSubmit(order));
    REQUIRE(market->OrderCancel(order_id));
    REQUIRE(market->RemoveBook(symbol));
}

TEST_CASE("simple matching order test", "[unit]") {
    auto                 market    = std::make_unique<lhft::me::Market>();
    lhft::book::Symbol   symbol    = 1;
    lhft::book::OrderId  order_id  = 1;
    bool                 is_buy    = true;
    lhft::book::Quantity quantity  = 1;
    lhft::book::Price    price     = 1;
    auto                 order_buy = std::make_shared<lhft::book::Order>(order_id, is_buy, symbol, quantity, price);
    auto order_sell = std::make_shared<lhft::book::Order>(order_id + 1, !is_buy, symbol, quantity, price);
    REQUIRE(market->AddBook(symbol));
    REQUIRE(market->OrderSubmit(order_buy));
    REQUIRE(market->OrderSubmit(order_sell));
    REQUIRE(market->RemoveBook(symbol));
}

TEST_CASE("multiple matching order test", "[unit]") {
    auto                market   = std::make_unique<lhft::me::Market>();
    lhft::book::Symbol  symbol   = 1;
    lhft::book::OrderId order_id = 100000;
    REQUIRE(market->AddBook(symbol));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, false, symbol, 1, 1075)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, true, symbol, 9, 1000)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, true, symbol, 30, 975)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, false, symbol, 10, 1050)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, true, symbol, 10, 950)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, false, symbol, 2, 1025)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, true, symbol, 1, 1000)));
    REQUIRE(market->OrderCancel(100004));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, false, symbol, 5, 1025)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, true, symbol, 3, 1050)));
    REQUIRE_FALSE(market->OrderCancel(100008));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, true, symbol, 10, 1000)));
    REQUIRE(market->OrderCancel(100009));
    market->Log();
    REQUIRE_FALSE(market->OrderCancel(100005));
    REQUIRE_FALSE(market->OrderCancel(100010));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, false, symbol, 10, 1025)));
    REQUIRE(market->OrderSubmit(std::make_shared<lhft::book::Order>(order_id++, true, symbol, 10, 1025)));
    market->Log();
}