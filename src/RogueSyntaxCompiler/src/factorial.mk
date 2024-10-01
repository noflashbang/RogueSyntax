let factorial = fn(x) {
    if (x == 0) {
        return 1;
    } else {
        return x * factorial(x - 1);
    }
};
let number = 10;
let result = factorial(number);
let printResult = fn(result) {
    let output = "The factorial of " + number + " is " + result;
    return output;
};
let message = printResult(result);
printLine(message);