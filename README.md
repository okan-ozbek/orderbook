```
    ███████    ███████████   ██████████   ██████████ ███████████   ███████████     ███████       ███████    █████   ████
  ███▒▒▒▒▒███ ▒▒███▒▒▒▒▒███ ▒▒███▒▒▒▒███ ▒▒███▒▒▒▒▒█▒▒███▒▒▒▒▒███ ▒▒███▒▒▒▒▒███  ███▒▒▒▒▒███   ███▒▒▒▒▒███ ▒▒███   ███▒ 
 ███     ▒▒███ ▒███    ▒███  ▒███   ▒▒███ ▒███  █ ▒  ▒███    ▒███  ▒███    ▒███ ███     ▒▒███ ███     ▒▒███ ▒███  ███   
▒███      ▒███ ▒██████████   ▒███    ▒███ ▒██████    ▒██████████   ▒██████████ ▒███      ▒███▒███      ▒███ ▒███████    
▒███      ▒███ ▒███▒▒▒▒▒███  ▒███    ▒███ ▒███▒▒█    ▒███▒▒▒▒▒███  ▒███▒▒▒▒▒███▒███      ▒███▒███      ▒███ ▒███▒▒███   
▒▒███     ███  ▒███    ▒███  ▒███    ███  ▒███ ▒   █ ▒███    ▒███  ▒███    ▒███▒▒███     ███ ▒▒███     ███  ▒███ ▒▒███  
 ▒▒▒███████▒   █████   █████ ██████████   ██████████ █████   █████ ███████████  ▒▒▒███████▒   ▒▒▒███████▒   █████ ▒▒████
   ▒▒▒▒▒▒▒    ▒▒▒▒▒   ▒▒▒▒▒ ▒▒▒▒▒▒▒▒▒▒   ▒▒▒▒▒▒▒▒▒▒ ▒▒▒▒▒   ▒▒▒▒▒ ▒▒▒▒▒▒▒▒▒▒▒     ▒▒▒▒▒▒▒       ▒▒▒▒▒▒▒    ▒▒▒▒▒   ▒▒▒▒                                                                                                                      
```
<div style="width:100%; text-align:right;">
<span style="font-size: 9px; text-align: right;">ASCII art generated with <a href="https://patorjk.com/software/taag/#p=display&f=Rebel&t=ORDERBOOK&x=none&v=4&h=4&w=80&we=false">patorjk.com</a></span>
</div>

A simple, fast C++ **limit order book** that matches **buy** and **sell** orders by best price, then by time (first-in, first-served).

## How it works

- **Place an order** `AddOrder(order)` adds a `BUY` or `SELL` order at a given price and quantity.
- **Matching** the book automatically matches the highest bid against the lowest ask whenever `bid_price >= ask_price`.
- **Partial fills** if quantities don't match exactly, the smaller order fully fills and the larger one keeps its remaining quantity in the book.
- **Cancel an order** `CancelOrder(id)` removes a resting order before it gets matched.
- **Trades** every match produces a `Trade` record: who bought, who sold, at what price, how much, and who was the "aggressor" (the order that arrived last).

```
   BUY orders (bids)          SELL orders (asks)
   highest price first        lowest price first
   ┌─────────────┐            ┌──────────────┐
   │ 101 → Buy A │  ◄──────►  │ 100 → Sell X │  matches (101 ≥ 100)
   │ 99  → Buy B │            │ 102 → Sell Y │
   └─────────────┘            └──────────────┘
```

## Running tests

Tests live in `/tests` and run via the helper scripts in `/scripts` (build with CMake + Catch2 under the hood):

| Command                          | What it does              |
|-----------------------------------|----------------------------|
| `.\scripts\app-build.cmd`         | Builds the project         |
| `.\scripts\app-test.cmd`          | Runs the test suite        |
| `.\scripts\app-test-verbose.cmd`  | Runs tests with detailed output |

**NOTE**: If your shell can resolve scripts directly, `app-test` also works — no `.\scripts\` prefix needed.
