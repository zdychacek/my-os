#pragma once

namespace std
{
  template <typename T>
  class Array
  {
  private:
    T *m_array;
    uint64_t m_size;

  public:
    Array() : m_array(nullptr), m_size(0) {}
    ~Array()
    {
      if (m_array)
      {
        delete[] m_array;
      }
    }
    Array(Array &other) : m_array(nullptr), m_size(0)
    {
      Resize(other.m_size);

      for (size_t i = 0; i < m_size; i++)
      {
        m_array[i] = other.m_array[i];
      }
    }

    uint64_t Size() const { return m_size; }

    T &operator[](int index) { return m_array[index]; }
    const T &operator[](int index) const { return m_array[index]; }

    void Resize(uint64_t nsize)
    {
      T *narray = new T[nsize];

      for (size_t i = 0; i < m_size && i < nsize; i++)
      {
        narray[i] = m_array[i];
      }

      if (m_size)
      {
        delete[] m_array;
      }

      m_array = narray;
      m_size = nsize;
    }
  };
}
