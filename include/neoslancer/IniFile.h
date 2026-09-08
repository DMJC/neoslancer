#pragma once

#include <map>
#include <string>
#include <vector>

namespace neoslancer {

// Windows-INI-style key/value store, read with the same case-insensitive
// section/key lookup semantics as the original engine's
// GetPrivateProfileIntA/GetPrivateProfileStringA calls (see
// reversing/reverse_engineered_functions.md, WinMain step 7). Also
// supports writing values back out, matching the options screens'
// confirmed WritePrivateProfileStringA usage (Sound/Video options,
// confidence_db.md's RunMenuScreenLoop screen table).
class IniFile {
public:
    bool load(const std::string& path);
    bool save(const std::string& path) const;

    int getInt(const std::string& section, const std::string& key, int defaultValue) const;
    std::string getString(const std::string& section, const std::string& key,
                           const std::string& defaultValue) const;

    void setInt(const std::string& section, const std::string& key, int value);
    void setString(const std::string& section, const std::string& key, const std::string& value);

private:
    struct Section {
        std::string originalName;
        std::map<std::string, std::string> keyOriginalCase; // lowercase key -> original-case key
        std::map<std::string, std::string> values;           // lowercase key -> value
        std::vector<std::string> keyOrder;                    // lowercase keys, first-seen order
    };

    Section& sectionFor(const std::string& sectionName);

    std::map<std::string, Section> m_sections; // lowercase section name -> Section
    std::vector<std::string> m_sectionOrder;   // lowercase section names, first-seen order
};

} // namespace neoslancer
