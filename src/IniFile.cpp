#include "neoslancer/IniFile.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>

namespace neoslancer {

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string trim(const std::string& s) {
    const char* whitespace = " \t\r\n";
    const size_t begin = s.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return "";
    }
    const size_t end = s.find_last_not_of(whitespace);
    return s.substr(begin, end - begin + 1);
}

} // namespace

bool IniFile::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    m_sections.clear();
    std::string currentSection;
    std::string line;
    while (std::getline(file, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') {
            continue;
        }

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = toLower(trimmed.substr(1, trimmed.size() - 2));
            continue;
        }

        const size_t equals = trimmed.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = toLower(trim(trimmed.substr(0, equals)));
        const std::string value = trim(trimmed.substr(equals + 1));
        m_sections[currentSection][key] = value;
    }

    return true;
}

int IniFile::getInt(const std::string& section, const std::string& key, int defaultValue) const {
    const std::string value = getString(section, key, "");
    if (value.empty()) {
        return defaultValue;
    }
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str()) {
        return defaultValue;
    }
    return static_cast<int>(parsed);
}

std::string IniFile::getString(const std::string& section, const std::string& key,
                                const std::string& defaultValue) const {
    const auto sectionIt = m_sections.find(toLower(section));
    if (sectionIt == m_sections.end()) {
        return defaultValue;
    }
    const auto keyIt = sectionIt->second.find(toLower(key));
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    return keyIt->second;
}

} // namespace neoslancer
