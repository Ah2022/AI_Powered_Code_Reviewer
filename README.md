# AI-Powered C++ Code Reviewer

A tool that analyzes C++ code, detects common errors, suggests optimizations, and provides explanations using OpenAI's GPT models.

## Features

- Detect common coding errors and anti-patterns in C++ code
- Identify memory management issues, uninitialized variables, buffer overflows
- Detect style violations and suggest modern C++ practices
- Provide performance optimization recommendations
- Generate detailed explanations and fix suggestions using OpenAI's GPT-4o model

## Requirements

- Python 3.6 or higher
- OpenAI API key (set as environment variable `OPENAI_API_KEY`)

## Installation

No special installation is needed beyond having Python and the OpenAI package:

```bash
pip install openai
```

## Usage

```bash
python cpp_code_reviewer.py -f <cpp_file> [-o <output_file>] [-v]
```

### Command-line options:

- `-f, --file`: C++ source file to review (required)
- `-o, --output`: Output file for review results (optional, defaults to console)
- `-v, --verbose`: Enable verbose output (optional)

## Example

```bash
python cpp_code_reviewer.py -f sample.cpp -v
```

This will analyze `sample.cpp` and output the review results to the console, with verbose progress updates.

## How It Works

1. Pattern-based analysis: First pass uses regex patterns to identify common C++ issues
2. AI-powered analysis: OpenAI's GPT model analyzes the full code to find additional issues
3. Issue enhancement: Each detected issue is explained in detail with recommendations for fixes
4. Results formatting: Issues are organized by severity and presented with code snippets

## Output Format

For each issue detected, the tool provides:

- Issue type (Memory Leak, Buffer Overflow, etc.)
- Severity level (ERROR, WARNING, INFO, OPTIMIZATION)
- Line number where the issue was found
- Code snippet showing the context
- Detailed explanation of why it's a problem
- Recommended fix with code examples

## License

This tool is provided for educational and development purposes.