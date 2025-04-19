#include "benchmark/benchmark.h"
#include "tsl/robin_map.h"
#include "ankerl/unordered_dense.h"
#include "fmt/core.h"


static uint32_t TIMES = 10000;
tsl::robin_map<std::string, std::string> a;
ankerl::unordered_dense::map<std::string, std::string> b;

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

BENCHMARK_MAIN();