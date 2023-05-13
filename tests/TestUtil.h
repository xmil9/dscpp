#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <typeinfo>


namespace test_detail
{

inline std::string composeErrorMessage(const std::string& label,
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


template <typename Exception>
std::string composeErrorMessage(const std::string& label, const std::string& fileName,
                                int lineNum)
{
   std::string msg;
   msg += "Failed - ";
   msg += label;
   msg += ": '";
   msg += "Exception ";
   msg += typeid(Exception).name();
   msg += "' not thrown.";
   msg += " (file: ";
   msg += fileName;
   msg += ", line: ";
   msg += std::to_string(lineNum);
   msg += ").";
   return msg;
}

} // namespace test_detail


inline bool verify(bool cond, const std::string& label, const std::string& condStr,
                   const std::string& fileName, int lineNum)
{
   if (!cond)
      std::cout << test_detail::composeErrorMessage(label, condStr, fileName, lineNum)
                << "\n";
   return cond;
}


template <typename Exception>
bool verifyThrow(std::function<void()> toVerify, const std::string& label,
                 const std::string& fileName, int lineNum)
{
   bool threwException = false;
   try
   {
      toVerify();
   }
   catch (const Exception&)
   {
      threwException = true;
   }
   catch (...)
   {
   }

   if (!threwException)
      std::cout << test_detail::composeErrorMessage<Exception>(label, fileName, lineNum)
                << "\n";
   return threwException;
}


#define VERIFY(cond, label) (verify(cond, label, #cond, __FILE__, __LINE__))
#define VERIFY_THROW(toVerify, Exception, label)                                         \
   (verifyThrow<Exception>(toVerify, label, __FILE__, __LINE__))
