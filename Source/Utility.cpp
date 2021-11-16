#include <vector>
#include <string_view>

#include "Utility.h"

const char* ReadEntireFileAsString(const char* fileName)
{
    FILE* file;
    file = fopen(fileName, "rb");
    fseek(file, 0, SEEK_END);
    const long fileLength = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[fileLength + 1];
    fread(buffer, 1, fileLength, file);
    fclose(file);
    buffer[fileLength] = 0;
    return buffer;
}

std::vector<int32> TextToIntArray(const char* text, const char lineEnd)
{
    std::vector<int32> workingNums;
    std::vector<int32> nums;
    workingNums.reserve(10);
    nums.reserve(16);

    for (int32 i = 0; text[i] != 0; i++)
    {

        //assert((data.text[i] - '0' >= 0  && data.text[i] - '0' <= '9') || data.text[i] == lineEnd);
        if (text[i] - '0' >= 0  && text[i] - '0' <= '9')
        {
            int32 value = text[i] - '0';
            workingNums.push_back(value);
        }
        else if (text[i] == lineEnd)
        {
            int32 value = 0;
            for (int32 j = 0; j < workingNums.size(); j++)
            {
                value += (workingNums[j] * (int)pow(10.0f, float(workingNums.size() - 1 - j)));
            }
            assert(value); //should not occurr
            if (value)
            {
                nums.push_back(value);
                workingNums.clear();
            }
        }
    }
    delete text;
    return nums;
}

std::vector<int32> TextToIntArray(const char* text)
{
    std::vector<int> nums;
    //std::string workingText = text;
    std::string_view test = text;
    int32 index = 0;

    for (int32 i = 0; i == 0 || text[i - 1] != 0; i++)
    {
        if (text[i] >= '0' && text[i] <= '9')
            index++;
        else if (index)
        {
            //nums.push_back(atoi(workingText.substr(i - index, index).c_str()));
            nums.push_back(atoi(test.substr(i - index, index).data()));
            index = 0;
        }
    }
    return nums;
}

int32 TextDetection(const std::string& string, const std::string& check)
{
#if 1
    auto result = string.find(check);
    if (result != std::string::npos)
        return (int32)result;
    return 0;

#else
    for (int32 i = 0; i + check.size() <= string.size(); i++)
    {
        if (string.substr(i, check.size()) == check)
            return true;
    }
    return false;
#endif
}

void TextRemoval(std::string& string, const std::string& removalRef)
{
    std::string_view sv = string;
    auto begin = string.begin();
    auto end = string.begin();
    int32 i = 0;
    while (end != string.end())
    {
        if (sv.substr(i, removalRef.size()) == removalRef)
        {
            begin = end;
            for (int32 j = 0; j < removalRef.size(); j++)
                end++;
            string.erase(begin, end);
            end = string.begin();
            i = 0;
            //return;

        }
        else
        {
            end++;
            i++;
        }
    }
}

void TextAddition(std::string& string, const std::string& additionalText, const std::string& after)
{
    std::string_view sv = string;
    for (int32 i = 0; i < string.size() - after.size(); i++)
    {
        if (sv.substr(i, (after.size())) == after && !(sv.substr(i, (after.size() + additionalText.size())) == (after + additionalText)))
        {
            string.insert(i + after.size(), additionalText);
        }
    }
}

SplitText TextSplit(const std::string& input, const std::string& splitString)
{
    SplitText result = {};
    
    if (int32 splitLocation = TextDetection(input, splitString))
    {
        result.before = input.substr(0, splitLocation);
        result.after = input.substr(splitLocation + 1, input.size() - (splitLocation + 1));
    }
    return result;
}

std::vector<std::string> TextToStringArray(const char* text)
{
    std::string_view sv = text;
    std::vector<std::string> result;

    int32 tokenLength = 0;
    for (int32 i = 0; text[i - 1] != 0; i++)
    {
        if (text[i] >= ' ' && text[i] <= '~')
        {
            tokenLength++;
        }
        else
        {
            if (tokenLength)
            {

                std::string token;
                token = sv.substr(i - tokenLength, tokenLength);
                result.push_back(token);
                tokenLength = 0;
            }
        }
    }
    assert(tokenLength == 0);
    return result;
}

std::vector<std::string> TextToStringArray(const char* text, const char* lineEnd)
{
    assert(lineEnd);
    assert(text);

    std::string_view sv = text;
    std::vector<std::string> result;

    int32 tokenLength = 0;
    for (int32 i = 0; text[i] != 0; i++)
    {
        bool isLineEnd = false;
        if (text[i] == lineEnd[0] || text[i + 1] == 0)
        {
            int32 incrimenter = 0;
            while (text[i + incrimenter] == lineEnd[incrimenter])
            {

                if (lineEnd[incrimenter + 1] == 0)
                {
                    isLineEnd = true;
                    break;
                }
                incrimenter++;
            }
            if (tokenLength && (isLineEnd || text[i + 1] == 0))
            {

                std::string token;
                token = sv.substr(i - tokenLength, tokenLength + 1);
                result.push_back(token);
                tokenLength = 0;
                i += incrimenter;
                isLineEnd = true;
            }
        }
        if (!isLineEnd)
            tokenLength++;
        
    }
    assert(tokenLength == 0);
    return result;
}

std::vector<std::string> FileToStringArray(const char* fileName)
{
    return TextToStringArray(ReadEntireFileAsString(fileName));
}

std::vector<std::string> FileToStringArray(const char* fileName, const char* lineEnd)
{
    return TextToStringArray(ReadEntireFileAsString(fileName), lineEnd);
}

std::vector<int32> FileToIntArray(const char* fileName, const char lineEnd)
{
    return TextToIntArray(ReadEntireFileAsString(fileName), lineEnd);
}

std::vector<int32> FileToIntArray(const char* fileName)
{
    return TextToIntArray(ReadEntireFileAsString(fileName));
}

int32 NumberLengthInString(const std::string& string, int32& i)
{

    int32 j = 0;
    while (string[i + j] >= '0' && string[i + j] <= '9')
        j++;
    return j;
}

int32 StringToInt(const std::string& string, int32 i, int32 length)
{
    return atoi(string.substr(i, length).c_str());
}

int32 StringToInt(const std::string& string, int32 i)
{
    return StringToInt(string, i, NumberLengthInString(string, i));
}

#include <algorithm>
/// Try to find in the Haystack the Needle - ignore case
bool FindStringCaseInsensitive(const std::string& str1, const std::string& str2)
{
  auto it = std::search(
    str1.begin(), str1.end(),
    str2.begin(),   str2.end(),
    [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
  );
  return (it != str1.end() );
}

