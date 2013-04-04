#pragma once
class NonCopyable
{
  protected:
    NonCopyable () {}
    ~NonCopyable () throw() {} /// Protected non-virtual destructor
  private: 
    NonCopyable (const NonCopyable &);
    NonCopyable & operator = (const NonCopyable &);
};