//
// Created by Dorzai on 18/08/2026.
//

#include <catch2/catch_test_macros.hpp>

#include "orderbook.h"

TEST_CASE("Construct Order", "[Order][Construction]")
{
    Order buy_order{ 1, 100, 10, Order::BUY };
    Order sell_order{ 2, 100, 10, Order::SELL };

    SECTION("Buy Order")
    {
        REQUIRE(buy_order.GetId() == 1);
        REQUIRE(buy_order.GetPrice() == 100);
        REQUIRE(buy_order.GetInitialQuantity() == 10);
        REQUIRE(buy_order.GetRemainingQuantity() == 10);

        REQUIRE(buy_order.GetSide() == Order::BUY);
        REQUIRE(buy_order.IsBuy());
        REQUIRE_FALSE(buy_order.IsSell());
    }

    SECTION("Sell Order")
    {
        REQUIRE(sell_order.GetId() == 2);
        REQUIRE(sell_order.GetPrice() == 100);
        REQUIRE(sell_order.GetInitialQuantity() == 10);
        REQUIRE(sell_order.GetRemainingQuantity() == 10);

        REQUIRE(sell_order.GetSide() == Order::SELL);
        REQUIRE(sell_order.IsSell());
        REQUIRE_FALSE(sell_order.IsBuy());
    }
}

TEST_CASE("Order recency in order of creation", "[Order][Recency]")
{
    const Order order1{ 1, 100, 10, Order::BUY };
    const Order order2{ 2, 100, 10, Order::BUY };
    const Order order3{ 3, 100, 10, Order::BUY };

    REQUIRE(order1.GetRecencyId() < order2.GetRecencyId());
    REQUIRE(order2.GetRecencyId() < order3.GetRecencyId());
    REQUIRE(order3.GetRecencyId() > order1.GetRecencyId());
}

TEST_CASE("Fill Order", "[Order][Fill]")
{
    Order order(1, 100, 10, Order::BUY);

    SECTION("Fill less than remaining quantity")
    {
        order.Fill(5);
        REQUIRE(order.GetRemainingQuantity() == 5);
        REQUIRE_FALSE(order.IsFilled());
    }

    SECTION("Fill exactly remaining quantity")
    {
        order.Fill(10);
        REQUIRE(order.GetRemainingQuantity() == 0);
        REQUIRE(order.IsFilled());
    }

    SECTION("Fill more than remaining quantity")
    {
        REQUIRE_THROWS_AS(order.Fill(15), std::logic_error);
    }
}