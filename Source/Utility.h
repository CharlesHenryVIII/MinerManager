#pragma once

#include "Types.h"

#include <cassert>
#include <string>

#define arrsize(arr__) (sizeof(arr__) / sizeof(arr__[0]))
#define BIT(num) (1<<(num))

struct SplitText {
    std::string before;
    std::string after;
};

template <typename T>
[[nodiscard]] T Min(T a, T b)
{
    return a < b ? a : b;
}

 template <typename T>
[[nodiscard]] T Max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
[[nodiscard]] T Clamp(T v, T min, T max)
{
    return Max(min, Min(max, v));
}

std::vector<int32> TextToIntArray(const char* text, const char lineEnd);
std::vector<int32> TextToIntArray(const char* text);
std::vector<int32> FileToIntArray(const char* fileName, const char lineEnd);
std::vector<int32> FileToIntArray(const char* fileName);
int32 TextDetection(const std::string& string, const std::string& check);
void TextRemoval(std::string& string, const std::string& removalRef);
void TextRemoval(std::string& string, const std::string& remove, const std::string& removeFrom);
void TextAddition(std::string& string, const std::string& additionalText, const std::string& after);
SplitText TextSplit(const std::string& input, const std::string& splitString);
std::vector<std::string> TextToStringArray(const char* text);
std::vector<std::string> TextToStringArray(const char* text, const char* lineEnd);
std::vector<std::string> FileToStringArray(const char* fileName);
std::vector<std::string> FileToStringArray(const char* fileName, const char* lineEnd);
int32 NumberLengthInString(const std::string& string, int32& i);
int32 StringToInt(const std::string& string, int32 i, int32 length);
int32 StringToInt(const std::string& string, int32 i);
bool FindString_ExactCaseInsensitive(const std::string& x, const std::string& y);
bool FindStringCaseInsensitive(const std::string& str1, const std::string& str2);
