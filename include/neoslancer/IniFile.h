#pragma once

#include <map>
#include <string>

namespace neoslancer {

// Windows-INI-style key/value store, read with the same case-insensitive
// section/key lookup semantics as the original engine's
// GetPrivateProfileIntA/GetPrivateProfileStringA calls (see
// reversing/reverse_engineered_functions.md, WinMain step 7).
class IniFile {
public:
    bool load(const std::string& path);

    int getInt(const std::string& section, const std::string& key, int defaultValue) const;
    std::string getString(const std::string& section, const std::string& key,
                           const std::string& defaultValue) const;

private:
    // section -> (key -> value), keys/section names stored lowercase.
    std::map<std::string, std::map<std::string, std::string>> m_sections;
};

} // namespace neoslancer
