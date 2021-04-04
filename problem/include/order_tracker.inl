namespace lhft::book {
    template <typename OrderPtr>
    OrderTracker<OrderPtr>::OrderTracker(const OrderPtr& order) : order_(order), open_qty_(order->OrderQty()) {
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::ChangeQty(int64_t delta) -> void {
        if (delta < 0 && (int64_t)open_qty_ < std::abs(delta)) {
            throw std::runtime_error("Replace size reduction larger than open quantity");
        }
        open_qty_ += delta;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Fill(Quantity qty) -> void {
        if (qty > open_qty_) {
            throw std::runtime_error("Fill size larger than open quantity");
        }
        open_qty_ -= qty;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Filled() const -> bool {
        return open_qty_ == 0;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::FilledQty() const -> Quantity {
        return order_->OrderQty() - OpenQty();
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::OpenQty() const -> Quantity {
        return open_qty_;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Ptr() const -> const OrderPtr& {
        return order_;
    }

    template <typename OrderPtr>
    auto OrderTracker<OrderPtr>::Ptr() -> OrderPtr& {
        return order_;
    }
}    // namespace lhft::book