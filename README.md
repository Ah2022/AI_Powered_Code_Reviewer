# AI-Powered C++ Code Reviewer

A powerful AI-driven tool designed to analyze C++ source code, detect common errors, recommend performance improvements, and offer detailed explanations using OpenAI's GPT models.

## Features

- Detect common coding errors and anti-patterns in C++ code
- Identify memory management issues, uninitialized variables, buffer overflows
- Detect style violations and suggest modern C++ practices
- Provide performance optimization recommendations
- Generate detailed explanations and fix suggestions using OpenAI's GPT-4o model

## Requirements

- Python 3.6 or higher
- OpenAI API key (set as environment variable `OPENAI_API_KEY`)

##  Installation

### 🖥️ Option 1:  Running Locally with Python Virtual Environment


# Clone the repository
```bash
git clone https://github.com/yourusername/AI_Powered_Code_Reviewer.git
cd AI_Powered_Code_Reviewer
```
# Create virtual environment (optional but recommended)
```bash
python -m venv venv
source venv/bin/activate     # On Windows: venv\Scripts\activate
```
# Install required packages
```bash
pip install -r openai
tqdm
rich
python-dotenv

```
## 🌐 Option 2: Run on Replit (Web IDE)
1- Go to https://replit.com

2- Create a new Python Repl

3- Upload your project files or link GitHub repo

4- Set your environment variable:

 - Go to the 🔐 Secrets tab

 - Add OPENAI_API_KEY and paste your key

5-Run the script using the shell:

```bash
python cpp_code_reviewer.py -f test.cpp -v
```

## FlowChart

<img src="AI-Powered Code Reviewer – System Architecture.png"/>

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


1. Pattern-based analysis: First pass uses regex patterns to identify common C++ issues.
2. AI-powered analysis: OpenAI's GPT model analyzes the full code to find additional issues.
3. Sends code to GPT-4o for intelligent review and recommendations.
4. Issue enhancement: Each detected issue is explained in detail with recommendations for fixes.
5. Results formatting: Issues are organized by severity and presented with code snippets.

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
