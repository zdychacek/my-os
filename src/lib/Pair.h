#pragma once

namespace std
{
  template <typename T1, typename T2>
  class Pair
  {
  private:
    T1 m_t1;
    T2 m_t2;

  public:
    Pair(T1 &t1, T2 &t2) : m_t1(t1), m_t2(t2) {}

    T1 &first() { return m_t1; }
    const T1 &first() const { return m_t1; }
    T2 &second() { return m_t2; }
    const T2 &second() const { return m_t2; }
  };
}
