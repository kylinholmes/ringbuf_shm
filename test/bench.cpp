#include "benchmark/benchmark.h"
#include "tsl/robin_map.h"
#include "ankerl/unordered_dense.h"
#include "fmt/core.h"
#include <map>
#include <unordered_map>


static uint32_t TIMES = 10000;
std::map<std::string, std::string> std_map;
std::unordered_map<std::string, std::string> std_unordered_map;
tsl::robin_map<std::string, std::string> a;
ankerl::unordered_dense::map<std::string, std::string> b;

static void test_std_map_insert(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        std_map.clear();
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            std_map[buffer] = buffer;
            std_map[buffer];
            benchmark::DoNotOptimize(std_map);
        }
    }
}
BENCHMARK(test_std_map_insert)->Arg(TIMES);

static void test_std_unordered_map_insert(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        std_unordered_map.clear();
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            std_unordered_map[buffer] = buffer;
            std_unordered_map[buffer];
            benchmark::DoNotOptimize(std_unordered_map);
        }
    }
}
BENCHMARK(test_std_unordered_map_insert)->Arg(TIMES);

static void test_robin_insert(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        a.clear();
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            a[buffer] = buffer;
            a[buffer];
            benchmark::DoNotOptimize(a);
        }
    }
}
BENCHMARK(test_robin_insert)->Arg(TIMES);

static void test_ankerl_insert(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        b.clear();
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            b[buffer] = buffer;
            b[buffer];
            benchmark::DoNotOptimize(b);
        }
    }
}
BENCHMARK(test_ankerl_insert)->Arg(TIMES);

static void test_robin_find(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            a[buffer];
            benchmark::DoNotOptimize(a);
        }
    }
}
BENCHMARK(test_robin_find)->Arg(TIMES);

static void test_ankerl_find(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            b[buffer];
            benchmark::DoNotOptimize(b);
        }
    }
}
BENCHMARK(test_ankerl_find)->Arg(TIMES);

static void test_std_map_find(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            std_map[buffer];
            benchmark::DoNotOptimize(std_map);
        }
    }
}
BENCHMARK(test_std_map_find)->Arg(TIMES);

static void test_std_unordered_map_find(benchmark::State &state)
{
    auto times = state.range(0);
    for (auto _ : state)
    {
        char buffer[16] = {0};
        for (uint32_t i = 0; i < times; i++)
        {
            fmt::format_to(buffer, "{:09d}", i);
            std_unordered_map[buffer];
            benchmark::DoNotOptimize(std_unordered_map);
        }
    }
}
BENCHMARK(test_std_unordered_map_find)->Arg(TIMES);

BENCHMARK_MAIN();