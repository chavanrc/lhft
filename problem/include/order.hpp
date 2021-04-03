#pragma once

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "types.hpp"

namespace lhft::book {
    enum class State {
        SUBMITTED,
        REJECTED,
        ACCEPTED,
        MODIFY_REQUESTED,
        MODIFY_REJECTED,
        MODIFIED,
        PARTIAL_FILLED,
        FILLED,
        CANCEL_REQUESTED,
        CANCEL_REJECTED,
        CANCELLED,
        UNKNOWN
    };

    enum TickType : char { ORDER_TICK = 'O', TRADE_EVENT_TICK = 'T', BOOK_UPDATE = 'B', BOOK_CHANGE = 'C' };

    struct StreamHeader {
        size_t   seq_no_;
        TickType message_type_;
    };

    struct OrderData {
        StreamHeader stream_header_;
        OrderId      id_;
        bool         buy_side_;
        Symbol       symbol_;
        Quantity     quantity_;
        Price        price_;
        Quantity     quantity_filled_;
        Quantity     quantity_on_market_;
        Cost         fill_cost_;
        State        state_;
        std::string  reason_;
    };

    struct TradeData {
        StreamHeader stream_header_;
        OrderId      buyer_id_;
        OrderId      seller_id_;
        Symbol       symbol_;
        Quantity     quantity_;
        Price        price_;
        bool         buyer_maker_;
        FillId       fill_id_;
    };

    template <int32_t SIZE = BOOK_DEPTH>
    struct BookData {
        StreamHeader               stream_header_;
        Symbol                     symbol_;
        std::pair<Price, Quantity> bids_[SIZE];
        std::pair<Price, Quantity> asks_[SIZE];
    };

    struct BookChange {
        StreamHeader stream_header_;
        Symbol       symbol_;
    };

    struct StateChange {
        State       state_{State::UNKNOWN};
        std::string description_{};

        explicit StateChange(State state, const std::string& description = "")
            : state_(state), description_(description) {
        }

        friend std::ostream& operator<<(std::ostream& os, const StateChange& change) {
            os /*<< "state_: " << change.state_*/ << " description_: " << change.description_;
            return os;
        }
    };

    struct MatchedTrade {
        OrderId  matched_order_id_;
        Cost     fill_cost_;
        Quantity quantity_;
        Quantity quantity_on_market_;
        Price    price_;
        FillId   fill_id_;
    };

    class Order {
    public:
        using History = std::vector<StateChange>;
        using Trades  = std::vector<MatchedTrade>;

        Order(OrderId id, bool buy_side, Symbol symbol, Quantity quantity, Price price)
            : id_{id}, buy_side_{buy_side}, symbol_{symbol}, quantity_{quantity}, price_{price} {
        }

        [[nodiscard]] auto GetOrderId() const -> OrderId {
            return id_;
        }

        [[nodiscard]] auto IsLimit() const -> bool {
            return GetPrice() != 0;
        }

        [[nodiscard]] auto IsBuy() const -> bool {
            return buy_side_;
        }

        [[nodiscard]] auto GetSymbol() const -> Symbol {
            return symbol_;
        }

        [[nodiscard]] auto GetPrice() const -> Price {
            return price_;
        }

        [[nodiscard]] auto OrderQty() const -> Quantity {
            return quantity_;
        }

        [[nodiscard]] auto QuantityOnMarket() const -> Quantity {
            return quantity_on_market_;
        }

        [[nodiscard]] auto QuantityFilled() const -> Quantity {
            return quantity_filled_;
        }

        [[nodiscard]] auto FillCost() const -> Cost {
            return fill_cost_;
        }

        [[nodiscard]] auto GetHistory() const -> const History& {
            return history_;
        }

        [[nodiscard]] auto GetTrades() const -> const Trades& {
            return trades_;
        }

        [[nodiscard]] auto CurrentState() const -> std::optional<StateChange> {
            if (!history_.empty()) {
                return history_.back();
            }
            return {};
        }

        auto OnSubmitted() -> void {
            std::stringstream msg;
            msg << (IsBuy() ? "BUY " : "SELL ") << quantity_ << ' ' << symbol_ << " @";
            if (price_ == 0) {
                msg << "MKT";
            } else {
                msg << price_;
            }
            history_.emplace_back(State::SUBMITTED, msg.str());
        }

