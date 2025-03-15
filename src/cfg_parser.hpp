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

typedef std::unordered_map<std::string, std::string> unordered_container;
typedef std::vector<std::pair<std::string, std::string>> ordered_container;
typedef std::vector<std::string> list_container;

namespace strutils {

inline list_container split(const std::string& s, std::string delimiter, bool dropEmptyTokens = false) {
    list_container res;
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

inline std::string concat(const list_container& tokens, std::string delimiter = " ") {
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

inline bool startsWith(std::string s, char ch) { return s.length() > 0 && s[0] == ch; }
inline bool startsWith(const std::string& s, std::string token) {
    if (s.length() < token.length()) return false;
    for (int i = 0; i < token.length(); ++i) {
        if (s[i] != token[i]) return false;
    }
    return true;
}
inline bool endsWith(std::string s, char ch) { return s.length() > 0 && s[0] == ch; }
inline bool endsWith(std::string s, std::string token) {
    if (s.length() < token.length()) return false;
    for (int i = 1; i <= token.length(); ++i) {
        if (token[token.length() - i] != s[s.length() - i]) return false;
    }
    return true;
}

}  // namespace strutils

class _CfgParser {
   private:
    enum SectionType { UNORDERED, ORDERED, LIST };
    std::vector<std::string> settingsFileNames;
    std::unordered_map<std::string, unordered_container> unorderedSections;
    std::unordered_map<std::string, ordered_container> orderedSections;
    std::unordered_map<std::string, list_container> listSections;

    std::string delimiter = " = ";

    void handleCommand(std::string line) {
        std::vector<std::string> tokens = strutils::split(line, " ");
        if (tokens.size() == 0) throw std::runtime_error("Command expected after !");
        std::string cmd = tokens[0];
        tokens.erase(tokens.begin());

        if (cmd == "include") {
            parse(strutils::concat(tokens));
            return;
        }
        throw std::runtime_error("Unknown command '" + cmd + "'");
    }
    void parse(std::string filename) {
        try {
            unordered_container mainSection;
            std::unordered_map<std::string, unordered_container> unordSectionsTmp;
            std::unordered_map<std::string, ordered_container> ordSectionsTmp;
            std::unordered_map<std::string, list_container> listSectionsTmp;

            std::ifstream file(filename.c_str());
            if (file.fail()) {
                throw std::runtime_error("can not open file: " + std::string(strerror(errno)));
            }

            // текущая секции
            std::string sectionName;
            SectionType type = UNORDERED;

            std::string line;
            std::string key, value;

            int linecnt = 1;
            while (getline(file, line)) {
                ++linecnt;
                line = strutils::trim(line, '\r');
                line = strutils::trimLeft(line);
                if (line.empty()) continue;

                // комманды (инклюды и в будущем возможно что-то еще)
                if (strutils::startsWith(line, "!")) {
                    handleCommand(strutils::trimLeft(line, '!'));
                    continue;
                }

                // комментарии
                if (strutils::startsWith(line, "#")) continue;

                // неупорядоченные секции
                if (strutils::startsWith(line, "[")) {
                    line = strutils::trimRight(line);
                    if (!strutils::endsWith(line, "]"))
                        throw std::runtime_error(
                            "Incorrect section format at line" + std::to_string(linecnt)
                        );
                    type = UNORDERED;
                    sectionName = line.substr(1, line.length() - 2);
                    unordSectionsTmp[sectionName];
                    continue;
                }

                // упорядоченные секции
                if (strutils::startsWith(line, "<")) {
                    line = strutils::trimRight(line);
                    if (!strutils::endsWith(line, ">"))
                        throw std::runtime_error(
                            "Incorrect section format at line" + std::to_string(linecnt)
                        );
                    sectionName = line.substr(1, line.length() - 2);
                    if (sectionName.empty()) throw std::runtime_error("Invalid name for ordered section");
                    type = ORDERED;
                    ordSectionsTmp[sectionName];
                    continue;
                }

                // секция-список
                if (strutils::startsWith(line, "{")) {
                    line = strutils::trimRight(line);
                    if (!strutils::endsWith(line, "}"))
                        throw std::runtime_error(
                            "Incorrect section format at line" + std::to_string(linecnt)
                        );
                    sectionName = strutils::trim(line.substr(1, line.length() - 2));
                    if (sectionName.empty()) throw std::runtime_error("Invalid name for list section");
                    type = LIST;
                    listSectionsTmp[sectionName];
                    continue;
                }

                // парсинг настройки
                if (type != LIST) {
                    std::vector<std::string> splitt = strutils::split(line, delimiter);
                    if (splitt.size() < 2)
                        throw std::runtime_error("incorrect line format at line " + std::to_string(linecnt));
                    key = strutils::trim(splitt[0]);
                    splitt.erase(splitt.begin());
                    value = strutils::trim(strutils::concat(splitt, delimiter));
                }

                // определяем куда класть
                switch (type) {
                    case UNORDERED: {
                        unordSectionsTmp[sectionName][key] = value;
                        break;
                    }
                    case ORDERED: {
                        ordSectionsTmp[sectionName].push_back(std::pair<std::string, std::string>(key, value)
                        );
                        break;
                    }
                    case LIST: {
                        listSectionsTmp[sectionName].push_back(strutils::trim(line));
                        break;
                    }
                }
            }

            // мерж
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
            throw std::runtime_error("Settings loader error in file " + filename + "\n" + ex.what());
        } catch (...) {
            throw std::runtime_error("Failed to load settings");
        }
    }

   public:
    _CfgParser(std::string filename) {
        this->settingsFileNames.push_back(filename);
        parse(filename);
    }

    _CfgParser(std::vector<std::string> fileNames) {
        this->settingsFileNames = fileNames;
        for (std::string name : fileNames) {
            parse(name);
        }
    }

    _CfgParser(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            parse(argv[i]);
            settingsFileNames.push_back(argv[i]);
        }
    }

    bool contains(std::string name) { return contains("", name); }

    bool contains(std::string unordSec, std::string name) {
        if (unorderedSections.find(unordSec) == unorderedSections.end()) return false;
        return unorderedSections[unordSec].find(name) != unorderedSections[unordSec].end();
    }

    std::string getSetting(std::string key) { return getSetting("", key); }

    unordered_container& getUnorderedSection(std::string section) {
        if (unorderedSections.find(section) == unorderedSections.end())
            throw std::runtime_error(section + " not found in unordered sections");
        return unorderedSections[section];
    }

    std::string getSetting(std::string section, std::string key) {
        unordered_container& sectionMap = getUnorderedSection(section);
        if (sectionMap.find(key) == sectionMap.end())
            throw std::runtime_error(key + " not found in section " + section);
        return sectionMap[key];
    }

    ordered_container& getOrderedSection(std::string section) {
        if (orderedSections.find(section) == orderedSections.end())
            throw std::runtime_error(section + " not found in ordered sections");
        return orderedSections[section];
    }

    std::string getOrderedSetting(std::string section, std::string key) {
        ordered_container& sectionMap = getOrderedSection(section);
        ordered_container::iterator it;
        it = find_if(sectionMap.begin(), sectionMap.end(), [key](std::pair<std::string, std::string>& p) {
            return p.first == key;
        });
        if (it == sectionMap.end()) throw std::runtime_error(key + " not found in section " + section);
        return it->second;
    }
    list_container getList(std::string name) {
        std::unordered_map<std::string, list_container>::iterator it = listSections.find(name);
        if (it == listSections.end()) throw std::runtime_error(name + " not found in list sections");
        return it->second;
    }

    std::vector<std::string> getSettingsFileNames() { return settingsFileNames; }
    std::string getSettingsFileName() { return settingsFileNames[settingsFileNames.size() - 1]; }

    unordered_container& getMainSection() { return unorderedSections[""]; }
    std::unordered_map<std::string, ordered_container>& getOrdered() { return orderedSections; }
    std::unordered_map<std::string, unordered_container>& getUnordered() { return unorderedSections; }
    std::unordered_map<std::string, list_container>& getLists() { return listSections; }
};

class CfgParser : public std::shared_ptr<_CfgParser> {
   public:
    CfgParser() : std::shared_ptr<_CfgParser>() {}
    CfgParser(std::string filename) : std::shared_ptr<_CfgParser>(new _CfgParser(filename)) {}
    CfgParser(std::vector<std::string> filenames) : std::shared_ptr<_CfgParser>(new _CfgParser(filenames)) {}
    CfgParser(int argc, char** argv) : std::shared_ptr<_CfgParser>(new _CfgParser(argc, argv)) {}
};

inline CfgParser _globalCfgParser;

inline void initCfgParser(std::string filename) { _globalCfgParser = CfgParser(filename); }
inline void initCfgParser(std::vector<std::string> filenames) { _globalCfgParser = CfgParser(filenames); }
inline void initCfgParser(int argc, char** argv) { _globalCfgParser = CfgParser(argc, argv); }

inline CfgParser getCfgParser() {
    if (!_globalCfgParser) throw std::runtime_error("Global config parser is not initialized");
    return _globalCfgParser;
}
}  // namespace cfgparser

#endif