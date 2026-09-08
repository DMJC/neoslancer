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

IniFile::Section& IniFile::sectionFor(const std::string& sectionName) {
    const std::string lower = toLower(sectionName);
    auto it = m_sections.find(lower);
    if (it == m_sections.end()) {
        Section section;
        section.originalName = sectionName;
        it = m_sections.emplace(lower, std::move(section)).first;
        m_sectionOrder.push_back(lower);
    }
    return it->second;
}

bool IniFile::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    m_sections.clear();
    m_sectionOrder.clear();

    std::string currentSectionLower;
    bool haveSection = false;
    std::string line;
    while (std::getline(file, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') {
            continue;
        }

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            const std::string sectionName = trimmed.substr(1, trimmed.size() - 2);
            sectionFor(sectionName);
            currentSectionLower = toLower(sectionName);
            haveSection = true;
            continue;
        }

        if (!haveSection) {
            continue;
        }

        const size_t equals = trimmed.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = trim(trimmed.substr(0, equals));
        const std::string value = trim(trimmed.substr(equals + 1));
        const std::string keyLower = toLower(key);

        Section& section = m_sections[currentSectionLower];
        if (section.values.find(keyLower) == section.values.end()) {
            section.keyOrder.push_back(keyLower);
        }
        section.keyOriginalCase[keyLower] = key;
        section.values[keyLower] = value;
    }

    return true;
}

bool IniFile::save(const std::string& path) const {
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    for (const std::string& sectionLower : m_sectionOrder) {
        const Section& section = m_sections.at(sectionLower);
        file << "[" << section.originalName << "]\n";
        for (const std::string& keyLower : section.keyOrder) {
            file << section.keyOriginalCase.at(keyLower) << "=" << section.values.at(keyLower) << "\n";
        }
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
    const auto keyIt = sectionIt->second.values.find(toLower(key));
    if (keyIt == sectionIt->second.values.end()) {
        return defaultValue;
    }
    return keyIt->second;
}

void IniFile::setInt(const std::string& section, const std::string& key, int value) {
    setString(section, key, std::to_string(value));
}

void IniFile::setString(const std::string& section, const std::string& key, const std::string& value) {
    Section& target = sectionFor(section);
    const std::string keyLower = toLower(key);
    if (target.values.find(keyLower) == target.values.end()) {
        target.keyOrder.push_back(keyLower);
        target.keyOriginalCase[keyLower] = key;
    }
    target.values[keyLower] = value;
}

} // namespace neoslancer
