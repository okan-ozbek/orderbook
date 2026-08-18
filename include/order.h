//
// Created by Dorza on 18/08/2026.
//

#ifndef ORDERBOOK_ORDER_H
#define ORDERBOOK_ORDER_H

#include <cstddef>
#include <cstdint>
#include <format>
#include <stdexcept>

using Id = std::size_t;
using Price = long;
using Quantity = std::size_t;

class Order
{
public:
    enum Side : std::uint8_t
    {
        BUY,
        SELL,
    };

    Order(const Id id, const Price price, const Quantity quantity, const Side side)
        : m_id(id)
        , m_price(price)
        , m_initial_quantity(quantity)
        , m_remaining_quantity(quantity)
        , m_side(side)
    {}

    [[nodiscard]]
    const Id& GetId() const
    {
        return m_id;
    }

    [[nodiscard]]
    const Price& GetPrice() const
    {
        return m_price;
    }

    [[nodiscard]]
    const Quantity& GetInitialQuantity() const
    {
        return m_initial_quantity;
    }

    [[nodiscard]]
    const Quantity& GetRemainingQuantity() const
    {
        return m_remaining_quantity;
    }

    [[nodiscard]]
    const Side& GetSide() const
    {
        return m_side;
    }

    [[nodiscard]]
    const Id& GetRecencyId() const
    {
        return m_recency_id;
    }

    void Fill(const Quantity& quantity)
    {
        if (quantity > m_remaining_quantity) {
            throw std::logic_error(
                std::format("Order ID: {} - Fill quantity {} exceeds remaining quantity {}", GetId(), quantity, GetRemainingQuantity())
            );
        }

        m_remaining_quantity -= quantity;
    }

    [[nodiscard]]
    bool IsFilled() const
    {
        return m_remaining_quantity == 0;
    }

    [[nodiscard]]
    bool IsBuy() const
    {
        return m_side == BUY;
    }

    [[nodiscard]]
    bool IsSell() const
    {
        return m_side == SELL;
    }

private:
    Id m_id;
    Price m_price;
    Quantity m_initial_quantity;
    Quantity m_remaining_quantity;
    Side m_side;

    Id m_recency_id{ s_next_recency_id++ };
    static inline Id s_next_recency_id{ 0 };
};

#endif //ORDERBOOK_ORDER_H
