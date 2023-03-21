//
// Mar-2023, Michael Lindner
// MIT license
//
#include <random>


namespace ds
{
///////////////////

// Generates random floating point numbers in range [a, b).
template <typename Fp = double> class Random
{
 public:
   // Values in range (0, 1] with random seed.
   Random();
   // Values in range (0, 1] with given seed.
   Random(unsigned int seed);
   // Values in range (a, b] with random seed.
   Random(Fp a, Fp b);
   // Values in range (a, b] with given seed.
   Random(Fp a, Fp b, unsigned int seed);
   Random(const Random&) = delete;
   Random(Random&&) = default;

   Random& operator=(const Random&) = delete;
   Random& operator=(Random&&) = default;

   Fp next();

 private:
   std::mt19937 m_gen;
   std::uniform_real_distribution<Fp> m_dist;
};


template <typename Fp> Random<Fp>::Random() : Random{std::random_device{}()}
{
}


template <typename Fp>
Random<Fp>::Random(unsigned int seed)
: Random{static_cast<Fp>(0), static_cast<Fp>(1), seed}
{
}


template <typename Fp>
Random<Fp>::Random(Fp a, Fp b) : Random{a, b, std::random_device{}()}
{
}


template <typename Fp>
Random<Fp>::Random(Fp a, Fp b, unsigned int seed) : m_gen{seed}, m_dist{a, b}
{
}


template <typename Fp> Fp Random<Fp>::next()
{
   return m_dist(m_gen);
}

} // namespace ds
