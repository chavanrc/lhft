#pragma once
#include <list>
#include <map>
#include <vector>

#include "order_tracker.hpp"
#include "types.hpp"

namespace lhft::book {
    template <typename OrderPtr>
    class OrderBook {
    public:
        using Tracker         = OrderTracker<OrderPtr>;
        using TrackerMap      = std::multimap<ComparablePrice, Tracker>;
        using DeferredMatches = std::list<typename TrackerMap::iterator>;
        using TrackerVec      = std::vector<Tracker>;
        using Bids            = TrackerMap;
        using Asks            = TrackerMap;

        explicit OrderBook(Symbol symbol = 0) : symbol_(symbol), market_price_(MARKET_ORDER_PRICE) {
        }

        auto SetSymbol(Symbol symbol) -> void {
            symbol_ = symbol;
        }

        [[nodiscard]] auto GetSymbol() const -> Symbol {
            return symbol_;
        }

        [[nodiscard]] auto Add(const OrderPtr& order) -> bool {
            bool matched = false;

            if (order->OrderQty() <= 0) {
                std::cout << "size must be positive\n";
            } else {
                Tracker inbound(order);
                matched = SubmitOrder(inbound);
            }
            return matched;
        }

        auto Cancel(const OrderPtr& order) -> void {
            if (order->IsBuy()) {
                typename TrackerMap::iterator bid;
                FindOnMarket(order, bid);
                if (bid != bids_.end()) {
                    bids_.erase(bid);
                }
            } else {
                typename TrackerMap::iterator ask;
                FindOnMarket(order, ask);
                if (ask != asks_.end()) {
                    asks_.erase(ask);
                }
            }
        }

        auto Replace(const OrderPtr& order, int64_t size_delta = SIZE_UNCHANGED, Price new_price = PRICE_UNCHANGED)
                -> void {
            // TODO
        }

        auto MarketPrice(Price price) -> void {
            market_price_ = price;
        }

        [[nodiscard]] auto MarketPrice() const -> Price {
            return market_price_;
        }

        auto GetBids() const -> const TrackerMap& {
            return bids_;
        };

        auto GetAsks() const -> const TrackerMap& {
            return asks_;
        };

        auto MatchOrder(Tracker& inbound, Price inbound_price, TrackerMap& current_orders) -> bool {
            return MatchRegularOrder(inbound, inbound_price, current_orders);
        }

        auto MatchRegularOrder(Tracker& inbound, Price inbound_price, TrackerMap& current_orders) -> bool {
            bool                          matched     = false;
            Quantity                      inbound_qty = inbound.OpenQty();
            typename TrackerMap::iterator pos         = current_orders.begin();
            while (pos != current_orders.end() && !inbound.Filled()) {
                auto                   entry         = pos++;
                const ComparablePrice& current_price = entry->first;
                if (!current_price.Matches(inbound_price)) {
                    break;
                }

                Tracker& current_order    = entry->second;
                Quantity current_quantity = current_order.OpenQty();
                Quantity traded           = CreateTrade(inbound, current_order);
                if (traded > 0) {
                    matched = true;
                    if (current_order.Filled()) {
                        current_orders.erase(entry);
                    }
                    inbound_qty -= traded;
                }
            }
            return matched;
        }

        auto CreateTrade(Tracker& inbound_tracker, Tracker& current_tracker, Quantity max_quantity = UINT64_MAX)
                -> Quantity {
            Price cross_price = current_tracker.Ptr()->GetPrice();
            // If current order is a market order, cross at inbound price
            if (MARKET_ORDER_PRICE == cross_price) {
                cross_price = inbound_tracker.Ptr()->GetPrice();
            }
            if (MARKET_ORDER_PRICE == cross_price) {
                cross_price = market_price_;
            }
            if (MARKET_ORDER_PRICE == cross_price) {
                // No price available for this order
                return 0;
            }
            Quantity fill_qty =
                    (std::min)(max_quantity, (std::min)(inbound_tracker.OpenQty(), current_tracker.OpenQty()));
            if (fill_qty > 0) {
                inbound_tracker.Fill(fill_qty);
                current_tracker.Fill(fill_qty);
                MarketPrice(cross_price);
            }
            return fill_qty;
        }

        auto FindOnMarket(const OrderPtr& order, typename TrackerMap::iterator& result) -> bool {
            const ComparablePrice KEY(order->IsBuy(), order->GetPrice());
            TrackerMap&           side_map = order->IsBuy() ? bids_ : asks_;

            for (result = side_map.find(KEY); result != side_map.end(); ++result) {
                if (result->second.Ptr() == order) {
                    return true;
                } else if (KEY < result->first) {
                    result = side_map.end();
                    return false;
                }
            }
            return false;
        }

        auto AllOrderCancel() -> std::vector<OrderId> {
            std::vector<OrderId>  order_id_list;
            std::vector<OrderPtr> orders;

            order_id_list.reserve(asks_.size() + bids_.size());
            orders.reserve(asks_.size() + bids_.size());

            for (auto ask = asks_.rbegin(); ask != asks_.rend(); ++ask) {
                orders.emplace_back(ask->second.Ptr());
            }
            for (auto bid = bids_.begin(); bid != bids_.end(); ++bid) {
                orders.emplace_back(bid->second.Ptr());
            }
            for (const auto& order : orders) {
                Cancel(order);
                order_id_list.emplace_back(order->GetOrderId());
            }
            return order_id_list;
        }

    private:
        auto SubmitOrder(Tracker& inbound) -> bool {
            Price order_price = inbound.Ptr()->GetPrice();
            return AddOrder(inbound, order_price);
        }

        auto AddOrder(Tracker& inbound, Price order_price) -> bool {
            bool      matched = false;
            OrderPtr& order   = inbound.Ptr();
            if (order->IsBuy()) {
                matched = MatchOrder(inbound, order_price, asks_);
            } else {
                matched = MatchOrder(inbound, order_price, bids_);
            }

            if (inbound.OpenQty()) {
                if (order->IsBuy()) {
                    bids_.insert({ComparablePrice(true, order_price), inbound});
                } else {
                    asks_.insert({ComparablePrice(false, order_price), inbound});
                }
            }
            return matched;
        }

        Symbol     symbol_;
        TrackerMap bids_;
        TrackerMap asks_;
        Price      market_price_;
    };
}    // namespace lhft::book