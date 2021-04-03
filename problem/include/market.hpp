#pragma once
#include <memory>
#include <unordered_map>

#include "depth_order_book.hpp"
#include "order.hpp"
#include "order_book.hpp"

namespace lhft::me {
    class Market {
    public:
        using OrderId           = book::OrderId;
        using Symbol            = book::Symbol;
        using OrderPtr          = std::shared_ptr<book::Order>;
        using OrderBook         = book::OrderBook<OrderPtr>;
        using OrderBookPtr      = std::shared_ptr<OrderBook>;
        using DepthOrderBook    = book::DepthOrderBook<OrderPtr>;
        using DepthOrderBookPtr = std::shared_ptr<DepthOrderBook>;
        using BookDepth         = book::Depth<>;
        using OrderMap          = std::unordered_map<OrderId, OrderPtr>;
        using SymbolToBookMap   = std::unordered_map<Symbol, OrderBookPtr>;

        auto AddBook(Symbol symbol) -> bool;

        auto RemoveBook(Symbol symbol) -> bool;

        auto FindBook(Symbol symbol) -> OrderBookPtr;

        auto OrderSubmit(const OrderPtr order) -> bool;

        auto OrderCancel(OrderId order_id) -> bool;

        auto RemoveOrder(OrderId order_id) -> bool;

        auto FindExistingOrder(OrderId order_id, OrderPtr& order, OrderBookPtr& book) -> bool;

        auto OnAccept(const OrderPtr& order) -> void;

        auto OnReject(const OrderPtr& order, const char* reason) -> void;

        auto OnFill(const OrderPtr& order, const OrderPtr& matched_order, book::Quantity fill_qty, book::Cost fill_cost,
                    book::FillId fill_id) -> void;

        auto OnCancel(const OrderPtr& order) -> void;

        auto OnCancelReject(const OrderPtr& order, const char* reason) -> void;

        auto OnReplace(const OrderPtr& order, const int64_t& size_delta, book::Price new_price) -> void;

        auto OnReplaceReject(const OrderPtr& order, const char* reason) -> void;

        auto OnTrade(const OrderBook* book, const OrderId& id_1, const OrderId& id_2, book::Quantity qty,
                     book::Price price, bool buyer_maker, book::FillId fill_id) -> void;

        auto OnOrderBookChange(const OrderBook* book) -> void;

        auto OnDepthChange(const DepthOrderBook* book, const BookDepth* depth) -> void;

        auto Snapshot(book::BookData<>& book_snapshot, Symbol symbol) -> void;

    private:
        OrderMap        orders_;
        SymbolToBookMap books_;
    };
}    // namespace lhft::me