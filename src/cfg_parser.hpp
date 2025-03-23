#ifndef CFG_PARSER_HPP
#define CFG_PARSER_HPP
#include <string.h>  //strerror()

#include <algorithm>
#include <exception>
#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace cfgparser {

class Value {
   public:
    std::string value;

    Value(std::string value) : value(value) {}
    Value() {}

    int asInt() const {
        try {
            return std::stoi(value);
        } catch (std::exception& ex) {
            throw std::runtime_error("Can not cast to int: '" + value + "'");
        }
    }
    double asDouble() const {
        try {
            return std::stod(value);
        } catch (std::exception& ex) {
            throw std::runtime_error("Can not cast to double: '" + value + "'");
        }
    }
    long long asLongLong() const {
        try {
            return std::stoll(value);
        } catch (std::exception& ex) {
            throw std::runtime_error("Can not cast to long long: '" + value + "'");
        }
    }
    bool asBool() const {
        if (value == "true") return true;
        if (value == "false") return false;
        throw std::runtime_error("Can not cast to bool: '" + value + "'");
    }
    operator std::string() const { return value; }

    bool operator==(const Value& v) const { return value == v.value; }

    friend std::ostream& operator<<(std::ostream &out, const Value &v) {
        out << v.value;
        return out;
    }
};

const std::string defaultDelimiter = " = ";

typedef std::unordered_map<std::string, Value> unordered_container;
typedef std::vector<std::pair<std::string, Value>> ordered_container;
typedef std::vector<Value> list_container;

namespace strutils {

inline std::vector<std::string> split(
    const std::string& s, std::string delimiter, bool dropEmptyTokens = false
) {
    std::vector<std::string> res;
    if (s.empty()) return res;
    std::string buf = "";
    std::string delimbuf = "";
    int j = 0;
    for (int i = 0; i < s.length(); ++i) {
        if (s[i] != delimiter[j]) {
            buf.append(delimbuf);
            delimbuf.clear();
            buf.push_back(s[i]);
        } else {
            ++j;
            delimbuf.push_back(s[i]);
            if (j == delimiter.length()) {
                if (!dropEmptyTokens || !buf.empty()) res.push_back(buf);
                buf.clear();
                j = 0;
                delimbuf.clear();
            }
        }
    }
    if (!buf.empty() || (!dropEmptyTokens && s.substr(s.length() - delimiter.length()) == delimiter))
        res.push_back(buf);
    return res;
}

inline std::string concat(const std::vector<std::string>& tokens, std::string delimiter = " ") {
    std::stringstream ss;
    for (int i = 0; i < tokens.size(); ++i) {
        if (i > 0) ss << delimiter;
        ss << tokens[i];
    }
    return ss.str();
}

inline std::string trimLeft(const std::string& s, char symbol = ' ') {
    int l = s.length();
    for (int i = 0; i < s.length(); ++i) {
        if (s[i] != symbol) {
            l = i;
            break;
        }
    }
    return s.substr(l, s.length() - l);
}

inline std::string trimRight(const std::string& s, char symbol = ' ') {
    int r = 0;
    for (int i = s.length(); i > 0; --i) {
        if (s[i - 1] != symbol) {
            r = i;
            break;
        }
    }
    return s.substr(0, r);
}
inline std::string trim(const std::string& s, char symbol = ' ') {
    std::string res = trimLeft(s, symbol);
    res = trimRight(res, symbol);
    return res;
}

inline bool startsWith(const std::string& s, char ch) { return s.length() > 0 && s[0] == ch; }
inline bool startsWith(const std::string& s, std::string token) {
    if (s.length() < token.length()) return false;
    for (int i = 0; i < token.length(); ++i) {
        if (s[i] != token[i]) return false;
    }
    return true;
}
inline bool endsWith(const std::string& s, char ch) { return s.length() > 0 && s[s.length() - 1] == ch; }
inline bool endsWith(const std::string& s, std::string token) {
    if (s.length() < token.length()) return false;
    for (int i = 1; i <= token.length(); ++i) {
        if (token[token.length() - i] != s[s.length() - i]) return false;
    }
    return true;
}

inline std::string to_string(const unordered_container& container, std::string delimiter = defaultDelimiter) {
    std::stringstream ss;
    for (auto it = container.begin(); it != container.end(); ++it) {
        ss << it->first << delimiter << it->second.value << std::endl;
    }
    return ss.str();
}
inline std::string to_string(const ordered_container& container, std::string delimiter = defaultDelimiter) {
    std::stringstream ss;
    for (auto it = container.begin(); it != container.end(); ++it) {
        ss << it->first << delimiter << it->second.value << std::endl;
    }
    return ss.str();
}
inline std::string to_string(const list_container& container, std::string delimiter = defaultDelimiter) {
    std::stringstream ss;
    for (auto it = container.begin(); it != container.end(); ++it) {
        ss << it->value << std::endl;
    }
    return ss.str();
}

}  // namespace strutils

