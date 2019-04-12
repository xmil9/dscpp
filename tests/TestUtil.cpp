#include "TestUtil.h"
#include <iostream>
#include <string>


static std::string ComposeErrorMessage(const std::string& label,
                                       const std::string& condStr,
                                       const std::string& fileName, int lineNum)
{
   std::string msg;
   msg += "Failed - ";
   msg += label;
   msg += ": '";
   msg += condStr;
   msg += "'.";
   msg += " (file: ";
   msg += fileName;
   msg += ", line: ";
   msg += std::to_string(lineNum);
   msg += ").";
   return msg;
}


bool Verify(bool cond, const std::string& label, const std::string& condStr,
            const std::string& fileName, int lineNum)
{
   if (!cond)
      std::cout << ComposeErrorMessage(label, condStr, fileName, lineNum) << "\n";
   return cond;
}
