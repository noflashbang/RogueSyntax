let isPrime = fn(n)
{
    if (n < 2) 
    {
        return false;
    }
    let i = 2;
    while (i * i <= n)
    {
        if (n % i == 0)
        {
            return false;
        }
        i = i + 1;
    }
    return true;
};
let findPrimes = fn(limit)
{
    let primes = [];
    let num = 2;    
    while (num <= limit)
    {
        if (isPrime(num))
        {
            primes = push(primes,num);
        }
        num = num + 1;
    }    
    return primes;
};
let limit = 50;
let primeNumbers = findPrimes(limit);
let printPrimes = fn(primes)
{
    let output = "Prime numbers: ";
    let length = len(primes);
    let j = 0; 
    while (j < length)
    {
        output = output + primes[j];
        if (j < length - 1)
        {
            output = output + ", ";
        }
        j = j + 1;
    }
    return output;
};
let message = printPrimes(primeNumbers);
printLine(message);