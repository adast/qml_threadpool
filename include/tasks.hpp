#pragma once

#include <gmpxx.h>

namespace tasks
{
    
    inline mpz_class fib(int n)
    {
        if (n <= 1)
            return n;
        mpz_class prev2(0);
        mpz_class prev1(1);
        mpz_class cur;
        for (int i = 2; i <= n; i++)
        {
            cur = prev2 + prev1;
            prev2 = prev1;
            prev1 = cur;
        }
        return cur;
    }

    inline mpz_class factorial(int n)
    {
        mpz_class res(1);
        for (int i = 1; i <= n; i++)
        {
            res *= i;
        }
        return res;
    }

    inline mpz_class double_factorial(int n)
    {
        mpz_class res(1);
        for (int i = n; i >= 1; i -= 2)
        {
            res *= i;
        }
        return res;
    }

}