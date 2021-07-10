#pragma once

#include <lib/unique_ptr.h>
#include <utils/SingletonFactory.h>
#include <debug/Assertion.h>

namespace Utils
{
  template <typename T>
  class Singleton
  {
    friend class SingletonFactory<T>;

  protected:
    Singleton()
    {
    }

    Singleton(const Singleton &other)
    {
    }

    Singleton &operator=(const Singleton &other)
    {
      return *this;
    }

  public:
    static T *GetInstance()
    {
      Debug::Assertion(m_instance.Get(), "SingletonFactory<>::create(...) call must precede Singleton<>::instance() call.");

      return m_instance.Get();
    }

  protected:
    static std::unique_ptr<T> m_instance;
  };

  template <typename T>
  std::unique_ptr<T> Singleton<T>::m_instance;
}
