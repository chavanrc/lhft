#include <market.hpp>

namespace lhft::me {
    auto Market::AddBook(Symbol symbol) -> bool {
        std::cout << "Create new depth order book for " << symbol << '\n';
        auto [iter, inserted] = books_.insert_or_assign(symbol, std::make_shared<OrderBook>(symbol));
        return inserted;
    }

    auto Market::RemoveBook(Symbol symbol) -> bool {
        bool result = false;
        auto book   = books_.find(symbol);
        if (book != books_.end()) {
            auto order_id_list = book->second->AllOrderCancel();
            result             = books_.erase(symbol) == 1;
            for (const auto &order_id : order_id_list) {
                RemoveOrder(order_id);
            }
        }
        return result;
    }

    auto Market::FindBook(Symbol symbol) -> OrderBookPtr {
        OrderBookPtr result = nullptr;
        auto         entry  = books_.find(symbol);
        if (entry != books_.end()) {
            result = entry->second;
        }
        return result;
    }

    auto Market::OrderSubmit(const OrderPtr &order) -> bool {
        bool result = false;
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
        std::cout << "ADDING order: " << *order << '\n';
        auto [iter, inserted] = orders_.insert_or_assign(order_id, order);
        if (inserted && book->Add(order)) {
            std::cout << order_id << " matched\n";
            for (const auto &event : order->GetTrades()) {
                OrderPtr     matched_order;
                OrderBookPtr matched_book;
                if (FindExistingOrder(event.matched_order_id_, matched_order, matched_book)) {
                    if(matched_order->QuantityOnMarket() == 0) {
                        RemoveOrder(matched_order->GetOrderId());
                    }
                }
            }
            if (order->QuantityOnMarket() == 0) {
                RemoveOrder(order_id);
            }
        }
        return inserted;
    }

    auto Market::OrderCancel(OrderId order_id) -> bool {
        OrderPtr     order  = nullptr;
        OrderBookPtr book   = nullptr;
        bool         result = false;
        if (FindExistingOrder(order_id, order, book)) {
            std::cout << "Requesting Cancel: " << *order << '\n';
            book->Cancel(order);
            result = RemoveOrder(order_id);
        }
        return result;
    }

    auto Market::RemoveOrder(OrderId order_id) -> bool {
        return orders_.erase(order_id) == 1;
    }

    auto Market::FindExistingOrder(OrderId order_id, OrderPtr &order, OrderBookPtr &book) -> bool {
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

    auto Market::Log() const -> void {
        for (const auto &[symbol, book] : books_) {
            book->Log();
        }
    }
}    // namespace lhft::me