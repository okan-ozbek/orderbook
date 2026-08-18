//
// Created by Dorzai on 18/08/2026.
//

#include <format>
#include <iostream>

int main()
{
    std::cout << "=== Orderbook Console README ===\n";
    std::cout << "Run these from the repository root:\n\n";
    std::cout << "1) scripts\\app-test.cmd          -> Runs tests\n";
    std::cout << "2) scripts\\app-test-verbose.cmd  -> Runs tests with verbose output\n";
    std::cout << "3) scripts\\app-build.cmd         -> Builds the app before testing\n\n";
    std::cout << "Example:\n";
    std::cout << "  .\\scripts\\app-test.cmd\n";
    std::cout << "\nAlso possible:\n";
    std::cout << "  app-test\n";
    std::cout << "  app-test-verbose\n";
    std::cout << "  app-build\n";
    std::cout << "  (works if your shell can resolve commands from the scripts folder)\n";

    return 0;
}