class _Config {
   private:
    enum SectionType { UNORDERED, ORDERED, LIST };
    std::vector<std::string> configFileNames;
    std::unordered_map<std::string, unordered_container> unorderedSections;
    std::unordered_map<std::string, ordered_container> orderedSections;
    std::unordered_map<std::string, list_container> listSections;

    std::string delimiter = defaultDelimiter;

    void handleCommand(std::string line, std::string filename, int linecnt) {
        std::vector<std::string> tokens = strutils::split(line, " ");
        if (tokens.size() == 0) error(filename, line, linecnt, "Command expected after '!'");
        std::string cmd = tokens[0];
        tokens.erase(tokens.begin());

        if (cmd == "include") {
            parse(strutils::concat(tokens));
            return;
        }
        error(filename, line, linecnt, "Unknown command '" + cmd + "'");
    }

    void error(std::string filename, std::string line, int linecnt, std::string description) {
        std::string message = "Config parser error in file '";
        message += filename;
        message += "', line: ";
        message += std::to_string(linecnt);
        message += "\n";
        message += line;
        message += "\n";
        message += description;
        throw std::runtime_error(message);
    }

    void parseAll() {
        for (std::string filename : configFileNames) {
            parse(filename);
        }
    }

    void parse(std::string filename) {
        try {
            unordered_container mainSection;
            std::unordered_map<std::string, unordered_container> unordSectionsTmp;
            std::unordered_map<std::string, ordered_container> ordSectionsTmp;
            std::unordered_map<std::string, list_container> listSectionsTmp;

            std::ifstream file(filename.c_str());
            if (file.fail()) {
                throw std::runtime_error(
                    "can not open file '" + filename + "': " + std::string(strerror(errno))
                );
            }

            // name of current section
            std::string sectionName;
            SectionType type = UNORDERED;

            std::string line;
            std::string key, value;

            int linecnt = 0;
            while (getline(file, line)) {
                ++linecnt;
                line = strutils::trim(line, '\r');
                line = strutils::trimLeft(line);
                if (line.empty()) continue;

                // commands (only include at this moment, maybe smth more later)
                if (strutils::startsWith(line, "!")) {
                    handleCommand(strutils::trimLeft(line, '!'), filename, linecnt);
                    continue;
                }

                // comments
                if (strutils::startsWith(line, "#")) continue;

                // unordered section
                if (strutils::startsWith(line, "[")) {
                    line = strutils::trimRight(line);
                    if (!strutils::endsWith(line, "]"))
                        error(filename, line, linecnt, "Incorrect section format");
                    type = UNORDERED;
                    sectionName = line.substr(1, line.length() - 2);
                    unordSectionsTmp[sectionName];
                    continue;
                }

                // ordered section
                if (strutils::startsWith(line, "<")) {
                    line = strutils::trimRight(line);
                    if (!strutils::endsWith(line, ">"))
                        error(filename, line, linecnt, "Incorrect section format");
                    sectionName = line.substr(1, line.length() - 2);
                    type = ORDERED;
                    ordSectionsTmp[sectionName];
                    continue;
                }

                // list section
                if (strutils::startsWith(line, "{")) {
                    line = strutils::trimRight(line);
                    if (!strutils::endsWith(line, "}"))
                        error(filename, line, linecnt, "Incorrect section format");
                    sectionName = strutils::trim(line.substr(1, line.length() - 2));
                    type = LIST;
                    listSectionsTmp[sectionName];
                    continue;
                }

                // key-value parsing
                if (type != LIST) {
                    std::vector<std::string> splitt = strutils::split(line, delimiter);
                    if (splitt.size() < 2) error(filename, line, linecnt, "Incorrect line format");
                    key = strutils::trim(splitt[0]);
                    splitt.erase(splitt.begin());
                    value = strutils::trim(strutils::concat(splitt, delimiter));
                }

                switch (type) {
                    case UNORDERED: {
                        unordSectionsTmp[sectionName][key] = value;
                        break;
                    }
                    case ORDERED: {
                        ordSectionsTmp[sectionName].push_back({key, value});
                        break;
                    }
                    case LIST: {
                        listSectionsTmp[sectionName].push_back(strutils::trim(line));
                        break;
                    }
                }
            }

            // merge
            for (auto it = unordSectionsTmp.begin(); it != unordSectionsTmp.end(); ++it) {
                for (auto it1 = it->second.begin(); it1 != it->second.end(); ++it1) {
                    unorderedSections[it->first][it1->first] = it1->second;
                }
            }
            for (auto it = ordSectionsTmp.begin(); it != ordSectionsTmp.end(); ++it) {
                orderedSections[it->first] = it->second;
            }
            for (auto it = listSectionsTmp.begin(); it != listSectionsTmp.end(); ++it) {
                listSections[it->first] = it->second;
            }

            file.close();
        } catch (std::exception& ex) {
            throw std::runtime_error("Config parser error in file " + filename + "\n" + ex.what());
        } catch (...) {
            throw std::runtime_error("Config parser unknown error");
        }
    }

