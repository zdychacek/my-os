#pragma once

namespace std
{
  template <class I>
  concept Iterable = requires(I i)
  {
    i.begin()->T;
    i.end()->T;
  };
}
