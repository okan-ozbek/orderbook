//
// Created by Dorza on 18/08/2026.
//

#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#include "order.h"

struct Trade
{
    Id buy_order_id;
    Id sell_order_id;
    Order::Side aggressor_side;
    Price price;
    Quantity quantity;
};

using Orders = std::vector<Order>;
using Trades = std::vector<Trade>;

class Orderbook
{
public:
    using Quantities = std::map<Quantity, Orders, std::greater<>>;
    using BidPrices = std::map<Price, Quantities, std::greater<>>;
    using AskPrices = std::map<Price, Quantities, std::less<>>;
    using OrderHashmap = std::unordered_map<Id, Order>;

#ifdef ORDERBOOK_ENABLE_TEST_HOOKS
    // Only compiled in when the `tests` target defines ORDERBOOK_ENABLE_TEST_HOOKS.
    // Grants the test accessor read access to private state without adding any
    // public API surface to production builds.
    friend struct OrderbookTestAccess;
#endif

    Trades AddOrder(const Order& order)
    {
        if (m_orders.contains(order.GetId()))
        {
            return {};
        }

        if (order.IsBuy()) {
            m_bids[order.GetPrice()][order.GetRemainingQuantity()].push_back(order);
        }

        if (order.IsSell()) {
            m_asks[order.GetPrice()][order.GetRemainingQuantity()].push_back(order);
        }

        m_orders.emplace(order.GetId(), order);

        return MatchOrders();
    }

    void CancelOrder(const Id& order_id)
    {
        if (!m_orders.contains(order_id)) {
            return;
        }

        const auto& order = m_orders.at(order_id);
        auto RemoveOrder = [&order](auto& prices) {
            auto& quantities = prices[order.GetPrice()];
            auto& orders = quantities[order.GetRemainingQuantity()];

            orders.erase(
                std::remove_if(
                    orders.begin(), orders.end(),
                    [&order](const Order& current) {
                        return current.GetId() == order.GetId();
                    }
                ),
                orders.end()
            );

            if (orders.empty()) {
                quantities.erase(order.GetRemainingQuantity());
            }

            if (quantities.empty()) {
                prices.erase(order.GetPrice());
            }
        };

        if (order.IsBuy()) {
            RemoveOrder(m_bids);
        }

        if (order.IsSell()) {
            RemoveOrder(m_asks);
        }

        m_orders.erase(order_id);
    }

private:
    BidPrices m_bids;
    AskPrices m_asks;
    OrderHashmap m_orders;

    Trades MatchOrders()
    {
        Trades trades{};

        while (!m_bids.empty() && !m_asks.empty()) {
            auto& [bid_price, bid_quantities] = *m_bids.begin();
            auto& [ask_price, ask_quantities] = *m_asks.begin();

            if (bid_price < ask_price) {
                break;
            }

            auto& [bid_quantity, bid_orders] = *bid_quantities.begin();
            auto& [ask_quantity, ask_orders] = *ask_quantities.begin();

            while (!bid_orders.empty() && !ask_orders.empty()) {
                auto& buy_order = bid_orders.front();
                auto& sell_order = ask_orders.front();

                trades.push_back(GetTrade(buy_order, sell_order));

                const auto trade_quantity = trades.back().quantity;

                auto UpdateOrders = [this](Order& order, const Quantity& fill_amount, Orders& orders, Quantities& quantities) {
                    order.Fill(fill_amount);

                    const auto updated_order = order;
                    orders.erase(orders.begin());

                    if (updated_order.IsFilled()) {
                        m_orders.erase(updated_order.GetId());
                        return;
                    }

                    quantities[updated_order.GetRemainingQuantity()].push_back(updated_order);
                    m_orders.at(updated_order.GetId()) = updated_order;
                };

                UpdateOrders(buy_order, trade_quantity, bid_orders, bid_quantities);
                UpdateOrders(sell_order, trade_quantity, ask_orders, ask_quantities);
            }

            auto UpdateMaps = [](auto& orders, auto& quantities, auto& prices, const auto& price) {
                if (orders.empty()) {
                    quantities.erase(quantities.begin());
                }

                if (quantities.empty()) {
                    prices.erase(price);
                }
            };

            UpdateMaps(bid_orders, bid_quantities, m_bids, bid_price);
            UpdateMaps(ask_orders, ask_quantities, m_asks, ask_price);
        }

        return trades;
    }

    Trade GetTrade(const Order& buy_order, const Order& sell_order)
    {
        const auto trade_quantity = std::min(
            buy_order.GetRemainingQuantity(),
            sell_order.GetRemainingQuantity()
        );
        const auto is_bid_aggressor{ buy_order.GetRecencyId() > sell_order.GetRecencyId() };
        const auto aggressor_side = (is_bid_aggressor) ? Order::Side::BUY : Order::Side::SELL;

        return Trade{
            .buy_order_id   = buy_order.GetId(),
            .sell_order_id  = sell_order.GetId(),
            .aggressor_side = aggressor_side,
            .price          = sell_order.GetPrice(),
            .quantity       = trade_quantity
        };
    }
};

#endif //ORDERBOOK_ORDERBOOK_H
