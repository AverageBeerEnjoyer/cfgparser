
# cfgparser

A header-only C++ library for parsing configuration files with support for unordered, ordered, and list sections, includes, and custom delimiters.

## Features

*   **Header-only:** Easy integration; just include `cfg_parser.hpp`.
*   **Multiple Configuration Files:** Load settings from multiple files, with later files overriding earlier ones.
*   **Section Support:** Organize settings into unordered, ordered, and list sections.
*   **Includes:** Support for including other configuration files using the `!include` directive.
*   **Custom Delimiters:** Specify a custom delimiter for key-value pairs.  The default is ```' = '```.
*   **String Utilities:** Includes a namespace of useful string manipulation functions.
*   **Error Handling:** Provides informative error messages for parsing errors, including filename and line number.
*   **Global Configuration Instance (Optional):**  Provides a global configuration object for simplified access.

## Usage

### Include

```c++
#include "cfg_parser.hpp"
```

### Basic Parsing

```c++
#include <iostream>
#include "cfg_parser.hpp"

int main() {
    try {
        cfgparser::Config config("config.cfg");  // Load from a file
        std::string value = config->get("section", "key");  // Get a value from a section
        std::cout << "Value: " << value << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;  // Handle errors
    }

    return 0;
}
```

c++

### Configuration File Format

The configuration file uses a simple key-value pair format. Sections are defined using delimiters:

-   **Unordered Sections:** `[section_name]` (Key-value pairs are unordered)
-   **Ordered Sections:** `<section_name>` (Key-value pairs are ordered)
-   **List Sections:** `{section_name}` (Contains a list of values, one per line)

Key-value pairs are separated by a delimiter (default ```' = '```). Lines starting with `#` are comments. Includes are specified with `!include filename`.

```cfg
# This is a comment
!include another_config.cfg

[section1]
key1 = value1
key2 = value2

<ordered_section>
keyA = valueA
keyB = valueB

{list_section}
item1
item2
item3
```

### Constructors

`cfgparser::Config` is a `shared_ptr<cfgparser::_Config>`

-   `cfgparser::Config(std::string filename, std::string delimiter = " = ")`: Loads settings from a single file, using the specified delimiter.
-   `cfgparser::Config(std::vector<std::string> filenames, std::string delimiter = " = ")`: Loads settings from multiple files, using the specified delimiter. Later files override earlier ones.
-   `cfgparser::Config(int argc, char** argv, std::string delimiter = " = ")`: Loads settings from files specified as command-line arguments (skipping the program name), using the specified delimiter.
-   `cfgparser::Config()`: Creates an empty shared pointer.

### Accessing Values

-   `bool contains(std::string name)`: Checks if a key exists in the main (unordered) section.
-   `bool contains(std::string unordSec, std::string name)`: Checks if a key exists in a specific unordered section.
-   `std::string get(std::string key)`: Retrieves a value from the main (unordered) section.
-   `std::string get(std::string section, std::string key)`: Retrieves a value from a specific unordered section.
-   `cfgparser::unordered_container& getSection(std::string section)`: Returns the unordered section as a `std::unordered_map<std::string, std::string>`.
-   `cfgparser::ordered_container& getOrderedSection(std::string section)`: Returns the ordered section as a `std::vector<std::pair<std::string, std::string>>`.
-   `std::string getOrdered(std::string section, std::string key)`: Retrieves a value from a specific ordered section.
-   `cfgparser::list_container getList(std::string name)`: Returns the list section as a `std::vector<std::string>`.

### Getting all Sections

-   `cfgparser::unordered_container& getMainSection()`: Returns the main section as a `std::unordered_map<std::string, std::string>`.
-   `std::unordered_map<std::string, cfgparser::ordered_container>& getAllOrdered()`: Returns all ordered sections.
-   `std::unordered_map<std::string, cfgparser::unordered_container>& getAllUnordered()`: Returns all unordered sections.
-   `std::unordered_map<std::string, cfgparser::list_container>& getAllLists()`: Returns all list sections.
-   `std::vector<std::string> getConfigFileNames()`: Returns the list of configuration files parsed.
-   `std::string getConfigFileName()`: Returns the last configuration file parsed.

### Global Configuration Object

The library provides a global configuration object for simplified access.

-   `void initConfig(std::string filename, std::string delimiter = " = ")`: Initializes the global configuration object with a single file and an optional delimiter.
-   `void initConfig(std::vector<std::string> filenames, std::string delimiter = " = ")`: Initializes the global configuration object with multiple files and an optional delimiter.
-   `void initConfig(int argc, char** argv, std::string delimiter = " = ")`: Initializes the global configuration object from command-line arguments and an optional delimiter.
-   `cfgparser::Config getConfig()`: Returns the global configuration object.

```c++
#include <iostream>
#include "cfg_parser.hpp"

int main() {
    try {
        cfgparser::initConfig("config.cfg"); // Initialize the global config
        std::string value = cfgparser::getConfig()->get("section", "key"); // Access values
        std::cout << "Value: " << value << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```


**Important:** The global configuration object must be initialized before being used. If you call `cfgparser::getConfig()` before initialization, it will throw a `std::runtime_error`.

### Custom Delimiters

```c++
cfgparser::Config config("config.cfg", ":"); // Use ":" as the delimiter
std::string value = config->get("section", "key");
```


Your `config.cfg` file would then need to use `:` as the separator:

```cfg
[section]
key:value
```

## String Utilities

The `cfgparser::strutils` namespace provides the following utility functions:

-   `split(const std::string& s, std::string delimiter, bool dropEmptyTokens = false)`: Splits a string into a vector of tokens based on a delimiter.
-   `concat(const std::vector<std::string>& tokens, std::string delimiter = " ")`: Concatenates a vector of strings into a single string with a delimiter.
-   `trimLeft(const std::string& s, char symbol = ' ')`: Trims leading characters from a string.
-   `trimRight(const std::string& s, char symbol = ' ')`: Trims trailing characters from a string.
-   `trim(const std::string& s, char symbol = ' ')`: Trims leading and trailing characters from a string.
-   `startsWith(const std::string& s, char ch)`: Checks if a string starts with a character.
-   `startsWith(const std::string& s, std::string token)`: Checks if a string starts with a token.
-   `endsWith(const std::string& s, char ch)`: Checks if a string ends with a character.
-   `endsWith(const std::string& s, std::string token)`: Checks if a string ends with a token.

## Error Handling

The library throws `std::runtime_error` exceptions for various error conditions, such as:

-   File not found.
-   Incorrect section format.
-   Incorrect line format.
-   Unknown command (e.g., in an `!include` directive).
-   Key not found in a section.
-   Attempting to use the global configuration object before initialization.

## Dependencies

-   C++11 or later

## Installation

Just copy `cfg_parser.hpp` into your project and include it. No external libraries are required.

## Plans
- add conversion of values to int, long long, bool
