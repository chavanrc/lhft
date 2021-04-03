#pragma once

#include "depth.hpp"
#include "order_book.hpp"

namespace lhft::book {
    template <typename OrderPtr, int32_t SIZE = 5>
    class DepthOrderBook : public OrderBook<OrderPtr> {
    public:
        using DepthTracker = Depth<SIZE>;

        explicit DepthOrderBook(Symbol symbol = 0);

        DepthTracker& GetDepth() {
            return depth_;
        }

        const DepthTracker& GetDepth() const {
            return depth_;
        }

    protected:
        void OnAccept(const OrderPtr& order, Quantity quantity);

        void OnFill(const OrderPtr& order, const OrderPtr& matched_order, Quantity quantity, Cost fill_cost,
                    bool inbound_order_filled, bool matched_order_filled);

        void OnCancel(const OrderPtr& order, Quantity quantity);

        void OnReplace(const OrderPtr& order, Quantity current_qty, Quantity new_qty, Price new_price);

        void OnOrderBookChange();

    private:
        DepthTracker depth_;
    };
}    // namespace lhft::book

#include "../src/depth_order_book.inl"