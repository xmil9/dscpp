#pragma once
#include <iterator>
#include <type_traits>


namespace ds
{

///////////////////

template <typename T, typename = void> inline constexpr bool IsIterator_v = false;

// Will only pass SFINAE, if the given type defines iterator_category.
template <typename T>
inline constexpr bool
   IsIterator_v<T, std::void_t<typename std::iterator_traits<T>::iterator_category>> =
      true;

template <typename T> struct IsIterator : std::bool_constant<IsIterator_v<T>>
{
};

} // namespace ds
