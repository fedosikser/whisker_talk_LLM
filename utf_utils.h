#pragma once

#include <codecvt>
#include <locale>
#include <string>

inline std::wstring utf8ToWide(const std::string& text) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(text);
}

inline std::string wideToUtf8(const std::wstring& text) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(text);
}