   public:
    _Config(std::string filename, std::string delimiter) {
        this->delimiter = delimiter;
        this->configFileNames.push_back(filename);
        parseAll();
    }

    _Config(std::vector<std::string> fileNames, std::string delimiter) {
        this->delimiter = delimiter;
        this->configFileNames = fileNames;
        parseAll();
    }

    _Config(int argc, char** argv, std::string delimiter) {
        this->delimiter = delimiter;
        for (int i = 1; i < argc; ++i) {
            configFileNames.push_back(argv[i]);
        }
        parseAll();
    }

    // only for unordered sections
    bool contains(std::string name) { return contains("", name); }

    // only for unordered sections
    bool contains(std::string unordSec, std::string name) {
        if (unorderedSections.find(unordSec) == unorderedSections.end()) return false;
        return unorderedSections[unordSec].find(name) != unorderedSections[unordSec].end();
    }

    Value& get(std::string key) { return get("", key); }

    unordered_container& getSection(std::string section) {
        if (unorderedSections.find(section) == unorderedSections.end())
            throw std::runtime_error("No such unordered section '" + section + "'");
        return unorderedSections[section];
    }

    Value& get(std::string section, std::string key) {
        unordered_container& sectionMap = getSection(section);
        if (sectionMap.find(key) == sectionMap.end())
            throw std::runtime_error("'" + key + "' not found in unordered section '" + section + "'");
        return sectionMap[key];
    }

    ordered_container& getOrderedSection(std::string section) {
        if (orderedSections.find(section) == orderedSections.end())
            throw std::runtime_error("No such ordered section '" + section + "'");
        return orderedSections[section];
    }

    Value& getOrdered(std::string section, std::string key) {
        ordered_container& sectionMap = getOrderedSection(section);
        ordered_container::iterator it;
        it = find_if(sectionMap.begin(), sectionMap.end(), [key](std::pair<std::string, Value>& p) {
            return p.first == key;
        });
        if (it == sectionMap.end())
            throw std::runtime_error("'" + key + "' not found in ordered section '" + section + "'");
        return it->second;
    }
    list_container& getList(std::string name) {
        std::unordered_map<std::string, list_container>::iterator it = listSections.find(name);
        if (it == listSections.end()) throw std::runtime_error("No such list section '" + name + "'");
        return it->second;
    }

    std::vector<std::string> getConfigFileNames() { return configFileNames; }
    std::string getConfigFileName() { return configFileNames[configFileNames.size() - 1]; }

    unordered_container& getMainSection() { return unorderedSections[""]; }
    std::unordered_map<std::string, ordered_container>& getAllOrdered() { return orderedSections; }
    std::unordered_map<std::string, unordered_container>& getAllUnordered() { return unorderedSections; }
    std::unordered_map<std::string, list_container>& getAllLists() { return listSections; }

    std::string dump() {
        std::stringstream ss;

        unordered_container& mainSection = getMainSection();
        ss << strutils::to_string(mainSection);

        for (auto section = unorderedSections.begin(); section != unorderedSections.end(); ++section) {
            if (section->second == mainSection) continue;
            ss << "[" << section->first << "]" << std::endl;
            ss << strutils::to_string(section->second);
        }

        for (auto section = orderedSections.begin(); section != orderedSections.end(); ++section) {
            ss << "<" << section->first << ">" << std::endl;
            ss << strutils::to_string(section->second);
        }

        for (auto section = listSections.begin(); section != listSections.end(); ++section) {
            ss << "{" << section->first << "}" << std::endl;
            ss << strutils::to_string(section->second);
        }

        return ss.str();
    }
};

class Config : protected std::shared_ptr<_Config> {
   public:
    Config() : std::shared_ptr<_Config>() {}
    Config(std::string filename, std::string delimiter = defaultDelimiter)
        : std::shared_ptr<_Config>(new _Config(filename, delimiter)) {}
    Config(std::vector<std::string> filenames, std::string delimiter = defaultDelimiter)
        : std::shared_ptr<_Config>(new _Config(filenames, delimiter)) {}
    Config(int argc, char** argv, std::string delimiter = defaultDelimiter)
        : std::shared_ptr<_Config>(new _Config(argc, argv, delimiter)) {}

    operator bool() { return std::shared_ptr<_Config>::operator bool(); }
    _Config* operator->() { return std::shared_ptr<_Config>::operator->(); }
};

inline Config _globalConfig;

inline void initConfig(std::string filename, std::string delimiter = defaultDelimiter) {
    _globalConfig = Config(filename);
}
inline void initConfig(std::vector<std::string> filenames, std::string delimiter = defaultDelimiter) {
    _globalConfig = Config(filenames);
}
inline void initConfig(int argc, char** argv, std::string delimiter = defaultDelimiter) {
    _globalConfig = Config(argc, argv);
}

inline Config getConfig() {
    if (!_globalConfig) throw std::runtime_error("Global config parser is not initialized");
    return _globalConfig;
}
}  // namespace cfgparser

#endif