#pragma once

#include <cstring>
#include <cmath>
#include <functional>
#include <map>
#include <stdexcept>

#include "depth_level.hpp"
#include "types.hpp"

namespace lhft::book {
    template <int32_t SIZE = 5>
    class Depth {
    public:
        [[nodiscard]] auto Bids() const -> const DepthLevel* {
            return levels_;
        }

        [[nodiscard]] auto LastBidLevel() const -> const DepthLevel* {
            return levels_ + (SIZE - 1);
        }

        [[nodiscard]] auto Asks() const -> const DepthLevel* {
            return levels_ + SIZE;
        }

        [[nodiscard]] auto LastAskLevel() const -> const DepthLevel* {
            return levels_ + (SIZE * 2 - 1);
        }

        [[nodiscard]] auto end() const -> const DepthLevel* {
            return levels_ + (SIZE * 2);
        }

        auto Bids() -> DepthLevel* {
            return levels_;
        }

        auto LastBidLevel() -> DepthLevel* {
            return levels_ + (SIZE - 1);
        }

        auto Asks() -> DepthLevel* {
            return levels_ + SIZE;
        }

        auto LastAskLevel() -> DepthLevel* {
            return levels_ + (SIZE * 2 - 1);
        }

        auto AddOrder(Price price, Quantity qty, bool is_bid) -> void {
            ChangeId    last_change_copy = last_change_;
            DepthLevel* level            = FindLevel(price, is_bid);
            if (level) {
                level->AddOrder(qty);
                // If this is a visible level
                if (!level->IsExcess()) {
                    // The depth changed
                    last_change_ = last_change_copy + 1;    // Ensure incremented
                    level->LastChange(last_change_copy + 1);
                }
                // The level is not marked as changed if it is not visible
            }
        }

        auto IgnoreFillQty(Quantity qty, bool is_bid) -> void {
            if (is_bid) {
                if (ignore_bid_fill_qty_) {
                    throw std::runtime_error("Unexpected ignore_bid_fill_qty_");
                }
                ignore_bid_fill_qty_ = qty;
            } else {
                if (ignore_ask_fill_qty_) {
                    throw std::runtime_error("Unexpected ignore_ask_fill_qty_");
                }
                ignore_ask_fill_qty_ = qty;
            }
        }

        auto FillOrder(Price price, Quantity fill_qty, bool filled, bool is_bid) -> void {
            if (is_bid && ignore_bid_fill_qty_) {
                ignore_bid_fill_qty_ -= fill_qty;
            } else if ((!is_bid) && ignore_ask_fill_qty_) {
                ignore_ask_fill_qty_ -= fill_qty;
            } else if (filled) {
                CloseOrder(price, fill_qty, is_bid);
            } else {
                ChangeQtyOrder(price, -(int64_t)fill_qty, is_bid);
            }
        }

        auto CloseOrder(Price price, Quantity open_qty, bool is_bid) -> bool {
            DepthLevel* level = FindLevel(price, is_bid, false);
            if (level) {
                if (level->CloseOrder(open_qty)) {
                    EraseLevel(level, is_bid);
                    return true;
                } else {
                    level->LastChange(++last_change_);
                }
            }
            return false;
        }

        auto ChangeQtyOrder(Price price, int64_t qty_delta, bool is_bid) -> void {
            DepthLevel* level = FindLevel(price, is_bid, false);
            if (level && qty_delta) {
                if (qty_delta > 0) {
                    level->IncreaseQty(Quantity(qty_delta));
                } else {
                    level->DecreaseQty(Quantity(std::abs(qty_delta)));
                }
                level->LastChange(++last_change_);
            }
            // Ignore if not found - may be beyond our depth size
        }

        auto ReplaceOrder(Price current_price, Price new_price, Quantity current_qty, Quantity new_qty, bool is_bid)
                -> bool {
            bool erased = false;
            // If the price is unchanged, modify this level only
            if (current_price == new_price) {
                int64_t qty_delta = ((int64_t)new_qty) - current_qty;
                ChangeQtyOrder(current_price, qty_delta, is_bid);
            } else {
                AddOrder(new_price, new_qty, is_bid);
                erased = CloseOrder(current_price, current_qty, is_bid);
            }
            return erased;
        }

        auto NeedsBidRestoration(Price& restoration_price) -> bool {
            if (SIZE > 1) {
                restoration_price = (LastBidLevel() - 1)->GetPrice();
                return restoration_price != INVALID_LEVEL_PRICE;
            } else if (SIZE == 1) {
                restoration_price = MARKET_ORDER_BID_SORT_PRICE;
                return true;
            }
            throw std::runtime_error("Depth size less than one not allowed");
        }

        auto NeedsAskRestoration(Price& restoration_price) -> bool {
            if (SIZE > 1) {
                restoration_price = (LastAskLevel() - 1)->GetPrice();
                return restoration_price != INVALID_LEVEL_PRICE;
            } else if (SIZE == 1) {
                restoration_price = MARKET_ORDER_ASK_SORT_PRICE;
                return true;
            }
            throw std::runtime_error("Depth size less than one not allowed");
        }

        [[nodiscard]] auto Changed() const -> bool {
            return last_change_ > last_published_change_;
        }

