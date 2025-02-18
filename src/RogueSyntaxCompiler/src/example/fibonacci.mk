let fibonacci = fn(n) {
    let fib = [0, 1];
    let i = 2;
    while (i < n) {
        let next = fib[i - 1] + fib[i - 2];
        fib = push(fib, next);
        i = i + 1;
    }
    return fib;
};
let printFibonacci = fn(fib) {
    let output = "Fibonacci sequence: ";
    let length = len(fib);
    let j = 0;
    while (j < length) {
        output = output + fib[j];
        if (j < length - 1) {
            output = output + ", ";
        }
        j = j + 1;
    }
    return output;
};
let number = 30;
let fibNumbers = fibonacci(number);
let message = printFibonacci(fibNumbers);
printLine(message);
