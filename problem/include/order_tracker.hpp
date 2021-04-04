#pragma once
#include "comparable_price.hpp"
#include "types.hpp"

namespace lhft::book {
    template <typename OrderPtr>
    class OrderTracker {
    public:
        explicit OrderTracker(const OrderPtr& order);

        auto ChangeQty(int64_t delta) -> void;

        auto Fill(Quantity qty) -> void;

        [[nodiscard]] auto Filled() const -> bool;

        [[nodiscard]] auto FilledQty() const -> Quantity;

        [[nodiscard]] auto OpenQty() const -> Quantity;

        auto Ptr() const -> const OrderPtr&;

        auto Ptr() -> OrderPtr&;

    private:
        OrderPtr order_;
        Quantity open_qty_;
    };
}    // namespace lhft::book

#include "order_tracker.inl"