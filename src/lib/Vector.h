#pragma once

#include <types.h>

namespace std
{
  template <typename T>
  class Vector
  {
  public:
    struct Iterator
    {
      uint64_t m_index;
      const Vector<T> *vector;

      bool operator==(const Iterator &other) const
      {
        return vector == other.vector && other.m_index == m_index;
      }
      bool operator!=(const Iterator &other) const
      {
        return !(other == *this);
      }
      Iterator &operator++()
      {
        m_index++;
        return *this;
      }

      T &operator*()
      {
        return vector->m_storage.t[m_index];
      }
    };

  private:
    union Storage
    {
      uint8_t *raw;
      T *t;
    } m_storage;
    uint64_t m_used;
    uint64_t m_allocated;

  public:
    Vector() : m_used(0), m_allocated(0)
    {
      m_storage.raw = nullptr;
    }
    Vector(const Vector &other)
    {
      m_used = 0;
      m_allocated = 0;
      m_storage.raw = nullptr;

      resize(other.m_allocated);

      for (uint64_t i = 0; i < other.m_used; i++)
      {
        new ((void *)&m_storage.t[i]) T(other.m_storage.t[i]);
      }
      m_used = other.m_used;
    }
    ~Vector()
    {
      for (uint64_t i = 0; i < m_used; i++)
      {
        m_storage.t[i].~T();
      }
      delete[] m_storage.raw;
    }

    size_t size() const { return m_used; }

    Iterator begin() const
    {
      return Iterator{0, this};
    }

    Iterator end() const
    {
      return Iterator{m_used, this};
    }

    void push(const T &t)
    {
      if (m_used == m_allocated)
      {
        if (m_used)
          resize(m_used * 2);
        else
          resize(1);
      }
      new ((void *)&m_storage.t[m_used++]) T(t);
    }

    void pop()
    {
      m_used--;
      m_storage.t[m_used].~T();
    }

    T &back()
    {
      return m_storage.t[m_used - 1];
    }
    const T &back() const
    {
      return m_storage.t[m_used - 1];
    }

    T &front()
    {
      return m_storage.t[0];
    }
    const T &front() const
    {
      return m_storage.t[0];
    }

    T &operator[](int64_t index)
    {
      return m_storage.t[index];
    }
    const T &operator[](int64_t index) const
    {
      return m_storage.t[index];
    }

  private:
    void resize(size_t nsize)
    {
      Storage ns;

      ns.raw = new uint8_t[nsize * sizeof(T)];

      for (size_t i = 0; i < m_used && i < nsize; i++)
      {
        new ((void *)&ns.t[i]) T(m_storage.t[i]);
      }
      for (size_t i = 0; i < m_used; i++)
      {
        m_storage.t[i].~T();
      }
      delete[] m_storage.raw;

      m_storage = ns;
      m_allocated = nsize;
    }
  };
}
