namespace lhft::book {
    template<class OrderPtr, int32_t SIZE>
    DepthOrderBook<OrderPtr, SIZE>::DepthOrderBook(Symbol symbol) : OrderBook<OrderPtr>{symbol} {
    }

    template<class OrderPtr, int32_t SIZE>
    void DepthOrderBook<OrderPtr, SIZE>::OnAccept(const OrderPtr &order, Quantity quantity) {
        if (order->IsLimit()) {
            if (quantity == order->OrderQty()) {
                depth_.IgnoreFillQty(quantity, order->IsBuy());
            } else {
                depth_.AddOrder(order->GetPrice(), order->OrderQty(), order->IsBuy());
            }
        }
    }

    template<class OrderPtr, int32_t SIZE>
    void DepthOrderBook<OrderPtr, SIZE>::OnFill(const OrderPtr &order, const OrderPtr &matched_order, Quantity quantity,
                                                Cost fill_cost,
                                                bool inbound_order_filled, bool matched_order_filled) {
        if (matched_order->IsLimit()) {
            depth_.FillOrder(matched_order->GetPrice(), quantity, matched_order_filled, matched_order->IsBuy());
        }
        if (order->IsLimit()) {
            depth_.FillOrder(order->GetPrice(), quantity, inbound_order_filled, order->IsBuy());
        }
    }

    template<class OrderPtr, int32_t SIZE>
    void DepthOrderBook<OrderPtr, SIZE>::OnCancel(const OrderPtr &order, Quantity quantity) {
        if (order->IsLimit()) {
            depth_.CloseOrder(order->GetPrice(), quantity, order->IsBuy());
        }
    }

    template<class OrderPtr, int32_t SIZE>
    void DepthOrderBook<OrderPtr, SIZE>::OnReplace(const OrderPtr &order, Quantity current_qty, Quantity new_qty,
                                                   Price new_price) {
        // TODO
    }

    template<class OrderPtr, int32_t SIZE>
    void DepthOrderBook<OrderPtr, SIZE>::OnOrderBookChange() {
        if (depth_.Changed()) {
            depth_.Published();
        }
    }
}    // namespace lhft::book