#pragma once

#include <lib/concepts.h>

namespace std
{
  template <class ForwardIt, class T>
  ForwardIt remove(ForwardIt first, ForwardIt last, const T &value) requires Iterable<ForwardIt>
  {
    first = find(first, last, value);

    if (first != last)
    {
      for (ForwardIt i = first; ++i != last;)
      {
        if (!(*i == value))
        {
          *first++ = move(*i);
        }
      }
    }

    return first;
  }

  template <class ForwardIt, class UnaryPredicate>
  ForwardIt remove_if(ForwardIt first, ForwardIt last, UnaryPredicate p)
  {
    first = find_if(first, last, p);

    if (first != last)
    {
      for (ForwardIt i = first; ++i != last;)
      {
        if (!p(*i))
        {
          *first++ = move(*i);
        }
      }
    }

    return first;
  }

  template <class InputIt, class T>
  constexpr InputIt find(InputIt first, InputIt last, const T &value)
  {
    for (; first != last; ++first)
    {
      if (*first == value)
      {
        return first;
      }
    }

    return last;
  }

  template <class InputIt, class UnaryPredicate>
  constexpr InputIt find_if(InputIt first, InputIt last, UnaryPredicate p)
  {
    for (; first != last; ++first)
    {
      if (p(*first))
      {
        return first;
      }
    }

    return last;
  }

  template <class InputIt, class UnaryPredicate>
  constexpr InputIt find_if_not(InputIt first, InputIt last, UnaryPredicate q)
  {
    for (; first != last; ++first)
    {
      if (!q(*first))
      {
        return first;
      }
    }

    return last;
  }

  /// copy
  template <class InputIt, class OutputIt>
  OutputIt copy(InputIt first, InputIt last, OutputIt d_first)
  {
    while (first != last)
    {
      *d_first++ = *first++;
    }

    return d_first;
  }

  template <class InputIt, class OutputIt, class UnaryPredicate>
  OutputIt copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate pred)
  {
    while (first != last)
    {
      if (pred(*first))
      {
        *d_first++ = *first;
      }

      first++;
    }

    return d_first;
  }

  template <class InputIt, class Size, class OutputIt>
  OutputIt copy_n(InputIt first, Size count, OutputIt result)
  {
    if (count > 0)
    {
      *result++ = *first;

      for (Size i = 1; i < count; ++i)
      {
        *result++ = *++first;
      }
    }

    return result;
  }

  /// equal
  template <class InputIt1, class InputIt2>
  bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2)
  {
    for (; first1 != last1; ++first1, ++first2)
    {
      if (!(*first1 == *first2))
      {
        return false;
      }
    }

    return true;
  }

  template <class InputIt1, class InputIt2, class BinaryPredicate>
  bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2, BinaryPredicate p)
  {
    for (; first1 != last1; ++first1, ++first2)
    {
      if (!p(*first1, *first2))
      {
        return false;
      }
    }

    return true;
  }

  template <class InputIt, class UnaryFunction>
  constexpr UnaryFunction for_each(InputIt first, InputIt last, UnaryFunction f)
  {
    for (; first != last; ++first)
    {
      f(*first);
    }

    return f; // implicit move since C++11
  }
}
