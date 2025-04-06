#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace std;  // Style issue: using namespace std

// Function to demonstrate memory leaks
void memoryLeakExample() {
    int* array = new int[100];  // Memory leak: no delete[] 
    
    // Do something with array
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    // Missing: delete[] array;
}

// Function with uninitialized variables
void uninitializedVariables() {
    int x;  // Uninitialized
    int result;
    
    if (x > 0) {  // Using x without initialization
        result = x * 10;
    }
    
    cout << "Result: " << result << endl;  // Using result without initialization
}

// Function with buffer overflow risk
void bufferOverflow(const char* input) {
    char buffer[10];
    strcpy(buffer, input);  // Unsafe: no bounds checking
    cout << "Buffer: " << buffer << endl;
}

// Function with resource leak
FILE* openFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        cout << "Error opening file" << endl;
        return nullptr;
    }
    
    // Note: no fclose(file) if error occurs after this point
    return file;
}

// Function with dead code
int deadCodeExample(int value) {
    if (value < 0) {
        return -1;
    }
    
    return value * 2;
    
    // Dead code - never executed
    value = 0;
    return value;
}

// Performance issue: inefficient loop
void inefficientLoop(const vector<int>& data) {
    for (int i = 0; i < data.size(); i++) {  // Inefficient: data.size() called in each iteration
        cout << data[i] << endl;
    }
}

// Performance issue: passing large object by value
void processVector(vector<string> data) {  // Should be const reference
    for (const auto& item : data) {
        cout << item << endl;
    }
}

// Class with potential null pointer issues
class ResourceManager {
private:
    int* resource;
    
public:
    ResourceManager() {
        resource = nullptr;  // Initialize to null
    }
    
    void initialize() {
        resource = new int(42);
    }
    
    int getValue() {
        return *resource;  // Potential null dereference if initialize() not called
    }
    
    ~ResourceManager() {
        delete resource;  // Correct cleanup, but could be null
    }
};

int main() {
    memoryLeakExample();
    uninitializedVariables();
    
    char* input = "This is a very long string that will cause a buffer overflow";
    bufferOverflow(input);
    
    FILE* file = openFile("nonexistent.txt");
    // Missing: if (file) fclose(file);
    
    int result = deadCodeExample(10);
    
    vector<int> numbers = {1, 2, 3, 4, 5};
    inefficientLoop(numbers);
    
    vector<string> words = {"Hello", "World", "C++", "Programming"};
    processVector(words);
    
    ResourceManager manager;
    // Missing: manager.initialize();
    cout << "Value: " << manager.getValue() << endl;  // Potential crash
    
    return 0;
}