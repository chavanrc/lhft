#pragma once

#include "depth.hpp"
#include "order_book.hpp"

namespace lhft::book {
    template <typename OrderPtr, int32_t SIZE = 5>
    class DepthOrderBook : public OrderBook<OrderPtr> {
    public:
        using DepthTracker = Depth<SIZE>;

        explicit DepthOrderBook(Symbol symbol = 0) : OrderBook<OrderPtr>{symbol} {
        }

        DepthTracker& GetDepth() {
            return depth_;
        }

        const DepthTracker& GetDepth() const {
            return depth_;
        }

    protected:
        void OnAccept(const OrderPtr& order, Quantity quantity) {
            if (order->IsLimit()) {
                if (quantity == order->OrderQty()) {
                    depth_.IgnoreFillQty(quantity, order->IsBuy());
                } else {
                    depth_.AddOrder(order->GetPrice(), order->OrderQty(), order->IsBuy());
                }
            }
        }

        void OnFill(const OrderPtr& order, const OrderPtr& matched_order, Quantity quantity, Cost fill_cost,
                    bool inbound_order_filled, bool matched_order_filled) {
            if (matched_order->IsLimit()) {
                depth_.FillOrder(matched_order->GetPrice(), quantity, matched_order_filled, matched_order->IsBuy());
            }
            if (order->IsLimit()) {
                depth_.FillOrder(order->GetPrice(), quantity, inbound_order_filled, order->IsBuy());
            }
        }

        void OnCancel(const OrderPtr& order, Quantity quantity) {
            if (order->IsLimit()) {
                depth_.CloseOrder(order->GetPrice(), quantity, order->IsBuy());
            }
        }

        void OnReplace(const OrderPtr& order, Quantity current_qty, Quantity new_qty, Price new_price) {
            // TODO
        }

        void OnOrderBookChange() {
            if (depth_.Changed()) {
                depth_.Published();
            }
        }

    private:
        DepthTracker depth_;
    };
}    // namespace lhft::book