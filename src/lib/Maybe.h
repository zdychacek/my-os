#pragma once

namespace std
{
  template <typename T>
  class Maybe
  {
  private:
    uint8_t m_data[sizeof(T)];
    T *m_pointer;

  public:
    Maybe() : m_pointer(nullptr) {}
    Maybe(T &t)
    {
      void *place = m_data;
      m_pointer = new (place) T(t);
    }
    Maybe(const T &t)
    {
      void *place = m_data;
      m_pointer = new (place) T(t);
    }
    ~Maybe()
    {
      if (m_pointer)
        m_pointer->~T();
    }

    T *operator->()
    {
      return m_pointer;
    }

    T &operator*()
    {
      return *m_pointer;
    }

    bool good() const
    {
      return !!m_pointer;
    }

    operator bool() const
    {
      return good();
    }
  };
}
