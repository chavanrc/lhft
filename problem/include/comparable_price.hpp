#pragma once

#include <iostream>

#include "types.hpp"

namespace lhft::book {
    class ComparablePrice {
    public:
        ComparablePrice(bool buy_side, Price price);

        [[nodiscard]] auto Matches(Price rhs) const -> bool;

        auto operator<(Price rhs) const -> bool;

        auto operator==(Price rhs) const -> bool;

        auto operator!=(Price rhs) const -> bool;

        auto operator>(Price rhs) const -> bool;

        auto operator<=(Price rhs) const -> bool;

        auto operator>=(Price rhs) const -> bool;

        auto operator<(const ComparablePrice& rhs) const -> bool;

        auto operator==(const ComparablePrice& rhs) const -> bool;

        auto operator!=(const ComparablePrice& rhs) const -> bool;

        auto operator>(const ComparablePrice& rhs) const -> bool;

        [[nodiscard]] auto GetPrice() const -> Price;

        [[nodiscard]] auto IsBuy() const -> bool;

        [[nodiscard]] auto IsMarket() const -> bool;

        friend std::ostream &operator<<(std::ostream &os, const ComparablePrice &price);

    private:
        Price price_;
        bool  buy_side_;
    };

    inline auto operator<(Price price, const ComparablePrice& key) -> bool {
        return key > price;
    }

    inline auto operator>(Price price, const ComparablePrice& key) -> bool {
        return key < price;
    }

    inline auto operator==(Price price, const ComparablePrice& key) -> bool {
        return key == price;
    }

    inline auto operator!=(Price price, const ComparablePrice& key) -> bool {
        return key != price;
    }

    inline auto operator<=(Price price, const ComparablePrice& key) -> bool {
        return key >= price;
    }

    inline auto operator>=(Price price, const ComparablePrice& key) -> bool {
        return key <= price;
    }
}    // namespace lhft::book
