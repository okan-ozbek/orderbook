//
// Created by Dorzai on 18/08/2026.
//

#include <catch2/catch_test_macros.hpp>

#include "orderbook.h"
#include "include/orderbook_test_access.h"

using Access = OrderbookTestAccess;

TEST_CASE("Orderbook AddOrder", "[Orderbook][AddOrder]")
{
    SECTION("Adding a single order")
    {
        Orderbook orderbook{};
        const Order order{ 1, 100, 10, Order::BUY };

        const auto trades = orderbook.AddOrder(order);

        REQUIRE(trades.empty());
        REQUIRE(Access::OrderCount(orderbook) == 1);
        REQUIRE(Access::HasOrder(orderbook, 1));
        REQUIRE(Access::HasBidPriceLevel(orderbook, 100));
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 10) == 1);
    }

    SECTION("Adding multiple orders with different prices")
    {
        Orderbook orderbook{};
        const Order order1{ 1, 100, 10, Order::BUY };
        const Order order2{ 2, 101, 5, Order::BUY };
        const Order order3{ 3, 105, 15, Order::SELL };

        orderbook.AddOrder(order1);
        orderbook.AddOrder(order2);
        orderbook.AddOrder(order3);

        // Two distinct bid price levels (100 and 101); ask at 105 rests unmatched
        REQUIRE(Access::BidPriceLevelCount(orderbook) == 2);
        REQUIRE(Access::HasBidPriceLevel(orderbook, 100));
        REQUIRE(Access::HasBidPriceLevel(orderbook, 101));
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 10) == 1);
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 101, 5) == 1);
        REQUIRE(Access::HasAskPriceLevel(orderbook, 105));
    }

    SECTION("Adding multiple orders with the same price but different quantities")
    {
        Orderbook orderbook{};
        const Order order1{ 1, 100, 10, Order::BUY };
        const Order order2{ 2, 100, 5, Order::BUY };
        const Order order3{ 3, 200, 15, Order::SELL };

        orderbook.AddOrder(order1);
        orderbook.AddOrder(order2);
        orderbook.AddOrder(order3);

        REQUIRE(Access::BidPriceLevelCount(orderbook) == 1);
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 10) == 1);
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 5) == 1);
        REQUIRE(Access::HasAskPriceLevel(orderbook, 200));
    }

    SECTION("Adding multiple orders with the same price and same quantity")
    {
        Orderbook orderbook{};
        const Order order1{ 1, 100, 10, Order::BUY };
        const Order order2{ 2, 100, 10, Order::BUY };
        const Order order3{ 3, 200, 10, Order::SELL };

        orderbook.AddOrder(order1);
        orderbook.AddOrder(order2);
        orderbook.AddOrder(order3);

        // Both orders share price + quantity, so they should be queued at the
        // same quantity level (FIFO), giving 2 orders at that level.
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 10) == 2);
    }

    SECTION("Adding the exact same order again")
    {
        Orderbook orderbook{};
        const Order order{ 1, 100, 10, Order::BUY };

        orderbook.AddOrder(order);
        const auto trades = orderbook.AddOrder(order);

        // Duplicate add is a no-op: no trades, still only 1 order tracked,
        // still only 1 order resting at that level (not 2).
        REQUIRE(trades.empty());
        REQUIRE(Access::OrderCount(orderbook) == 1);
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 10) == 1);
    }

    SECTION("Adding a buy order that matches an existing sell order")
    {
        Orderbook orderbook{};
        const Order sell_order{ 1, 90, 10, Order::SELL };
        const Order buy_order{ 2, 100, 10, Order::BUY };

        orderbook.AddOrder(sell_order);
        const auto trades = orderbook.AddOrder(buy_order);
        const auto quantity = std::min(buy_order.GetRemainingQuantity(), sell_order.GetRemainingQuantity());

        REQUIRE(trades.size() == 1);
        REQUIRE(trades[0].quantity == quantity);
        REQUIRE(trades[0].price == sell_order.GetPrice());

        // Both orders fully filled by the trade, so neither should remain
        // in the book nor the order registry.
        REQUIRE(Access::OrderCount(orderbook) == 0);
        REQUIRE(Access::BidPriceLevelCount(orderbook) == 0);
        REQUIRE(Access::AskPriceLevelCount(orderbook) == 0);
    }
}

