#include "Denn/Config.h"
#include "Denn/Core/Random.h"

namespace Denn
{
	Random::Random(unsigned int seed) 
	: m_generator (seed)
	, m_deck(*this)
	, m_deck_ring_segment(*this) 
	{
	}

	Random::Random(const Random& random) 
	: m_generator        (random.m_generator)
	, m_deck             (*this,random.deck())
	, m_deck_ring_segment(*this,random.deck_ring_segment()) 
	{
		//copy
	}
	//reinit
	void Random::reinit(unsigned int seed)
	{
		m_generator = std::mt19937(seed);
	}

	//random integer in [0,size)
	int Random::irand(int max)
	{
		std::uniform_int_distribution<int> distribution(0, max-1);
		return distribution(m_generator);
	}
	int Random::irand(int min, int max)
	{
		std::uniform_int_distribution<int> distribution(min, max-1);
		return distribution(m_generator);
	}

	//random unsigned integer in [0,size)
	unsigned int Random::uirand(unsigned int max)
	{
		std::uniform_int_distribution<unsigned int> distribution(0, max-1);
		return distribution(m_generator);
	}
	unsigned int Random::uirand(unsigned int min,unsigned int max)
	{
		std::uniform_int_distribution<unsigned int> distribution(min, max-1);
		return distribution(m_generator);
	}

	//random integer in [0,size)
	size_t Random::index_rand(size_t max)
	{
		std::uniform_int_distribution<size_t> distribution(0, max-1);
		return distribution(m_generator);
	}

	//random value in flooting point [min,max]
	Scalar Random::uniform(Scalar min, Scalar max)
#if defined( USE_FAST_UNIFORM ) //vc performance issues
	{
		std::uniform_int_distribution<unsigned int> distribution(0, std::numeric_limits<unsigned int>::max());
		return Scalar(distribution(m_generator)) * (Scalar(1.0) / std::numeric_limits<unsigned int>::max()) * (max-min) + min;
	}
#else
	{
		std::uniform_real_distribution<Scalar> distribution(min, max);
		return distribution(m_generator);
	}
#endif
	//random value generated by normal distribution given a mean and a standard deviation in flooting point
	Scalar Random::normal(Scalar m, Scalar s)
	{
		std::normal_distribution<Scalar> distribution(m, s);
		return distribution(m_generator);
	}

	//random value generated by cauchy distribution given a location and a scale in flooting point
	Scalar Random::cauchy(Scalar location, Scalar scale)
	{
		std::cauchy_distribution<Scalar> distribution(location, scale);
		return distribution(m_generator);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	Random::RandomDeck::RandomDeck(Random& random, const RandomDeck& deck)
	:m_random(random)
	{
		reinit(deck.m_size);
	}

	size_t Random::RandomDeck::get_random_id(size_t target)
	{
		size_t j=m_random.index_rand(m_k);
		size_t r=m_deck[j];
		//reduce indices region
		--m_k;
		//swap with last element
		m_deck[j]  =m_deck[m_k];
		m_deck[m_k]=r;
		//all indices >= target are +1
		if( r >= target ) r++;
		//return
		return r;
	}

	void Random::RandomDeck::reset()
	{
		m_k = m_size-1;
	}

	void Random::RandomDeck::reinit(size_t size)
	{
		if(m_size!=size)
		{
			m_size = size;
			m_k    = size-1;
			m_deck = std::make_unique<size_t[]>(m_k);
			for(size_t i=0; i!=m_k; ++i) m_deck[i]=i;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////	
	Random::RandomDeckRingSegment::RandomDeckRingSegment(Random& random, const RandomDeckRingSegment& deck)
	:m_random(random)
	{
		reinit(deck.m_global_size, deck.m_target, deck.m_neighborhood);
	}

	size_t Random::RandomDeckRingSegment::get_random_id()
	{
		size_t j=m_random.index_rand(m_k);
		size_t r=m_deck[j];
		//reduce indices region
		--m_k;
		//swap with last element
		m_deck[j]  =m_deck[m_k];
		m_deck[m_k]=r;
		//all indices >= target are +1
		//target is the center of the dack
		//if r >= (m_neighborhood+1) then r++;
		if( r > m_neighborhood ) r++;
		//return
		return Denn::positive_mod(long(r) + long(m_target) - long(m_neighborhood), long(m_global_size));
	}

	void Random::RandomDeckRingSegment::reset()
	{
		m_k = m_size-1;
	}

	void Random::RandomDeckRingSegment::reinit(size_t g_size, size_t target, size_t neighborhood)
	{		
		//compute size
		size_t l_size = neighborhood*2+1;
		//save pivot
		m_global_size  = g_size;
		m_target       = target;
		m_neighborhood = neighborhood;
		//realloc
		if(m_size!=l_size)
		{
			m_size = l_size;
			m_k    = l_size-1;
			m_deck = std::make_unique<size_t[]>(m_k);
			for(size_t i=0; i!=m_k; ++i) m_deck[i]=i;
		}
	}
}