let MakeHeap = fn(arr, length, i)
{
    let largest = i; // Initialize largest as root
    let left = 2 * i + 1; // Left child
    let right = 2 * i + 2; // Right child

    // If left child is larger than root
    if (left < length && arr[left] > arr[largest])
    {
        largest = left;
    }

    // If right child is larger than the largest so far
    if (right < length && arr[right] > arr[largest])
    {
        largest = right;
    }

    // If the largest is not the root
    if (largest != i)
    {
        let temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        // Recursively heapify the affected subtree
        MakeHeap(arr, length, largest);
    }
};

let SortHeap = fn(arr)
{
    let length = len(arr);

    // Build heap (rearrange array)
    let i = length / 2 - 1;
    while (i >= 0)
    {
        MakeHeap(arr, length, i);
        i = i - 1;
    }

    // Extract elements one by one from the heap
    let j = length - 1;
    while (j > 0)
    {
        // Move current root to end
        let temp = arr[0];
        arr[0] = arr[j];
        arr[j] = temp;

        // Call MakeHeap on the reduced heap
        MakeHeap(arr, j, 0);

        j = j - 1;
    }
    return arr;
};

// Helper function to print array
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

// Main execution
let numbers = [64, 34, 25, 12, 22, 11, 90];
let sortedNumbers = SortHeap(numbers);
let message = printArray(sortedNumbers);
printLine(message);