        auto OnAccepted() -> void {
            quantity_on_market_ = quantity_;
            history_.emplace_back(State::ACCEPTED);
        }

        auto OnRejected(const char* reason) -> void {
            history_.emplace_back(State::REJECTED, reason);
        }

        auto OnFilled(Quantity fill_qty, Cost fill_cost) -> void {
            quantity_on_market_ -= fill_qty;
            fill_cost_ += fill_cost;
            quantity_filled_ += fill_qty;

            std::stringstream msg;
            msg << fill_qty << " for " << fill_cost;
            history_.emplace_back(State::FILLED, msg.str());
        }

        auto AddTradeHistory(Quantity fill_qty, Quantity remaining_qty, Cost fill_cost, const OrderId& matched_order_id,
                             Price price, FillId fill_id) -> void {
            MatchedTrade res{};
            res.matched_order_id_   = matched_order_id;
            res.fill_cost_          = fill_cost;
            res.quantity_           = fill_qty;
            res.price_              = price;
            res.quantity_on_market_ = remaining_qty;
            res.fill_id_            = fill_id;
            trades_.emplace_back(res);
        }

        auto OnCancelRequested() -> void {
            history_.emplace_back(State::CANCEL_REQUESTED);
        }

        auto OnCancelled() -> void {
            quantity_on_market_ = 0;
            history_.emplace_back(State::CANCELLED);
        }

        auto OnCancelRejected(const char* reason) -> void {
            history_.emplace_back(State::CANCEL_REJECTED, reason);
        }

        auto OnReplaceRequested(const int64_t& size_delta, Price new_price) -> void {
            // TODO
        }

        auto OnReplaced(const int64_t& size_delta, Price new_price) -> void {
            // TODO
        }

        auto OnReplaceRejected(const char* reason) -> void {
            // TODO
        }

        auto GetOrderData(OrderData& order_data, State state, const std::string& reason) -> void {
            order_data.id_                 = GetOrderId();
            order_data.buy_side_           = IsBuy();
            order_data.symbol_             = Symbol();
            order_data.quantity_           = OrderQty();
            order_data.price_              = GetPrice();
            order_data.quantity_filled_    = QuantityFilled();
            order_data.fill_cost_          = FillCost();
            order_data.quantity_on_market_ = QuantityOnMarket();
            auto cur_state                 = CurrentState();
            if (cur_state) {
                order_data.state_ = cur_state->state_;
            }
            order_data.reason_ = reason;
        }

        friend std::ostream& operator<<(std::ostream& os, const Order& order) {
            os << "[#" << order.GetOrderId();
            os << ' ' << (order.IsBuy() ? "BUY" : "SELL");
            os << ' ' << order.OrderQty();
            os << ' ' << order.GetSymbol();
            if (order.GetPrice() == 0) {
                os << " MKT";
            } else {
                os << " $" << order.GetPrice();
            }

            auto on_market = order.QuantityOnMarket();
            if (on_market != 0) {
                os << " Open: " << on_market;
            }

            auto filled = order.QuantityFilled();
            if (filled != 0) {
                os << " FILLED: " << filled;
            }

            auto cost = order.FillCost();
            if (cost != 0) {
                os << " Cost: " << cost;
            }

            if (order.IsVerbose()) {
                const Order::History& history = order.GetHistory();
                for (const auto& event : history) {
                    os << "\n\t" << event;
                }
            } else {
                auto cur_state = order.CurrentState();
                if (cur_state) {
                    os << " Last Event: " << *cur_state;
                }
            }

            os << ']';

            return os;
        }

        [[nodiscard]] auto IsVerbose() const -> bool {
            return verbose_;
        }

    private:
        OrderId  id_;
        bool     buy_side_;
        Symbol   symbol_;
        Quantity quantity_;
        Price    price_;
        Quantity quantity_filled_{0};
        Quantity quantity_on_market_{0};
        Cost     fill_cost_{0};
        History  history_;
        Trades   trades_;
        bool     verbose_{false};
    };
}    // namespace lhft::book