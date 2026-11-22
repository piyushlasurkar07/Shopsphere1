#ifndef SHOPSPHERE_STRING_UTILS_H
#define SHOPSPHERE_STRING_UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <iomanip>

namespace ShopSphere {
namespace Utils {

inline std::string trim(const std::string &str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

inline std::string toLower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

inline std::string toUpper(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return result;
}

inline std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        std::string trimmed = trim(token);
        if (!trimmed.empty()) {
            tokens.push_back(trimmed);
        }
    }
    return tokens;
}

inline std::string sanitizeHtml(const std::string &input) {
    std::string output;
    output.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&':  output.append("&amp;"); break;
            case '<':  output.append("&lt;"); break;
            case '>':  output.append("&gt;"); break;
            case '"':  output.append("&quot;"); break;
            case '\'': output.append("&#x27;"); break;
            default:   output.push_back(c); break;
        }
    }
    return output;
}

inline int levenshteinDistance(const std::string &s1, const std::string &s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();
    if (m == 0) return static_cast<int>(n);
    if (n == 0) return static_cast<int>(m);

    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));
    for (size_t i = 0; i <= m; ++i) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= n; ++j) dp[0][j] = static_cast<int>(j);

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            int cost = (std::tolower(static_cast<unsigned char>(s1[i - 1])) == 
                        std::tolower(static_cast<unsigned char>(s2[j - 1]))) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,     // deletion
                dp[i][j - 1] + 1,     // insertion
                dp[i - 1][j - 1] + cost // substitution
            });
        }
    }
    return dp[m][n];
}

inline std::string soundex(const std::string &s) {
    if (s.empty()) return "";
    std::string clean;
    for (char c : s) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            clean.push_back(std::toupper(static_cast<unsigned char>(c)));
        }
    }
    if (clean.empty()) return "";

    std::string result;
    result.push_back(clean[0]);

    auto getCode = [](char c) -> char {
        switch (c) {
            case 'B': case 'F': case 'P': case 'V': return '1';
            case 'C': case 'G': case 'J': case 'K': case 'Q': case 'S': case 'X': case 'Z': return '2';
            case 'D': case 'T': return '3';
            case 'L': return '4';
            case 'M': case 'N': return '5';
            case 'R': return '6';
            default: return '0';
        }
    };

    char prevCode = getCode(clean[0]);
    for (size_t i = 1; i < clean.size() && result.size() < 4; ++i) {
        char code = getCode(clean[i]);
        if (code != '0' && code != prevCode) {
            result.push_back(code);
        }
        prevCode = code;
    }
    while (result.size() < 4) {
        result.push_back('0');
    }
    return result;
}

inline bool isValidEmail(const std::string &email) {
    auto atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0 || atPos == email.size() - 1) return false;
    auto dotPos = email.find('.', atPos);
    if (dotPos == std::string::npos || dotPos == atPos + 1 || dotPos == email.size() - 1) return false;
    return true;
}

} // namespace Utils
} // namespace ShopSphere

#endif // SHOPSPHERE_STRING_UTILS_H
