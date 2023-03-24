//
// Mar-2023, Michael Lindner
// MIT license
//
#include <random>


namespace ds
{
///////////////////

// Generates random floating point numbers in range [a, b).
template <typename T = double> class Random
{
 public:
   // Values in range (0, 1] with random seed.
   Random() noexcept;
   // Values in range (0, 1] with given seed.
   Random(unsigned int seed);
   // Values in range (a, b] with random seed.
   Random(T a, T b);
   // Values in range (a, b] with given seed.
   Random(T a, T b, unsigned int seed);

   Random(const Random&) = delete;
   Random(Random&&) = default;
   Random& operator=(const Random&) = delete;
   Random& operator=(Random&&) = default;

   T next();

 private:
   std::mt19937 m_gen;
   std::uniform_real_distribution<T> m_dist;
};


template <typename T> Random<T>::Random() noexcept : Random{std::random_device{}()}
{
}

template <typename T>
Random<T>::Random(unsigned int seed) : Random{static_cast<T>(0), static_cast<T>(1), seed}
{
}

template <typename T> Random<T>::Random(T a, T b) : Random{a, b, std::random_device{}()}
{
}

template <typename T>
Random<T>::Random(T a, T b, unsigned int seed) : m_gen{seed}, m_dist{a, b}
{
}

template <typename T> T Random<T>::next()
{
   return m_dist(m_gen);
}

///////////////////

// Generates random integer numbers in range [a, b].
template <typename Int = int> class RandomInt
{
 public:
   // Values in range [a, b] with random seed.
   RandomInt(Int a, Int b);
   // Values in range [a, b] with given seed.
   RandomInt(Int a, Int b, unsigned int seed);

   RandomInt(const RandomInt&) = delete;
   RandomInt(RandomInt&&) = default;
   RandomInt& operator=(const RandomInt&) = delete;
   RandomInt& operator=(RandomInt&&) = default;

   Int next();

 private:
   Random<double> m_rand;
};


template <typename Int>
RandomInt<Int>::RandomInt(Int a, Int b)
: m_rand{
     // Make sure negative start values get truncated to include the start of the range.
     a < 0 ? a - 0.9999999 : a,
     // Make sure negative end values get truncated to include the start of the range.
     b < 0 ? b : b + 1}
{
}

template <typename Int>
RandomInt<Int>::RandomInt(Int a, Int b, unsigned int seed) : m_rand{a, b + 1, seed}
{
}

template <typename Int> Int RandomInt<Int>::next()
{
   return static_cast<Int>(m_rand.next());
}

///////////////////

// Generate random permutation of sequence.
// Cormen, pg 126
// Time: O(n)
template <typename Iter> void permute(Iter first, Iter last)
{
   for (Iter it = first; it != last; ++it)
   {
      const size_t n = std::distance(it, last);
      // The random offset could be zero because the element staying in place is a
      // valid permutation.
      const size_t pos = RandomInt(0, n - 1);
      if (pos != 0)
         std::swap(*it, *(it + pos));
   }
}

} // namespace ds
