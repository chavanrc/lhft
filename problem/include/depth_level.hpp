#pragma once

#include "types.hpp"

namespace lhft::book {

    class DepthLevel {
    public:
        DepthLevel() : price_{INVALID_LEVEL_PRICE}, order_count_{0}, aggregate_qty_{0} {
        }

        auto operator=(const DepthLevel& rhs) -> DepthLevel& {
            price_         = rhs.price_;
            order_count_   = rhs.order_count_;
            aggregate_qty_ = rhs.aggregate_qty_;
            if (rhs.price_ != INVALID_LEVEL_PRICE) {
                last_change_ = rhs.last_change_;
            }

            // Do not copy is_excess_

            return *this;
        }

        [[nodiscard]] auto GetPrice() const -> const Price& {
            return price_;
        }

        [[nodiscard]] auto OrderCount() const -> std::size_t {
            return order_count_;
        }

        [[nodiscard]] auto AggregateQty() const -> Quantity {
            return aggregate_qty_;
        }

        [[nodiscard]] auto IsExcess() const -> bool {
            return is_excess_;
        }

        auto Init(Price price, bool is_excess) -> void {
            price_         = price;
            order_count_   = 0;
            aggregate_qty_ = 0;
            is_excess_     = is_excess;
        }

        auto AddOrder(Quantity qty) -> void {
            ++order_count_;
            aggregate_qty_ += qty;
        }

        auto IncreaseQty(Quantity qty) -> void {
            aggregate_qty_ += qty;
        }

        auto DecreaseQty(Quantity qty) -> void {
            aggregate_qty_ -= qty;
        }

        auto Set(Price price, Quantity qty, std::size_t order_count, ChangeId last_change = 0) -> void {
            price_         = price;
            aggregate_qty_ = qty;
            order_count_   = order_count;
            last_change_   = last_change;
        }

        auto CloseOrder(Quantity qty) -> bool {
            bool empty = false;
            // If this is the last order, reset the level
            if (order_count_ == 0) {
                throw std::runtime_error("DepthLevel::CloseOrder order count too low");
            } else if (order_count_ == 1) {
                order_count_   = 0;
                aggregate_qty_ = 0;
                empty          = true;
                // Else, decrement/decrease
            } else {
                --order_count_;
                if (aggregate_qty_ >= qty) {
                    aggregate_qty_ -= qty;
                } else {
                    throw std::runtime_error("DepthLevel::CloseOrder level quantity too low");
                }
            }
            return empty;
        }

        auto LastChange(ChangeId last_change) -> void {
            last_change_ = last_change;
        }

        [[nodiscard]] auto LastChange() const -> ChangeId {
            return last_change_;
        }

        [[nodiscard]] auto ChangedSince(ChangeId last_published_change) const -> bool {
            return last_change_ > last_published_change;
        }

    private:
        Price       price_;
        std::size_t order_count_;
        Quantity    aggregate_qty_;
        bool        is_excess_;
        ChangeId    last_change_;
    };
}    // namespace lhft::book
