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

        auto AddBook(Symbol symbol) -> bool {
            std::cout << "Create new depth order book for " << symbol << '\n';
            auto [iter, inserted] = books_.insert_or_assign(symbol, std::make_shared<DepthOrderBook>(symbol));
            return inserted;
        }

        auto RemoveBook(Symbol symbol) -> bool {
            bool result{false};
            auto book = books_.find(symbol);
            if (book != books_.end()) {
                auto order_id_list = book->second->AllOrderCancel();
                result             = books_.erase(symbol) == 1;
                for (const auto& order_id : order_id_list) {
                    RemoveOrder(order_id);
                }
            }
            return result;
        }

        auto FindBook(Symbol symbol) -> OrderBookPtr {
            OrderBookPtr result = nullptr;
            auto         entry  = books_.find(symbol);
            if (entry != books_.end()) {
                result = entry->second;
            }
            return result;
        }

        auto OrderSubmit(const OrderPtr order) -> bool {
            bool result{false};
            if (!order) {
                std::cout << "Invalid order ref.\n";
                return result;
            }
            auto symbol = order->GetSymbol();
            auto book   = FindBook(symbol);
            if (!book) {
                std::cout << "Symbol: " << symbol << "book not found.\n";
                return result;
            }
            auto order_id = order->GetOrderId();
            std::cout << "ADDING order: " << *order;
            auto [iter, inserted] = orders_.insert_or_assign(order_id, order);
            if (inserted && book->Add(order)) {
                std::cout << order_id << " matched\n";
            }
            return inserted;
        }

        auto OrderCancel(OrderId order_id) -> bool {
            OrderPtr     order{nullptr};
            OrderBookPtr book{nullptr};
            bool         result{false};
            if (FindExistingOrder(order_id, order, book)) {
                std::cout << "Requesting Cancel: " << *order;
                book->Cancel(order);
                result = true;
            }
            return result;
        }

        auto RemoveOrder(OrderId order_id) -> bool {
            return orders_.erase(order_id) == 1;
        }

        auto FindExistingOrder(OrderId order_id, OrderPtr& order, OrderBookPtr& book) -> bool {
            auto order_position = orders_.find(order_id);
            if (order_position == orders_.end()) {
                std::cout << "--Can't find OrderID #" << order_id << '\n';
                return false;
            }

            order       = order_position->second;
            auto symbol = order->GetSymbol();
            book        = FindBook(symbol);
            if (!book) {
                std::cout << "--No order book for symbol " << symbol << '\n';
                return false;
            }
            return true;
        }

        auto OnAccept(const OrderPtr& order) -> void {
            order->OnAccepted();
            std::cout << "Event: Accepted: " << *order;
        }

        auto OnReject(const OrderPtr& order, const char* reason) -> void {
            order->OnRejected(reason);
            std::cout << "Event: Rejected: " << *order << ' ' << reason;
        }

        auto OnFill(const OrderPtr& order, const OrderPtr& matched_order, book::Quantity fill_qty, book::Cost fill_cost,
                    book::FillId fill_id) -> void {
            order->OnFilled(fill_qty, fill_cost);
            matched_order->OnFilled(fill_qty, fill_cost);

            std::stringstream out;
            out << (order->IsBuy() ? "Event: Fill-Bought: " : "Event: Fill-Sold: ") << fill_qty << " Shares for "
                << fill_cost << ' ' << *order;
            out << (matched_order->IsBuy() ? "Bought: " : "Sold: ") << fill_qty << " Shares for " << fill_cost << ' '
                << *matched_order;
            std::cout << out.str() << '\n';

            order->AddTradeHistory(fill_qty, matched_order->QuantityOnMarket(), fill_cost, matched_order->GetOrderId(),
                                   matched_order->GetPrice(), fill_id);
            matched_order->AddTradeHistory(fill_qty, order->QuantityOnMarket(), fill_cost, order->GetOrderId(),
                                           order->GetPrice(), fill_id);
        }

        auto OnCancel(const OrderPtr& order) -> void {
            order->OnCancelled();
            std::cout << "Event: Canceled: " << *order;
        }

        auto OnCancelReject(const OrderPtr& order, const char* reason) -> void {
            order->OnCancelRejected(reason);
            std::cout << "Event: Cancel Reject: " << *order << ' ' << reason;
        }

        auto OnReplace(const OrderPtr& order, const int64_t& size_delta, book::Price new_price) -> void {
            // TODO
        }

        auto OnReplaceReject(const OrderPtr& order, const char* reason) -> void {
            // TODO
        }

        auto OnTrade(const OrderBook* book, const OrderId& id_1, const OrderId& id_2, book::Quantity qty,
                     book::Price price, bool buyer_maker, book::FillId fill_id) -> void {
            // TODO
        }

        auto OnOrderBookChange(const OrderBook* book) -> void {
            // TODO
        }

        auto OnDepthChange(const DepthOrderBook* book, const BookDepth* depth) -> void {
            // TODO
        }

        auto Snapshot(book::BookData<>& book_snapshot, Symbol symbol) -> void {
            // lookup order book from symbol
            auto book = FindBook(symbol);
            if (!book) {
                throw std::runtime_error("Failed to find valid book.");
            }
            const OrderBook::TrackerMap&                          asks = book->GetAsks();
            const OrderBook::TrackerMap&                          bids = book->GetBids();
            std::map<book::Price, book::Quantity, std::greater<>> agg_bids;
            std::map<book::Price, book::Quantity>                 agg_asks;

            book_snapshot.symbol_ = symbol;
            // aggregate list of asks from order book
            for (auto ask = asks.crbegin(); ask != asks.crend(); ++ask) {
                book::Price    price = ask->first.GetPrice();
                book::Quantity qty   = ask->second.OpenQty();
                if (agg_asks.count(price))
                    agg_asks[price] += qty;
                else
                    agg_asks[price] = qty;
            }

            // aggregate list of bids from order book
            for (auto bid = bids.crbegin(); bid != bids.crend(); ++bid) {
                book::Price    price = bid->first.GetPrice();
                book::Quantity qty   = bid->second.OpenQty();
                if (agg_bids.count(price))
                    agg_bids[price] += qty;
                else
                    agg_bids[price] = qty;
            }
            int32_t iter = 0;
            // gather list of aggregate asks from order book
            for (auto& agg_ask : agg_asks) {
                book_snapshot.asks_[iter].first  = agg_ask.first;
                book_snapshot.asks_[iter].second = agg_ask.second;

                if (book::BOOK_DEPTH == ++iter) {
                    break;
                }
            }
            iter = 0;
            // gather list of aggregate bids from order book
            for (auto& agg_bid : agg_bids) {
                book_snapshot.bids_[iter].first  = agg_bid.first;
                book_snapshot.bids_[iter].second = agg_bid.second;

                if (book::BOOK_DEPTH == ++iter) {
                    break;
                }
            }
        }

    private:
        OrderMap        orders_;
        SymbolToBookMap books_;
    };
}    // namespace lhft::me