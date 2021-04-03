#pragma once

#include <cstring>
#include <cmath>
#include <functional>
#include <map>
#include <stdexcept>

#include "depth_level.hpp"
#include "types.hpp"

namespace lhft::book {
    template<int32_t SIZE = 5>
    class Depth {
    public:
        [[nodiscard]] auto Bids() const -> const DepthLevel *;

        [[nodiscard]] auto LastBidLevel() const -> const DepthLevel *;

        [[nodiscard]] auto Asks() const -> const DepthLevel *;

        [[nodiscard]] auto LastAskLevel() const -> const DepthLevel *;

        [[nodiscard]] auto end() const -> const DepthLevel *;

        auto Bids() -> DepthLevel *;

        auto LastBidLevel() -> DepthLevel *;

        auto Asks() -> DepthLevel *;

        auto LastAskLevel() -> DepthLevel *;

        auto AddOrder(Price price, Quantity qty, bool is_bid) -> void;

        auto IgnoreFillQty(Quantity qty, bool is_bid) -> void;

        auto FillOrder(Price price, Quantity fill_qty, bool filled, bool is_bid) -> void;

        auto CloseOrder(Price price, Quantity open_qty, bool is_bid) -> bool;

        auto ChangeQtyOrder(Price price, int64_t qty_delta, bool is_bid) -> void;

        auto ReplaceOrder(Price current_price, Price new_price, Quantity current_qty, Quantity new_qty, bool is_bid)
        -> bool;

        auto NeedsBidRestoration(Price &restoration_price) -> bool;

        auto NeedsAskRestoration(Price &restoration_price) -> bool;

        [[nodiscard]] auto Changed() const -> bool;

        [[nodiscard]] auto LastChange() const -> ChangeId;

        [[nodiscard]] auto LastPublishedChange() const -> ChangeId;

        auto Published() -> void;

    private:
        DepthLevel levels_[SIZE * 2]{};
        ChangeId last_change_{0};
        ChangeId last_published_change_{0};
        Quantity ignore_bid_fill_qty_{0};
        Quantity ignore_ask_fill_qty_{0};

        using BidLevelMap = std::map<Price, DepthLevel, std::greater<Price>>;
        using AskLevelMap = std::map<Price, DepthLevel, std::less<Price>>;
        BidLevelMap excess_bid_levels_;
        AskLevelMap excess_ask_levels_;

        auto FindLevel(Price price, bool is_bid, bool should_create = true) -> DepthLevel *;

        auto InsertLevelBefore(DepthLevel *level, bool is_bid, Price price) -> void;

        auto EraseLevel(DepthLevel *level, bool is_bid) -> void;
    };
}    // namespace lhft::book

#include "../src/depth.inl"