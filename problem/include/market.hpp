#pragma once
#include <memory>
#include <unordered_map>

#include "order.hpp"
#include "order_book.hpp"

namespace lhft::me {
    class Market {
    public:
        using OrderId         = book::OrderId;
        using Symbol          = book::Symbol;
        using OrderPtr        = std::shared_ptr<book::Order>;
        using OrderBook       = book::OrderBook<OrderPtr>;
        using OrderBookPtr    = std::shared_ptr<OrderBook>;
        using OrderMap        = std::unordered_map<OrderId, OrderPtr>;
        using SymbolToBookMap = std::unordered_map<Symbol, OrderBookPtr>;

        auto AddBook(Symbol symbol) -> bool;

        auto RemoveBook(Symbol symbol) -> bool;

        auto FindBook(Symbol symbol) -> OrderBookPtr;

        auto OrderSubmit(const OrderPtr& order) -> bool;

        auto OrderCancel(OrderId order_id) -> bool;

        auto RemoveOrder(OrderId order_id) -> bool;

        auto FindExistingOrder(OrderId order_id, OrderPtr& order, OrderBookPtr& book) -> bool;

        auto Log() const -> void;

    private:
        OrderMap        orders_{};
        SymbolToBookMap books_{};
    };
}    // namespace lhft::me