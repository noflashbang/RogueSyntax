let bubbleSort = fn(arr)
{
    let length = len(arr);
    let i = 0;    
    while (i < length)
    {
        let j = 0;
        while (j < length - i - 1)
        {
            if (arr[j] > arr[j + 1])
            {
                let temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    return arr;
};
let numbers = [64, 34, 25, 12, 22, 11, 90];
let sortedNumbers = bubbleSort(numbers);
let printArray = fn(arr)
{
    let output = "Sorted array: ";
    let length = len(arr);
    let j = 0;
    while (j < length)
    {
        output = output + arr[j];
        if (j < length - 1)
        {
            output = output + ", ";
        }
        j = j + 1;
    }
    return output;
};
let message = printArray(sortedNumbers);
printLine(message);