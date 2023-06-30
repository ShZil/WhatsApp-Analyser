#include <string>
#include <codecvt>
#include <locale>

// from: https://stackoverflow.com/questions/51352190/read-multi-language-file-wchar-t-vs-char
// not sure whether all this works well. be skeptical
// consider not using wchar_t and udf-8 decoding if at all possible
std::string narrow(const std::wstring& wide_string)
{
    std::wstring_convert <std::codecvt_utf8 <wchar_t>, wchar_t> convert;
    return convert.to_bytes(wide_string);
}

std::wstring widen(const std::string& utf8_string)
{
    std::wstring_convert <std::codecvt_utf8 <wchar_t>, wchar_t> convert;
    return convert.from_bytes(utf8_string);
}
