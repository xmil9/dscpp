#pragma once
#include <string>


bool Verify(bool cond, const std::string& label, const std::string& condStr,
            const std::string& fileName, int lineNum);

#define VERIFY(cond, label) (Verify(cond, label, #cond, __FILE__, __LINE__))
