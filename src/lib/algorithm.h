#pragma once

namespace std
{
  template <class ForwardIt, class T>
  ForwardIt remove(ForwardIt first, ForwardIt last, const T &value)
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
}
