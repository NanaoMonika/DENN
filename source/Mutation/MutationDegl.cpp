#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class DEGL : public Mutation
	{
	public:

		DEGL(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//... page 6 
			//https://pdfs.semanticscholar.org/5523/8adbd3d78dc83cf906240727be02f6560470.pdf
			//alias
			const auto& f = i_final.m_f;
			Scalar scalar_weight= *m_algorithm.parameters().m_degl_scalar_weight;
			size_t neighborhood = *m_algorithm.parameters().m_degl_neighborhood;
			//target
			const Individual& i_target = *population[id_target];
			//best
			//best
			size_t id_g_best;
			Scalar eval_g_best;
			population.best(id_g_best,eval_g_best);
			const Individual& g_best = *population[id_g_best];
			//local best
			long nn                     =  (long)neighborhood;
			long np                     =  (long)population.size();
			long id_l_best				=  (long)id_target;
			for(long k=-nn; k!=(nn+1); ++k)
			{
				long i = Denn::positive_mod(k + (long)id_target, np);
				if(loss_function_compare(population[i]->m_eval, population[id_l_best]->m_eval)) 
					id_l_best = i;
			}
			//local best ref
			const Individual& l_best = *population[id_l_best];	
			//get generator
			auto& rand_deck				 = random(id_target).deck();
			auto& rand_deck_ring_segment = random(id_target).deck_ring_segment();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck_ring_segment.reinit(population.size(), id_target, neighborhood);
			rand_deck.reset();
			rand_deck_ring_segment.reset();
			//for each layers
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and baias
				for (size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//do rand
					rand_deck.reset();
					rand_deck_ring_segment.reset();
					//do cross + mutation
					const Individual& nn_g_a = *population[rand_deck.get_random_id(id_target)]; //a != target
					const Individual& nn_g_b = *population[rand_deck.get_random_id(id_target)]; //b != target 

					const Individual& nn_l_a = *population[rand_deck_ring_segment.get_random_id()];//local a != target
					const Individual& nn_l_b = *population[rand_deck_ring_segment.get_random_id()];//local b != target
					//									
					auto& w_target = i_target[i_layer][m];
					auto& w_g_best = g_best[i_layer][m];
					auto& w_l_best = l_best[i_layer][m];
					auto  w_final   = i_final[i_layer][m];

					auto& x_g_a = nn_g_a[i_layer][m];
					auto& x_g_b = nn_g_b[i_layer][m];
					auto& x_l_a = nn_l_a[i_layer][m];
					auto& x_l_b = nn_l_b[i_layer][m];

					//global
					Matrix g_m = ( w_target + ((w_g_best - w_target) + (x_g_a - x_g_b)) * f );

					//local
					Matrix l_m = ( w_target + ((w_l_best - w_target) + (x_l_a - x_l_b)) * f );

					//final (lerp)
					//from the DEGL's peper
					//lambda = 1 -> g_m (aka rand-to-best/1)
					//lambda = 0 -> l_m
					Scalar* w_final_array = w_final.data();
					Scalar* w_g_m_array   = g_m.data();
					Scalar* w_l_m_array   = l_m.data();
					for(Matrix::Index i=0; i != w_final.size(); ++i)
						w_final_array[i] = Denn::lerp(w_l_m_array[i], w_g_m_array[i], scalar_weight);

					//clamp
					w_final = w_final.unaryExpr(m_algorithm.clamp_function());
				}
			}
		}
	};
	REGISTERED_MUTATION(DEGL, "degl")

	class ProDEGL : public Mutation
	{
	public:

		ProDEGL(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//... page 6 
			//https://pdfs.semanticscholar.org/5523/8adbd3d78dc83cf906240727be02f6560470.pdf
			//alias
			const auto& f = i_final.m_f;
			Scalar scalar_weight= *m_algorithm.parameters().m_degl_scalar_weight;
			size_t neighborhood = *m_algorithm.parameters().m_degl_neighborhood;
			//target
			const Individual& i_target = *population[id_target];
			//best
			//best
			size_t id_g_best;
			Scalar eval_g_best;
			population.best(id_g_best,eval_g_best);
			const Individual& g_best = *population[id_g_best];
			//local best
			long nn                     =  (long)neighborhood;
			long np                     =  (long)population.size();
			long id_l_best				=  (long)id_target;
			for(long k=-nn; k!=(nn+1); ++k)
			{
				long i = Denn::positive_mod(k + (long)id_target, np);
				if(loss_function_compare(population[i]->m_eval, population[id_l_best]->m_eval)) 
					id_l_best = i;
			}
			//local best ref
			const Individual& l_best = *population[id_l_best];	
			//get generator
			auto& rand_deck_ring_segment = random(id_target).deck_ring_segment();
			//set population size in deck
			rand_deck_ring_segment.reinit(population.size(), id_target, neighborhood);
			rand_deck_ring_segment.reset();
			//for each layers
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and baias
				for (size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//search near values
					Scalar v_min[2] { std::numeric_limits<Scalar>::max(),  std::numeric_limits<Scalar>::max() };
					size_t id_min[2]{ 0, 0 };
					for(size_t n = 0; n!= current_np(); ++n)
					{
						//jump target
						if(n==id_target) continue;
						//ref to near
						const Individual& i_near = *population[n];
						//search
						auto dist = distance(i_target[i_layer][m],i_near[i_layer][m]);
						// dist < min 0
						if(dist < v_min[0]) 
						{ 
							//move first to second
							v_min[1]  = v_min[0];
							id_min[1] = id_min[0];
							//save new 
							v_min[0]  = dist; 
							id_min[0] = n; 
						}
						// min 0 < dist < min 1
						else if(dist < v_min[1]) 
						{ 
							v_min[1] = dist; 
							id_min[1] = n; 
						}
					}
					//do rand
					rand_deck_ring_segment.reset();
					//do cross + mutation
					const Individual& nn_g_a = *population[id_min[0]]; //a != target
					const Individual& nn_g_b = *population[id_min[1]]; //b != target 

					const Individual& nn_l_a = *population[rand_deck_ring_segment.get_random_id()];//local a != target
					const Individual& nn_l_b = *population[rand_deck_ring_segment.get_random_id()];//local b != target
					//									
					auto& w_target = i_target[i_layer][m];
					auto& w_g_best = g_best[i_layer][m];
					auto& w_l_best = l_best[i_layer][m];
					auto  w_final  = i_final[i_layer][m];

					auto& x_g_a = nn_g_a[i_layer][m];
					auto& x_g_b = nn_g_b[i_layer][m];					
					auto& x_l_a = nn_l_a[i_layer][m];
					auto& x_l_b = nn_l_b[i_layer][m];

					//global
					Matrix g_m = ( w_target + ((w_g_best - w_target) + (x_g_a - x_g_b)) * f );

					//local
					Matrix l_m = ( w_target + ((w_l_best - w_target) + (x_l_a - x_l_b)) * f );

					//final (lerp)
					//from the DEGL's peper
					//lambda = 1 -> g_m (aka rand-to-best/1)
					//lambda = 0 -> l_m
					Scalar* w_final_array = w_final.data();
					Scalar* w_g_m_array   = g_m.data();
					Scalar* w_l_m_array   = l_m.data();
					for(Matrix::Index i=0; i != w_final.size(); ++i)
						w_final_array[i] = Denn::lerp(w_l_m_array[i], w_g_m_array[i], scalar_weight);

					//clamp
					w_final = w_final.unaryExpr(m_algorithm.clamp_function());
				}
			}
		}
	};
	REGISTERED_MUTATION(ProDEGL, "pro_degl")

	class DEGLBackPropagation : public Mutation
	{
	public:

		DEGLBackPropagation(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//... page 6 
			//https://pdfs.semanticscholar.org/5523/8adbd3d78dc83cf906240727be02f6560470.pdf
			//alias
			const auto& f = i_final.m_f;
			Scalar scalar_weight= *m_algorithm.parameters().m_degl_scalar_weight;
			size_t neighborhood = *m_algorithm.parameters().m_degl_neighborhood;
			//target
			const Individual& i_target = *population[id_target];
			//best
			//best
			size_t id_g_best;
			Scalar eval_g_best;
			population.best(id_g_best,eval_g_best);
			const Individual& g_best = *population[id_g_best];
			//local best
			long nn                     =  (long)neighborhood;
			long np                     =  (long)population.size();
			long id_l_best				=  (long)id_target;
			for(long k=-nn; k!=(nn+1); ++k)
			{
				long i = Denn::positive_mod(k + (long)id_target, np);
				if( population[i]->m_eval <  population[id_l_best]->m_eval) id_l_best = i;
			}
			//local best ref
			const Individual& l_best = *population[id_l_best];	
			//get generator
			auto& rand_deck				 = random(id_target).deck();
			auto& rand_deck_ring_segment = random(id_target).deck_ring_segment();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck_ring_segment.reinit(population.size(), id_target, neighborhood);
			rand_deck.reset();
			rand_deck_ring_segment.reset();
			//local / global network
			NeuralNetwork nn_l(i_final.m_network);
			NeuralNetwork nn_g(i_final.m_network);
			//for each layers
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and baias
				for (size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//do rand
					rand_deck.reset();
					rand_deck_ring_segment.reset();
					//do cross + mutation
					const Individual& nn_g_a = *population[rand_deck.get_random_id(id_target)]; //a != target
					const Individual& nn_g_b = *population[rand_deck.get_random_id(id_target)]; //b != target 

					const Individual& nn_l_a = *population[rand_deck_ring_segment.get_random_id()];//local a != target
					const Individual& nn_l_b = *population[rand_deck_ring_segment.get_random_id()];//local b != target
					// target + best								
					auto& w_target = i_target[i_layer][m];
					auto& w_g_best = g_best[i_layer][m];
					auto& w_l_best = l_best[i_layer][m];
					// others
					auto& x_g_a = nn_g_a[i_layer][m];
					auto& x_g_b = nn_g_b[i_layer][m];					
					auto& x_l_a = nn_l_a[i_layer][m];
					auto& x_l_b = nn_l_b[i_layer][m];

					//global
					nn_g[i_layer][m] = ( w_target + ((w_g_best - w_target) + (x_g_a - x_g_b)) * f );
					//local
					nn_l[i_layer][m] = ( w_target + ((w_l_best - w_target) + (x_l_a - x_l_b)) * f );
				}
			}
			//backpropagation
			for(short nbp=0; nbp!=2; ++nbp)
			{
				nn_g.fit
				(	
					  m_algorithm.current_batch().features()
					, m_algorithm.current_batch().labels()
					, SGD(*parameters().m_learning_rate
						 ,*parameters().m_decay
						 ,*parameters().m_momentum
						 ,*parameters().m_nesterov)
				);
				nn_l.fit
				(	
					  m_algorithm.current_batch().features()
					, m_algorithm.current_batch().labels()
					, SGD(*parameters().m_learning_rate
						 ,*parameters().m_decay
						 ,*parameters().m_momentum
						 ,*parameters().m_nesterov)
				);
			}
			//lerp
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			for (size_t m = 0; m != i_target[i_layer].size(); ++m)
			{
				auto    w_final		  = i_final[i_layer][m];
				Scalar* w_final_array = w_final.data();
				Scalar* w_g_m_array   = nn_g[i_layer][m].data();
				Scalar* w_l_m_array   = nn_l[i_layer][m].data();
				for(Matrix::Index i=0; i != w_final.size(); ++i)
					w_final_array[i] = Denn::lerp(w_l_m_array[i], w_g_m_array[i], scalar_weight);
				//clamp
				w_final = w_final.unaryExpr(m_algorithm.clamp_function());
			}
		}
	};
	REGISTERED_MUTATION(DEGLBackPropagation, "deglbp")
}