#ifndef UNIQUE_VIEW_H
#define UNIQUE_VIEW_H

#include <numeric>
#include <vector>

template <typename C>
class UniqueView
{
private:
  using ValueType     = typename C::value_type;
  using ConstIterator = typename C::const_iterator;
  struct Slice
  {
    ConstIterator begin;
    ConstIterator end;
  };
  std::vector<Slice> m_slices;
  using ConstSliceIterator = typename std::vector<Slice>::const_iterator;

  // Construction
public:
  explicit UniqueView(const C &data)
  {
    if (data.empty())
    {
      return; // trivial
    }

    // First element slice
    Slice slice{data.begin(), ++data.begin()};
    bool inUniqueRange = true;

    ConstIterator first     = data.begin();
    const ConstIterator end = data.end();
    for (auto it = ++data.begin(); it != end; )
    {
      if (*it != *first)
      {
        first = it;
        if (!inUniqueRange) [[unlikely]]
        {
          slice.begin   = it;
          inUniqueRange = true;
        }
        slice.end = ++it; // increase subrange
      }
      else
      {
        if (inUniqueRange) [[unlikely]]
        {
          m_slices.push_back(slice);
          slice.begin   = it;
          slice.end     = ++it;
          inUniqueRange = false;
        }
        else
        {
          ++it; // skip
        }
      }
    }

    if (inUniqueRange)
    {
      slice.end = end; // tail
      m_slices.push_back(slice);
    }
  }

  // Getters
public:
  bool empty() const { return m_slices.empty(); }
  std::size_t size() const
  {
    return std::accumulate(m_slices.begin(),
                           m_slices.end(),
                           0,
                           [](const std::size_t acc, const Slice &slice) {
                             return acc + std::distance(slice.begin, slice.end);
                           });
  }

  // Iterator support
public:
  struct Iterator
  {
    ConstSliceIterator sliceIt;
    ConstSliceIterator sliceEnd;
    ConstIterator innerIt;
    ConstIterator innerEnd;

    void operator++()
    {
      innerIt++;
      if (innerIt != innerEnd)
      {
        return;
      }

      sliceIt++;
      if (sliceIt != sliceEnd)
      {
        innerIt  = sliceIt->begin;
        innerEnd = sliceIt->end;
      }
    }

    const ValueType &operator*() const { return *innerIt; }

    friend bool operator==(const Iterator &a, const Iterator &b) = default;
  };

  Iterator begin() const
  {
    if (m_slices.empty())
    {
      return {m_slices.begin(), m_slices.end(), {}, {}};
    }

    return {m_slices.begin(), m_slices.end(), m_slices.front().begin, m_slices.front().end};
  }
  Iterator end() const
  {
    if (m_slices.empty())
    {
      return {m_slices.end(), m_slices.end(), {}, {}};
    }
    return {m_slices.end(), m_slices.end(), m_slices.back().end, m_slices.back().end};
  }
};

// CTAD
template <typename C>
UniqueView(const C &container) -> UniqueView<C>;

#endif // UNIQUE_VIEW_H
