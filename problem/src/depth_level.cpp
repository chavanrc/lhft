#include <depth_level.hpp>
#include <stdexcept>

namespace lhft::book {
    auto DepthLevel::operator=(const DepthLevel &rhs) -> DepthLevel & {
        price_ = rhs.price_;
        order_count_ = rhs.order_count_;
        aggregate_qty_ = rhs.aggregate_qty_;
        if (rhs.price_ != INVALID_LEVEL_PRICE) {
            last_change_ = rhs.last_change_;
        }
        return *this;
    }

    auto DepthLevel::GetPrice() const -> const Price & {
        return price_;
    }

    auto DepthLevel::OrderCount() const -> std::size_t {
        return order_count_;
    }

    auto DepthLevel::AggregateQty() const -> Quantity {
        return aggregate_qty_;
    }

    auto DepthLevel::IsExcess() const -> bool {
        return is_excess_;
    }

    auto DepthLevel::Init(Price price, bool is_excess) -> void {
        price_ = price;
        order_count_ = 0;
        aggregate_qty_ = 0;
        is_excess_ = is_excess;
    }

    auto DepthLevel::AddOrder(Quantity qty) -> void {
        ++order_count_;
        aggregate_qty_ += qty;
    }

    auto DepthLevel::IncreaseQty(Quantity qty) -> void {
        aggregate_qty_ += qty;
    }

    auto DepthLevel::DecreaseQty(Quantity qty) -> void {
        aggregate_qty_ -= qty;
    }

    auto DepthLevel::Set(Price price, Quantity qty, std::size_t order_count, ChangeId last_change) -> void {
        price_ = price;
        aggregate_qty_ = qty;
        order_count_ = order_count;
        last_change_ = last_change;
    }

    auto DepthLevel::CloseOrder(Quantity qty) -> bool {
        bool empty = false;
        // If this is the last order, reset the level
        if (order_count_ == 0) {
            throw std::runtime_error("DepthLevel::CloseOrder order count too low");
        } else if (order_count_ == 1) {
            order_count_ = 0;
            aggregate_qty_ = 0;
            empty = true;
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

    auto DepthLevel::LastChange(ChangeId last_change) -> void {
        last_change_ = last_change;
    }

    auto DepthLevel::LastChange() const -> ChangeId {
        return last_change_;
    }

    auto DepthLevel::ChangedSince(ChangeId last_published_change) const -> bool {
        return last_change_ > last_published_change;
    }
}    // namespace lhft::book
