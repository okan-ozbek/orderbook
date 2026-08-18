//
// Test-only accessor for Orderbook's private state.
//
// This exists purely so tests can assert on internal book structure
// (price levels, quantity levels, resting order counts) without adding
// any public getters to the production Orderbook API.
//
// Requires ORDERBOOK_ENABLE_TEST_HOOKS to be defined (see tests/CMakeLists.txt),
// which makes `Orderbook` declare `OrderbookTestAccess` as a friend.
//

#ifndef ORDERBOOK_TEST_ACCESS_H
#define ORDERBOOK_TEST_ACCESS_H

#include "../../include/orderbook.h"

struct OrderbookTestAccess
{
    // --- Order registry ---
    static std::size_t OrderCount(const Orderbook& orderbook)
    {
        return orderbook.m_orders.size();
    }

    static bool HasOrder(const Orderbook& orderbook, const Id& id)
    {
        return orderbook.m_orders.contains(id);
    }

    static Quantity RemainingQuantityOf(const Orderbook& orderbook, const Id& id)
    {
        return orderbook.m_orders.at(id).GetRemainingQuantity();
    }

    // --- Bid side (buy orders) ---
    static std::size_t BidPriceLevelCount(const Orderbook& orderbook)
    {
        return orderbook.m_bids.size();
    }

    static bool HasBidPriceLevel(const Orderbook& orderbook, const Price& price)
    {
        return orderbook.m_bids.contains(price);
    }

    static std::size_t OrdersAtBidLevel(const Orderbook& orderbook, const Price& price, const Quantity& quantity)
    {
        const auto price_it = orderbook.m_bids.find(price);
        if (price_it == orderbook.m_bids.end()) return 0;

        const auto quantity_it = price_it->second.find(quantity);
        if (quantity_it == price_it->second.end()) return 0;

        return quantity_it->second.size();
    }

    // --- Ask side (sell orders) ---
    static std::size_t AskPriceLevelCount(const Orderbook& orderbook)
    {
        return orderbook.m_asks.size();
    }

    static bool HasAskPriceLevel(const Orderbook& orderbook, const Price& price)
    {
        return orderbook.m_asks.contains(price);
    }

    static std::size_t OrdersAtAskLevel(const Orderbook& orderbook, const Price& price, const Quantity& quantity)
    {
        const auto price_it = orderbook.m_asks.find(price);
        if (price_it == orderbook.m_asks.end()) return 0;

        const auto quantity_it = price_it->second.find(quantity);
        if (quantity_it == price_it->second.end()) return 0;

        return quantity_it->second.size();
    }
};

#endif //ORDERBOOK_TEST_ACCESS_H