        [[nodiscard]] auto LastChange() const -> ChangeId {
            return last_change_;
        }

        [[nodiscard]] auto LastPublishedChange() const -> ChangeId {
            return last_published_change_;
        }

        auto Published() -> void {
            last_published_change_ = last_change_;
        }

    private:
        DepthLevel levels_[SIZE * 2]{};
        ChangeId   last_change_{0};
        ChangeId   last_published_change_{0};
        Quantity   ignore_bid_fill_qty_{0};
        Quantity   ignore_ask_fill_qty_{0};

        using BidLevelMap = std::map<Price, DepthLevel, std::greater<Price>>;
        using AskLevelMap = std::map<Price, DepthLevel, std::less<Price>>;
        BidLevelMap excess_bid_levels_;
        AskLevelMap excess_ask_levels_;

        auto FindLevel(Price price, bool is_bid, bool should_create = true) -> DepthLevel* {
            DepthLevel*       level    = is_bid ? Bids() : Asks();
            const DepthLevel* past_end = is_bid ? Asks() : end();
            for (; level != past_end; ++level) {
                if (level->GetPrice() == price) {
                    break;
                } else if (should_create && level->GetPrice() == INVALID_LEVEL_PRICE) {
                    level->Init(price, false);
                    break;
                } else if (is_bid && should_create && level->GetPrice() < price) {
                    InsertLevelBefore(level, is_bid, price);
                    break;
                } else if (!is_bid && should_create && level->GetPrice() > price) {
                    InsertLevelBefore(level, is_bid, price);
                    break;
                }
            }
            // If level was not found
            if (level == past_end) {
                if (is_bid) {
                    // Search in excess bid levels
                    auto find_result = excess_bid_levels_.find(price);
                    // If found in excess levels, return location
                    if (find_result != excess_bid_levels_.end()) {
                        level = &find_result->second;
                    } else if (should_create) {
                        DepthLevel new_level;
                        new_level.Init(price, true);
                        std::pair<BidLevelMap::iterator, bool> insert_result;
                        insert_result = excess_bid_levels_.insert({price, new_level});
                        level         = &insert_result.first->second;
                    }
                } else {
                    auto find_result = excess_ask_levels_.find(price);
                    if (find_result != excess_ask_levels_.end()) {
                        level = &find_result->second;
                    } else if (should_create) {
                        DepthLevel new_level;
                        new_level.Init(price, true);
                        std::pair<AskLevelMap::iterator, bool> insert_result;
                        insert_result = excess_ask_levels_.insert({price, new_level});
                        level         = &insert_result.first->second;
                    }
                }
            }
            return level;
        }

        auto InsertLevelBefore(DepthLevel* level, bool is_bid, Price price) -> void {
            DepthLevel* last_side_level = is_bid ? LastBidLevel() : LastAskLevel();

            if (last_side_level->GetPrice() != INVALID_LEVEL_PRICE) {
                DepthLevel excess_level;
                excess_level.Init(0, true);
                excess_level = *last_side_level;
                if (is_bid) {
                    excess_bid_levels_.insert({last_side_level->GetPrice(), excess_level});
                } else {
                    excess_ask_levels_.insert({last_side_level->GetPrice(), excess_level});
                }
            }
            DepthLevel* current_level = last_side_level - 1;
            ++last_change_;
            while (current_level >= level) {
                *(current_level + 1) = *current_level;
                if (current_level->GetPrice() != INVALID_LEVEL_PRICE) {
                    (current_level + 1)->LastChange(last_change_);
                }
                --current_level;
            }
            level->Init(price, false);
        }

        auto EraseLevel(DepthLevel* level, bool is_bid) -> void {
            if (level->IsExcess()) {
                if (is_bid) {
                    excess_bid_levels_.erase(level->GetPrice());
                } else {
                    excess_ask_levels_.erase(level->GetPrice());
                }
            } else {
                DepthLevel* last_side_level = is_bid ? LastBidLevel() : LastAskLevel();
                ++last_change_;
                DepthLevel* current_level = level;
                while (current_level < last_side_level) {
                    if ((current_level->GetPrice() != INVALID_LEVEL_PRICE) || (current_level == level)) {
                        *current_level = *(current_level + 1);
                        current_level->LastChange(last_change_);
                    }
                    ++current_level;
                }

                if ((level == last_side_level) || (last_side_level->GetPrice() != INVALID_LEVEL_PRICE)) {
                    if (is_bid) {
                        auto best_bid = excess_bid_levels_.begin();
                        if (best_bid != excess_bid_levels_.end()) {
                            *last_side_level = best_bid->second;
                            excess_bid_levels_.erase(best_bid);
                        } else {
                            last_side_level->Init(INVALID_LEVEL_PRICE, false);
                            last_side_level->LastChange(last_change_);
                        }
                    } else {
                        auto best_ask = excess_ask_levels_.begin();
                        if (best_ask != excess_ask_levels_.end()) {
                            *last_side_level = best_ask->second;
                            excess_ask_levels_.erase(best_ask);
                        } else {
                            last_side_level->Init(INVALID_LEVEL_PRICE, false);
                            last_side_level->LastChange(last_change_);
                        }
                    }
                    last_side_level->LastChange(last_change_);
                }
            }
        }
    };
}    // namespace lhft::book
