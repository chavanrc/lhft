#pragma once

#include "types.hpp"

namespace lhft::book {

    class DepthLevel {
    public:
        auto operator=(const DepthLevel& rhs) -> DepthLevel&;

        [[nodiscard]] auto GetPrice() const -> const Price&;

        [[nodiscard]] auto OrderCount() const -> std::size_t;

        [[nodiscard]] auto AggregateQty() const -> Quantity;

        [[nodiscard]] auto IsExcess() const -> bool;

        auto Init(Price price, bool is_excess) -> void;

        auto AddOrder(Quantity qty) -> void;

        auto IncreaseQty(Quantity qty) -> void;

        auto DecreaseQty(Quantity qty) -> void;

        auto Set(Price price, Quantity qty, std::size_t order_count, ChangeId last_change = 0) -> void;

        auto CloseOrder(Quantity qty) -> bool;

        auto LastChange(ChangeId last_change) -> void;

        [[nodiscard]] auto LastChange() const -> ChangeId;

        [[nodiscard]] auto ChangedSince(ChangeId last_published_change) const -> bool;

    private:
        Price       price_{INVALID_LEVEL_PRICE};
        std::size_t order_count_{0};
        Quantity    aggregate_qty_{0};
        bool        is_excess_;
        ChangeId    last_change_;
    };
}    // namespace lhft::book
