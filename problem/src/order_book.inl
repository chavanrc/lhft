namespace lhft::book {
    template <class OrderPtr>
    OrderBook<OrderPtr>::OrderBook(Symbol symbol) : symbol_(symbol) {
        callbacks_.reserve(16);
        working_callbacks_.reserve(callbacks_.capacity());
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::SetSymbol(Symbol symbol) -> void {
        symbol_ = symbol;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::GetSymbol() const -> Symbol {
        return symbol_;
    }

    template <class OrderPtr>
    [[nodiscard]] auto OrderBook<OrderPtr>::Add(const OrderPtr &order) -> bool {
        bool matched = false;

        if (order->OrderQty() <= 0) {
            callbacks_.push_back(TypedCallback::Reject(order, "size must be positive"));
        } else {
            size_t accept_cb_index = callbacks_.size();
            callbacks_.push_back(TypedCallback::Accept(order));
            Tracker inbound(order);
            matched                               = SubmitOrder(inbound);
            callbacks_[accept_cb_index].quantity_ = inbound.FilledQty();
            callbacks_.push_back(TypedCallback::BookUpdate(this));
        }
        CallbackNow();
        return matched;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::Cancel(const OrderPtr &order) -> void {
        bool     found    = false;
        Quantity open_qty = 0;
        if (order->IsBuy()) {
            typename TrackerMap::iterator bid;
            FindOnMarket(order, bid);
            if (bid != bids_.end()) {
                open_qty = bid->second.OpenQty();
                bids_.erase(bid);
                found = true;
            }
        } else {
            typename TrackerMap::iterator ask;
            FindOnMarket(order, ask);
            if (ask != asks_.end()) {
                open_qty = ask->second.OpenQty();
                asks_.erase(ask);
                found = true;
            }
        }
        if (found) {
            callbacks_.push_back(TypedCallback::Cancel(order, open_qty));
            callbacks_.push_back(TypedCallback::BookUpdate(this));
        } else {
            callbacks_.push_back(TypedCallback::CancelReject(order, "not found"));
        }
        CallbackNow();
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::Replace(const OrderPtr &order, int64_t size_delta, Price new_price) -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::MarketPrice(Price price) -> void {
        market_price_ = price;
    }

    template <class OrderPtr>
    [[nodiscard]] auto OrderBook<OrderPtr>::MarketPrice() const -> Price {
        return market_price_;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::GetBids() const -> const TrackerMap & {
        return bids_;
    };

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::GetAsks() const -> const TrackerMap & {
        return asks_;
    };

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::MatchOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool {
        return MatchRegularOrder(inbound, inbound_price, current_orders);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::MatchRegularOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders)
            -> bool {
        bool                          matched     = false;
        Quantity                      inbound_qty = inbound.OpenQty();
        typename TrackerMap::iterator pos         = current_orders.begin();
        while (pos != current_orders.end() && !inbound.Filled()) {
            auto                   entry         = pos++;
            const ComparablePrice &current_price = entry->first;
            if (!current_price.Matches(inbound_price)) {
                break;
            }

            Tracker &current_order    = entry->second;
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

    template <class OrderPtr>
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
        Quantity fill_qty = (std::min)(max_quantity, (std::min)(inbound_tracker.OpenQty(), current_tracker.OpenQty()));
        if (fill_qty > 0) {
            inbound_tracker.Fill(fill_qty);
            current_tracker.Fill(fill_qty);
            MarketPrice(cross_price);

            typename TypedCallback::FillFlags fill_flags = TypedCallback::FF_NEITHER_FILLED;
            if (!inbound_tracker.OpenQty()) {
                fill_flags = (typename TypedCallback::FillFlags)(fill_flags | TypedCallback::FF_INBOUND_FILLED);
            }
            if (!current_tracker.OpenQty()) {
                fill_flags = (typename TypedCallback::FillFlags)(fill_flags | TypedCallback::FF_MATCHED_FILLED);
            }

            callbacks_.push_back(TypedCallback::Fill(inbound_tracker.Ptr(), current_tracker.Ptr(), fill_qty,
                                                     cross_price, fill_flags));
        }
        return fill_qty;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::FindOnMarket(const OrderPtr &order, typename TrackerMap::iterator &result) -> bool {
        const ComparablePrice KEY(order->IsBuy(), order->GetPrice());
        TrackerMap &          side_map = order->IsBuy() ? bids_ : asks_;

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

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::AllOrderCancel() -> std::vector<OrderId> {
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
        for (const auto &order : orders) {
            Cancel(order);
            order_id_list.emplace_back(order->GetOrderId());
        }
        return order_id_list;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::SubmitOrder(Tracker &inbound) -> bool {
        Price order_price = inbound.Ptr()->GetPrice();
        return AddOrder(inbound, order_price);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::AddOrder(Tracker &inbound, Price order_price) -> bool {
        bool      matched = false;
        OrderPtr &order   = inbound.Ptr();
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

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnAccept(const OrderPtr &order, Quantity quantity) -> void {
        order->OnAccepted();
        std::cout << "Event: Accepted: " << *order << '\n';
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnReject(const OrderPtr &order, const char *reason) -> void {
        order->OnRejected(reason);
        std::cout << "Event: Rejected: " << *order << ' ' << reason << '\n';
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnFill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty,
                                     Cost fill_cost, FillId fill_id) -> void {
        order->OnFilled(fill_qty, fill_cost);
        matched_order->OnFilled(fill_qty, fill_cost);

        std::stringstream out;
        out << (order->IsBuy() ? "Event: Fill-Bought: " : "Event: Fill-Sold: ") << fill_qty << " Shares for "
            << fill_cost << ' ' << *order;
        out << (matched_order->IsBuy() ? " Bought: " : " Sold: ") << fill_qty << " Shares for " << fill_cost << ' '
            << *matched_order;
        std::cout << out.str() << '\n';

        order->AddTradeHistory(fill_qty, matched_order->QuantityOnMarket(), fill_cost, matched_order->GetOrderId(),
                               matched_order->GetPrice(), fill_id);
        matched_order->AddTradeHistory(fill_qty, order->QuantityOnMarket(), fill_cost, order->GetOrderId(),
                                       order->GetPrice(), fill_id);
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnCancel(const OrderPtr &order, Quantity quantity) -> void {
        order->OnCancelled();
        std::cout << "Event: Canceled: " << *order;
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnCancelReject(const OrderPtr &order, const char *reason) -> void {
        order->OnCancelRejected(reason);
        std::cout << "Event: Cancel Reject: " << *order << ' ' << reason << '\n';
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnReplace(const OrderPtr &order, Quantity current_qty, Quantity new_qty, Price new_price)
            -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnReplaceReject(const OrderPtr &order, const char *reason) -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnOrderBookChange() -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnStopLossTriggered(const OrderId &order_id) -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::OnTrade(const OrderBook *book, const OrderId &id_1, const OrderId &id_2, Quantity qty,
                                      Price price, bool buyer_maker) -> void {
        // TODO
    }

    template <class OrderPtr>
    auto OrderBook<OrderPtr>::CallbackNow() -> void {
        if (!handling_callbacks_) {
            handling_callbacks_ = true;
            while (!callbacks_.empty()) {
                working_callbacks_.reserve(callbacks_.capacity());
                working_callbacks_.swap(callbacks_);
                for (auto &cb : working_callbacks_) {
                    try {
                        PerformCallback(cb);
                    } catch (const std::exception &ex) {
                        std::cerr << "Caught exception during callback: " << ex.what() << '\n';
                    } catch (...) {
                        std::cerr << "Caught unknown exception during callback" << '\n';
                    }
                }
                working_callbacks_.clear();
            }
            handling_callbacks_ = false;
        }
    }

    template <class OrderPtr>
    void OrderBook<OrderPtr>::PerformCallback(TypedCallback &cb) {
        switch (cb.type_) {
            case TypedCallback::CbType::CB_ORDER_FILL: {
                Cost fill_cost      = cb.price_ * cb.quantity_;
                bool inbound_filled = (cb.flags_ & (TypedCallback::FillFlags::FF_INBOUND_FILLED |
                                                    TypedCallback::FillFlags::FF_BOTH_FILLED)) != 0;
                bool matched_filled = (cb.flags_ & (TypedCallback::FillFlags::FF_MATCHED_FILLED |
                                                    TypedCallback::FillFlags::FF_BOTH_FILLED)) != 0;
                // generate new trade id
                static FillId fill_id{0};
                ++fill_id;
                OnFill(cb.order_, cb.matched_order_, cb.quantity_, fill_cost, fill_id);
                OrderId buy_order_id, sell_order_id;
                if (cb.matched_order_->IsBuy()) {
                    buy_order_id  = cb.matched_order_->GetOrderId();
                    sell_order_id = cb.order_->GetOrderId();
                } else {
                    buy_order_id  = cb.order_->GetOrderId();
                    sell_order_id = cb.matched_order_->GetOrderId();
                }
                bool buyer_maker = cb.matched_order_->IsBuy() ? true : false;
                OnTrade(this, buy_order_id, sell_order_id, cb.quantity_, cb.price_, buyer_maker);
                break;
            }
            case TypedCallback::CbType::CB_ORDER_ACCEPT:
                OnAccept(cb.order_, cb.quantity_);
                break;
            case TypedCallback::CbType::CB_ORDER_REJECT:
                OnReject(cb.order_, cb.reject_reason_);
                break;
            case TypedCallback::CbType::CB_ORDER_CANCEL:
                OnCancel(cb.order_, cb.quantity_);
                break;
            case TypedCallback::CbType::CB_ORDER_CANCEL_REJECT:
                OnCancelReject(cb.order_, cb.reject_reason_);
                break;
            case TypedCallback::CbType::CB_ORDER_REPLACE:
                OnReplace(cb.order_, cb.quantity_, cb.quantity_ + cb.delta_, cb.price_);
                break;
            case TypedCallback::CbType::CB_ORDER_REPLACE_REJECT:
                OnReplaceReject(cb.order_, cb.reject_reason_);
                break;
            case TypedCallback::CbType::CB_BOOK_UPDATE:
                OnOrderBookChange();
                break;
            case TypedCallback::CbType::CB_SL_TRIGGERED:
                OnStopLossTriggered(cb.order_id_);
                break;
            default: {
                std::stringstream msg;
                msg << "Unexpected callback type " << cb.type_;
                throw std::runtime_error(msg.str());
            }
        }
    }

    template <class OrderPtr>
    void OrderBook<OrderPtr>::Log() const {
        std::cout << "Symbol " << symbol_ << '\n';
        std::cout << "Market Price " << market_price_ << '\n';
        for (auto ask = asks_.rbegin(); ask != asks_.rend(); ++ask) {
            std::cout << "  Ask " << ask->second.OpenQty() << " @ " << ask->first << '\n';
        }

        for (auto bid = bids_.begin(); bid != bids_.end(); ++bid) {
            std::cout << "  Bid " << bid->second.OpenQty() << " @ " << bid->first << '\n';
        }
    }
}    // namespace lhft::book