TEST_CASE("Orderbook CancelOrder", "[Orderbook][CancelOrder]")
{
    SECTION("Cancelling an existing order removes it")
    {
        Orderbook orderbook{};
        const Order order{ 1, 100, 10, Order::BUY };

        orderbook.AddOrder(order);
        orderbook.CancelOrder(1);

        REQUIRE(Access::OrderCount(orderbook) == 0);
        REQUIRE_FALSE(Access::HasOrder(orderbook, 1));
        REQUIRE(Access::BidPriceLevelCount(orderbook) == 0);
    }

    SECTION("Cancelling a non-existent order is a no-op")
    {
        Orderbook orderbook{};
        const Order order{ 1, 100, 10, Order::BUY };

        orderbook.AddOrder(order);
        orderbook.CancelOrder(999); // unknown id

        REQUIRE(Access::OrderCount(orderbook) == 1);
        REQUIRE(Access::HasOrder(orderbook, 1));
    }

    SECTION("Cancelling one of several orders at the same price/quantity level")
    {
        Orderbook orderbook{};
        const Order order1{ 1, 100, 10, Order::BUY };
        const Order order2{ 2, 100, 10, Order::BUY };

        orderbook.AddOrder(order1);
        orderbook.AddOrder(order2);
        orderbook.CancelOrder(1);

        REQUIRE(Access::OrderCount(orderbook) == 1);
        REQUIRE_FALSE(Access::HasOrder(orderbook, 1));
        REQUIRE(Access::HasOrder(orderbook, 2));
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 10) == 1);
    }

    SECTION("Cancelling the last order at a price level removes the level entirely")
    {
        Orderbook orderbook{};
        const Order order{ 1, 100, 10, Order::BUY };

        orderbook.AddOrder(order);
        orderbook.CancelOrder(1);

        REQUIRE_FALSE(Access::HasBidPriceLevel(orderbook, 100));
    }
}

TEST_CASE("Orderbook MatchOrders", "[Orderbook][MatchOrders]")
{
    SECTION("No match when bid is below ask")
    {
        Orderbook orderbook{};
        const Order sell_order{ 1, 100, 10, Order::SELL };
        const Order buy_order{ 2, 90, 10, Order::BUY };

        orderbook.AddOrder(sell_order);
        const auto trades = orderbook.AddOrder(buy_order);

        REQUIRE(trades.empty());
        REQUIRE(Access::HasBidPriceLevel(orderbook, 90));
        REQUIRE(Access::HasAskPriceLevel(orderbook, 100));
    }

    SECTION("Partial fill leaves remainder resting in the book")
    {
        Orderbook orderbook{};
        const Order sell_order{ 1, 100, 5, Order::SELL };
        const Order buy_order{ 2, 100, 10, Order::BUY };

        orderbook.AddOrder(sell_order);
        const auto trades = orderbook.AddOrder(buy_order);

        REQUIRE(trades.size() == 1);
        REQUIRE(trades[0].quantity == 5);

        // Sell order fully filled and gone; buy order has 5 remaining and rests.
        REQUIRE_FALSE(Access::HasOrder(orderbook, 1));
        REQUIRE(Access::HasOrder(orderbook, 2));
        REQUIRE(Access::RemainingQuantityOf(orderbook, 2) == 5);
        REQUIRE(Access::OrdersAtBidLevel(orderbook, 100, 5) == 1);
    }

    SECTION("Incoming order matches across multiple resting orders (FIFO)")
    {
        Orderbook orderbook{};
        const Order sell1{ 1, 100, 5, Order::SELL };
        const Order sell2{ 2, 100, 5, Order::SELL };
        const Order buy{ 3, 100, 10, Order::BUY };

        orderbook.AddOrder(sell1);
        orderbook.AddOrder(sell2);
        const auto trades = orderbook.AddOrder(buy);

        REQUIRE(trades.size() == 2);
        REQUIRE(trades[0].sell_order_id == 1); // FIFO: earliest resting order matched first
        REQUIRE(trades[1].sell_order_id == 2);

        REQUIRE(Access::OrderCount(orderbook) == 0);
    }

    SECTION("Incoming order matches across multiple resting orders with different prices")
    {
        Orderbook orderbook{};
        const Order sell1{ 1, 100, 5, Order::SELL };
        const Order sell2{ 2, 101, 5, Order::SELL };
        const Order buy{ 3, 101, 10, Order::BUY };

        orderbook.AddOrder(sell1);
        orderbook.AddOrder(sell2);
        const auto trades = orderbook.AddOrder(buy);

        REQUIRE(trades.size() == 2);
        REQUIRE(trades[0].sell_order_id == 1); // FIFO: earliest resting order matched first
        REQUIRE(trades[0].price == 100);
        REQUIRE(trades[1].sell_order_id == 2);
        REQUIRE(trades[1].price == 101);

        REQUIRE(Access::OrderCount(orderbook) == 0);
    }
}

TEST_CASE("Orderbook Trade aggressor attribution", "[Orderbook][Trade]")
{
    SECTION("Aggressor is sell order")
    {
        Orderbook orderbook{};
        const Order buy{ 1, 100, 10, Order::BUY };
        const Order sell{ 2, 100, 10, Order::SELL };

        orderbook.AddOrder(buy);
        const auto trades = orderbook.AddOrder(sell);

        REQUIRE(trades.size() == 1);
        REQUIRE(trades[0].buy_order_id == 1);
        REQUIRE(trades[0].sell_order_id == 2);
        REQUIRE(trades[0].aggressor_side == Order::Side::SELL);
    }

    SECTION("Aggressor is buy order")
    {
        Orderbook orderbook{};
        const Order sell{ 1, 100, 10, Order::SELL };
        const Order buy{ 2, 100, 10, Order::BUY };

        orderbook.AddOrder(sell);
        const auto trades = orderbook.AddOrder(buy);

        REQUIRE(trades.size() == 1);
        REQUIRE(trades[0].buy_order_id == 2);
        REQUIRE(trades[0].sell_order_id == 1);
        REQUIRE(trades[0].aggressor_side == Order::Side::BUY);
    }
}


