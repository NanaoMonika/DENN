#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class CurrToRandOne : public Mutation
	{
	public:

		CurrToRandOne(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//alias
			const auto& f = i_final.m_f;
			//target
			const Individual& i_target = *population[id_target];
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck.reset();
			//for each layers
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and baias
				for (size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//do rand
					rand_deck.reset();
					//do cross + mutation
					const Individual& nn_a = *population[rand_deck.get_random_id(id_target)];
					const Individual& nn_b = *population[rand_deck.get_random_id(id_target)];
					const Individual& nn_c = *population[rand_deck.get_random_id(id_target)];
					//
					auto  w_final = i_final[i_layer][m];
					auto& w_target = i_target[i_layer][m];
					auto& x_a = nn_a[i_layer][m];
					auto& x_b = nn_b[i_layer][m];
					auto& x_c = nn_c[i_layer][m];
					w_final = (w_target + ((x_a - w_target) + (x_b - x_c)) * f).unaryExpr(m_algorithm.clamp_function());
				}
			}
		}
	};
	REGISTERED_MUTATION(CurrToRandOne, "curr_to_rand/1")
}