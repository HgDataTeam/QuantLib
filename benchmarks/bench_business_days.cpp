// Benchmark for businessDaysBetween optimization
#include <ql/time/date.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <chrono>
#include <iostream>
#include <set>
#include <algorithm>

using namespace QuantLib;
using namespace std;
using namespace std::chrono;

// Baseline: Linear iteration through all dates
BigInteger businessDaysBetween_baseline(const Calendar& cal, Date from, Date to, bool includeFirst) {
    BigInteger res = 0;
    for (Date d = includeFirst ? from : from + 1; d < to; ++d) {
        res += static_cast<Date::serial_type>(cal.isBusinessDay(d));
    }
    return res;
}

// Optimized: Calculate by subtracting weekends and holidays
BigInteger businessDaysBetween_optimized(const Calendar& cal, Date from, Date to, bool includeFirst) {
    Date d1 = includeFirst ? from : from + 1;
    Date d2 = to;

    if (d1 >= d2) return 0;

    BigInteger totalDays = d2 - d1;

    // Count weekends
    BigInteger weeks = totalDays / 7;
    BigInteger remainingDays = totalDays % 7;
    BigInteger weekends = weeks * 2;

    // Count remaining weekend days
    Weekday startDay = d1.weekday();
    for (Integer i = 0; i < remainingDays; ++i) {
        Weekday day = Weekday((startDay + i) % 7);
        if (day == Saturday || day == Sunday) {
            weekends++;
        }
    }

    // Count holidays (this is simplified - in reality we'd use the calendar's holiday list)
    BigInteger holidays = 0;
    for (Date d = d1; d < d2; ++d) {
        if (cal.isHoliday(d) && d.weekday() != Saturday && d.weekday() != Sunday) {
            holidays++;
        }
    }

    return totalDays - weekends - holidays;
}

int main() {
    UnitedStates calendar(UnitedStates::NYSE);

    cout << "=== businessDaysBetween Benchmark ===" << endl << endl;

    // Test different date ranges
    vector<pair<string, pair<Date, Date>>> testCases = {
        {"1 month",  {Date(1, January, 2024), Date(1, February, 2024)}},
        {"3 months", {Date(1, January, 2024), Date(1, April, 2024)}},
        {"1 year",   {Date(1, January, 2024), Date(1, January, 2025)}},
        {"5 years",  {Date(1, January, 2020), Date(1, January, 2025)}}
    };

    for (const auto& testCase : testCases) {
        Date from = testCase.second.first;
        Date to = testCase.second.second;

        cout << "Range: " << testCase.first << " (" << from << " to " << to << ")" << endl;

        // Baseline
        auto start = high_resolution_clock::now();
        BigInteger result1 = businessDaysBetween_baseline(calendar, from, to, true);
        auto end = high_resolution_clock::now();
        auto baseline_time = duration_cast<microseconds>(end - start).count();

        // Optimized
        start = high_resolution_clock::now();
        BigInteger result2 = businessDaysBetween_optimized(calendar, from, to, true);
        end = high_resolution_clock::now();
        auto optimized_time = duration_cast<microseconds>(end - start).count();

        cout << "  Baseline:  " << baseline_time << " μs  (result: " << result1 << " days)" << endl;
        cout << "  Optimized: " << optimized_time << " μs  (result: " << result2 << " days)" << endl;
        cout << "  Speedup:   " << (double)baseline_time / (optimized_time + 1) << "x" << endl;
        cout << "  Match:     " << (result1 == result2 ? "YES" : "NO") << endl << endl;
    }

    return 0;
}
