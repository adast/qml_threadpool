#pragma once

#include <string>
#include <future>
#include <gmpxx.h>

namespace TP
{

    inline std::string make_string(double d) { return std::to_string(d); }
    inline std::string make_string(float f) { return std::to_string(f); }
    inline std::string make_string(int i) { return std::to_string(i); }
    inline std::string make_string(const std::string &str) { return str; }
    inline std::string make_string(const mpz_class &mpz) { 
        // Return string with all digits for relatively small numbers
        if (mpz_sizeinbase(mpz.get_mpz_t(), 10) < 64)
        {
            return mpz.get_str(); 
        }

        // For large numbers - return string with number in scientific format
        mpf_class mpf(std::move(mpz));
        std::string result(32, '0');
        int length = gmp_snprintf(&result[0], 32, "%.12FE", mpf);
        result.erase(result.begin() + length, result.end());
        return result; 
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, void>::value, std::string>::type
    make_string(std::shared_future<T> &)
    {
        return "";
    }

    template <typename T>
    typename std::enable_if<!std::is_same<T, void>::value, std::string>::type
    make_string(std::shared_future<T> &future)
    {
        if (future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            return make_string(future.get());
        return "";
    }

}