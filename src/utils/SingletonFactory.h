#pragma once

namespace Utils
{
  template <typename T, typename... Args>
  class SingletonFactory
  {
  public:
    virtual ~SingletonFactory() {}

    T *Create(Args... args)
    {
      if (m_isCreated)
      {
        return T::m_instance.Get();
      }

      m_isCreated = true;

      T *tmp = new T(args...);
      T::m_instance.Reset(tmp);

      return T::m_instance.Get();
    }

    void Destroy()
    {
      T::m_instance.Reset();
      m_isCreated = false;
    }

  private:
    static bool m_isCreated;
  };

  template <typename T, typename... Args>
  bool SingletonFactory<T, Args...>::m_isCreated = false;
}
