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
    auto                 order    = std::make_shared<lhft::book::Order>(order_id, is_buy, quantity, symbol, price);
    REQUIRE(market->AddBook(symbol));
    REQUIRE(market->OrderSubmit(order));
    REQUIRE(market->RemoveBook(symbol));
}

TEST_CASE("cancel order test", "[unit]") {
    auto                 market   = std::make_unique<lhft::me::Market>();
    lhft::book::Symbol   symbol   = 1;
    lhft::book::OrderId  order_id = 1;
    bool                 is_buy   = true;
    lhft::book::Quantity quantity = 1;
    lhft::book::Price    price    = 1;
    auto                 order    = std::make_shared<lhft::book::Order>(order_id, is_buy, quantity, symbol, price);
    REQUIRE(market->AddBook(symbol));
    REQUIRE(market->OrderSubmit(order));
    REQUIRE(market->OrderCancel(order_id));
    REQUIRE(market->RemoveBook(symbol));
}

TEST_CASE("matching order test", "[unit]") {
    auto                 market    = std::make_unique<lhft::me::Market>();
    lhft::book::Symbol   symbol    = 1;
    lhft::book::OrderId  order_id  = 1;
    bool                 is_buy    = true;
    lhft::book::Quantity quantity  = 1;
    lhft::book::Price    price     = 1;
    auto                 order_buy = std::make_shared<lhft::book::Order>(order_id, is_buy, quantity, symbol, price);
    auto order_sell = std::make_shared<lhft::book::Order>(order_id + 1, !is_buy, quantity, symbol, price);
    REQUIRE(market->AddBook(symbol));
    REQUIRE(market->OrderSubmit(order_buy));
    REQUIRE(market->OrderSubmit(order_sell));
    REQUIRE(market->RemoveBook(symbol));
}
