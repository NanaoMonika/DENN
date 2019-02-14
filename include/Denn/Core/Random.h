#pragma once
#include <random>

namespace Denn
{
	class Random
	{
		//random deck
		class RandomDeck
		{
			size_t 					     m_size{ 0 };
			size_t					     m_k   { 0 };
			std::unique_ptr < size_t[] > m_deck{ nullptr };
			Random&				         m_random;

		public:

			RandomDeck(Random& random) : m_random(random) {}
			
			RandomDeck(Random& random,const RandomDeck& deck);

			size_t get_random_id(size_t target);

			void reset();

			void reinit(size_t size);

		};

		//random deck
		class RandomDeckRingSegment
		{
			size_t 					     m_target       { 0 };
			size_t 					     m_neighborhood { 0 };
			size_t 					     m_global_size  { 0 };
			size_t 					     m_size         { 0 };
			size_t					     m_k            { 0 };
			std::unique_ptr < size_t[] > m_deck{ nullptr };
			Random&						 m_random;

		public:

			RandomDeckRingSegment(Random& random) : m_random(random) {}

			RandomDeckRingSegment(Random& random,const RandomDeckRingSegment& deck);
			
			size_t get_random_id();

			void reset();

			void reinit(size_t size, size_t m_target, size_t neighborhood);

		};

	public:

		Random() : Random (std::random_device{}()) {}

		Random(unsigned int value);

		Random(const Random& random) ;
		//reinit
		void reinit(unsigned int seed = std::random_device{}());

		//random integer in [0, max)
		int irand(int max = std::numeric_limits<int>::max());

		//random integer in [min, max)
		int irand(int min, int max);

		//unsigned int in [0, max)
		unsigned int uirand(unsigned int max = std::numeric_limits<unsigned int>::max());

		//unsigned int in [min, max)
		unsigned int uirand(unsigned int min, unsigned int max);

		//random integer in [0,size)
		size_t index_rand(size_t max = std::numeric_limits<size_t>::max());

		//random value generated by uniform distribution in flooting point in range [min,max]
		Scalar uniform(Scalar min = 0.0, Scalar max = 1.0);

		//random value generated by normal distribution given a mean and a standard deviation in flooting point
		Scalar normal(Scalar mean = 0.0, Scalar stddev = 1.0);

		//random value generated by cauchy[/Lorentz] distribution given a location and a scale in flooting point
		Scalar cauchy(Scalar location, Scalar scale);

		//get deck
		RandomDeck& 		   deck() 			   const { return m_deck; }
		RandomDeckRingSegment& deck_ring_segment() const { return m_deck_ring_segment; }

	protected:

		//number generator
		std::mt19937 		           m_generator;
		mutable RandomDeck   	       m_deck;
		mutable RandomDeckRingSegment  m_deck_ring_segment;

	};
}