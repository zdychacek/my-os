#pragma once

#include <debug/Assert.h>
#include <interfaces/NonCopyable.h>

namespace std
{
  template <typename T>
  struct DefaultDeleter
  {
    void operator()(T *p)
    {
      if (p)
      {
        delete p;
        p = nullptr;
      }
    }
  };

  template <typename T, typename Deleter = DefaultDeleter<T>>
  class unique_ptr : private Interfaces::NonCopyable
  {
  public:
    //construct
    unique_ptr(T *pT = nullptr);
    //destroy
    ~unique_ptr();

  public:
    //reset
    void Reset(T *p);
    //release the own of the pointer
    T *Release();
    //get the pointer
    T *Get();

  public:
    //convert unique_ptr to bool
    operator bool() const;
    //overload for operator *
    T &operator*();
    //overload for operator ->
    T *operator->();

  private:
    //pointer
    T *m_pointer;
    //deleter
    Deleter m_deleter;
    //call deleter
    void Delete();
  };

  //construct
  template <typename T, typename Deleter>
  unique_ptr<T, Deleter>::unique_ptr(T *pointer) : m_pointer(pointer)
  {
  }

  //destroy
  template <typename T, typename Deleter>
  unique_ptr<T, Deleter>::~unique_ptr()
  {
    Delete();
  }

  //call deleter
  template <typename T, typename Deleter>
  void unique_ptr<T, Deleter>::Delete()
  {
    if (*this)
    {
      m_deleter(m_pointer);
      m_pointer = nullptr;
    }
  }

  //get the pointer
  template <typename T, typename Deleter>
  T *unique_ptr<T, Deleter>::Get()
  {
    return m_pointer;
  }

  //reset
  template <typename T, typename Deleter>
  void unique_ptr<T, Deleter>::Reset(T *p)
  {
    Delete();
    m_pointer = p;
  }

  //release the own of the pointer
  template <typename T, typename Deleter>
  T *unique_ptr<T, Deleter>::Release()
  {
    T *p = m_pointer;
    m_pointer = nullptr;
    return p;
  }

  //convert unique_ptr to bool
  template <typename T, typename Deleter>
  unique_ptr<T, Deleter>::operator bool() const
  {
    return m_pointer != nullptr;
  }

  //overload for operator *
  template <typename T, typename Deleter>
  T &unique_ptr<T, Deleter>::operator*()
  {
    Assert(*this, "*this must be no-null pointer");
    return *m_pointer;
  }

  //overload for operator ->
  template <typename T, typename Deleter>
  T *unique_ptr<T, Deleter>::operator->()
  {
    return &*(*this);
  }
}
