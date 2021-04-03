namespace lhft::book {
    template<class OrderPtr>
    OrderBook<OrderPtr>::OrderBook(Symbol symbol) : symbol_(symbol) {
    }

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::SetSymbol(Symbol symbol) -> void {
        symbol_ = symbol;
    }

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::GetSymbol() const -> Symbol {
        return symbol_;
    }

    template<class OrderPtr>
    [[nodiscard]] auto OrderBook<OrderPtr>::Add(const OrderPtr &order) -> bool {
        bool matched = false;

        if (order->OrderQty() <= 0) {
            std::cout << "size must be positive\n";
        } else {
            Tracker inbound(order);
            matched = SubmitOrder(inbound);
        }
        return matched;
    }

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::Cancel(const OrderPtr &order) -> void {
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

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::Replace(const OrderPtr &order, int64_t size_delta, Price new_price) -> void {
        // TODO
    }

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::MarketPrice(Price price) -> void {
        market_price_ = price;
    }

    template<class OrderPtr>
    [[nodiscard]] auto OrderBook<OrderPtr>::MarketPrice() const -> Price {
        return market_price_;
    }

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::GetBids() const -> const TrackerMap & {
        return bids_;
    };

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::GetAsks() const -> const TrackerMap & {
        return asks_;
    };

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::MatchOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool {
        return MatchRegularOrder(inbound, inbound_price, current_orders);
    }

    template<class OrderPtr>
    auto
    OrderBook<OrderPtr>::MatchRegularOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool {
        bool matched = false;
        Quantity inbound_qty = inbound.OpenQty();
        typename TrackerMap::iterator pos = current_orders.begin();
        while (pos != current_orders.end() && !inbound.Filled()) {
            auto entry = pos++;
            const ComparablePrice &current_price = entry->first;
            if (!current_price.Matches(inbound_price)) {
                break;
            }

            Tracker &current_order = entry->second;
            Quantity current_quantity = current_order.OpenQty();
            Quantity traded = CreateTrade(inbound, current_order);
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

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::CreateTrade(Tracker &inbound_tracker, Tracker &current_tracker, Quantity max_quantity)
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

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::FindOnMarket(const OrderPtr &order, typename TrackerMap::iterator &result) -> bool {
        const ComparablePrice KEY(order->IsBuy(), order->GetPrice());
        TrackerMap &side_map = order->IsBuy() ? bids_ : asks_;

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

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::AllOrderCancel() -> std::vector<OrderId> {
        std::vector<OrderId> order_id_list;
        std::vector<OrderPtr> orders;

        order_id_list.reserve(asks_.size() + bids_.size());
        orders.reserve(asks_.size() + bids_.size());

        for (auto ask = asks_.rbegin(); ask != asks_.rend(); ++ask) {
            orders.emplace_back(ask->second.Ptr());
        }
        for (auto bid = bids_.begin(); bid != bids_.end(); ++bid) {
            orders.emplace_back(bid->second.Ptr());
        }
        for (const auto &order : orders) {
            Cancel(order);
            order_id_list.emplace_back(order->GetOrderId());
        }
        return order_id_list;
    }

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::SubmitOrder(Tracker &inbound) -> bool {
        Price order_price = inbound.Ptr()->GetPrice();
        return AddOrder(inbound, order_price);
    }

    template<class OrderPtr>
    auto OrderBook<OrderPtr>::AddOrder(Tracker &inbound, Price order_price) -> bool {
        bool matched = false;
        OrderPtr &order = inbound.Ptr();
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
}    // namespace lhft::book