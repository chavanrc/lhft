#pragma once

#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

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

    enum TickType : char {
        ORDER_TICK = 'O', TRADE_EVENT_TICK = 'T', BOOK_UPDATE = 'B', BOOK_CHANGE = 'C'
    };

    struct StreamHeader {
        size_t seq_no_;
        TickType message_type_;
    };

    struct OrderData {
        StreamHeader stream_header_;
        OrderId id_;
        bool buy_side_;
        Symbol symbol_;
        Quantity quantity_;
        Price price_;
        Quantity quantity_filled_;
        Quantity quantity_on_market_;
        Cost fill_cost_;
        State state_;
        std::string reason_;
    };

    struct TradeData {
        StreamHeader stream_header_;
        OrderId buyer_id_;
        OrderId seller_id_;
        Symbol symbol_;
        Quantity quantity_;
        Price price_;
        bool buyer_maker_;
        FillId fill_id_;
    };

    template<int32_t SIZE = BOOK_DEPTH>
    struct BookData {
        StreamHeader stream_header_;
        Symbol symbol_;
        std::pair<Price, Quantity> bids_[SIZE];
        std::pair<Price, Quantity> asks_[SIZE];
    };

    struct BookChange {
        StreamHeader stream_header_;
        Symbol symbol_;
    };

    struct StateChange {
        State state_{State::UNKNOWN};
        std::string description_{};

        explicit StateChange(State state, const std::string &description = "")
                : state_(state), description_(description) {
        }

        friend std::ostream &operator<<(std::ostream &os, const StateChange &change) {
            os /*<< "state_: " << change.state_*/ << " description_: " << change.description_;
            return os;
        }
    };

    struct MatchedTrade {
        OrderId matched_order_id_;
        Cost fill_cost_;
        Quantity quantity_;
        Quantity quantity_on_market_;
        Price price_;
        FillId fill_id_;
    };

    class Order {
    public:
        using History = std::vector<StateChange>;
        using Trades = std::vector<MatchedTrade>;

        Order(OrderId id, bool buy_side, Symbol symbol, Quantity quantity, Price price);

        [[nodiscard]] auto GetOrderId() const -> OrderId;

        [[nodiscard]] auto IsLimit() const -> bool;

        [[nodiscard]] auto IsBuy() const -> bool;

        [[nodiscard]] auto GetSymbol() const -> Symbol;

        [[nodiscard]] auto GetPrice() const -> Price;

        [[nodiscard]] auto OrderQty() const -> Quantity;

        [[nodiscard]] auto QuantityOnMarket() const -> Quantity;

        [[nodiscard]] auto QuantityFilled() const -> Quantity;

        [[nodiscard]] auto FillCost() const -> Cost;

        [[nodiscard]] auto GetHistory() const -> const History &;

        [[nodiscard]] auto GetTrades() const -> const Trades &;

        [[nodiscard]] auto CurrentState() const -> std::optional<StateChange>;

        auto OnSubmitted() -> void;

        auto OnAccepted() -> void;

        auto OnRejected(const char *reason) -> void;

        auto OnFilled(Quantity fill_qty, Cost fill_cost) -> void;

        auto AddTradeHistory(Quantity fill_qty, Quantity remaining_qty, Cost fill_cost, const OrderId &matched_order_id,
                             Price price, FillId fill_id) -> void;

        auto OnCancelRequested() -> void;

        auto OnCancelled() -> void;

        auto OnCancelRejected(const char *reason) -> void;

        auto OnReplaceRequested(const int64_t &size_delta, Price new_price) -> void;

        auto OnReplaced(const int64_t &size_delta, Price new_price) -> void;

        auto OnReplaceRejected(const char *reason) -> void;

        auto GetOrderData(OrderData &order_data, State state, const std::string &reason) -> void;

        friend std::ostream &operator<<(std::ostream &os, const Order &order);

        [[nodiscard]] auto IsVerbose() const -> bool;

    private:
        OrderId id_;
        bool buy_side_;
        Symbol symbol_;
        Quantity quantity_;
        Price price_;
        Quantity quantity_filled_{0};
        Quantity quantity_on_market_{0};
        Cost fill_cost_{0};
        History history_;
        Trades trades_;
        bool verbose_{false};
    };
}    // namespace lhft::book