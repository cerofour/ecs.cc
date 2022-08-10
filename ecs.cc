#include <iostream>
#include <vector>
#include <stack>
#include <tuple>
#include <numeric>
#include <cassert>

#include "utils/result.hpp"
#include "utils/types.hpp"
#include "utils/metaprog.hpp"
#include "utils/numeric.hpp"

u64 setflag(u64 flag, u64 x = 0) {
	return x | (1 << flag);
}

void turn_bit(u64& x, u64 bitno) {
	x |= (1 << bitno);
}

bool isbiton(u64 x, u64 bitno) {
	return x & (1 << bitno);
}

namespace ecs {

#define INTERNAL_FLAG_ALIVE 0

	struct Entity {
		// flags = 0 means the entity is dead and the space allocated for it
		// will be reused.
		u64 flags;
		u64 masks;
	};
	static_assert(std::is_trivial<Entity>(), "ecs::Entity must be a trivial type");

	struct EntityStore {

		/**
		 * The way of using @handle_type will probably change in the future,
		 * because, I don't know why, but I see something bad about it right now.
		 * Tbh, it works, for now.
		 */
		using handle_type = u64;
		std::vector<Entity> entities;

		// index to non used entries in this->entities
		std::stack<handle_type> entity_pool;

		handle_type spawn() {

			if (entity_pool.size() > 0) {
				handle_type h = entity_pool.top();
				entities[h].flags |= INTERNAL_FLAG_ALIVE;
				entity_pool.pop();
				return h;
			}

			handle_type handle = entities.size();
			Entity e {setflag(INTERNAL_FLAG_ALIVE), 0};
			entities.push_back(e);

			return handle;
		}

		void kill(handle_type handle) {
			entities[handle].flags = 0;
			entity_pool.push(handle);
		}
	};

	/**
	 * A ComponentStore holds a container of ComponentSets, where a
	 * ComponentSet is a set of all the Components an entity can have.
	 * Components are implemented as C++ types.
	 */
	template<
		typename ...Rest>
	struct ComponentStore {

		static_assert(utils::metaprog::only_unique_types<Rest...>(),
				"Components must be unique");

		// there can't be any more components than u64 can handle
		static_assert(sizeof...(Rest) <= 64);

		using ComponentSet = std::tuple<Rest...>;
		static const size_t type_count = sizeof...(Rest);

		std::vector<ComponentSet> comps;

		// Get a reference to the Nth component of an entity
		template<int N>
		auto& get() {
			return std::get<N>(this->comps);
		}
	};

	template<
		typename ...Cs>
	class System {
	public:
		using handle_type = EntityStore::handle_type;

	public:
		System() = default;

	/// Public ECS related methods
	public:
		handle_type spawn_entity() {
			auto h = this->es.spawn();

			// create space in the ComponentStore for a new entity if needed.
			if (h >= cs.comps.capacity())
				cs.comps.reserve(h);
			
			return h;
		}

		template<
			typename First,
			typename ...Rest>
		void enable_components(handle_type handle) {
			this->enable_component<First>(handle);
			if constexpr (sizeof...(Rest) == 0)
				return;
			else
				this->enable_components<Rest...>(handle);
		}

		// return a vector of entity handles of all entities that have the Ts
		// components enabled.
		template<typename ...Ts>
		const std::vector<handle_type> query() {
			auto mask = this->get_components_mask<Ts...>();
			std::vector<handle_type> query_result;

			for (handle_type i = 0; i < this->es.entities.size(); i++) {
				auto& e = this->es.entities[i];
				if (isbiton(e.flags, INTERNAL_FLAG_ALIVE) && (e.masks & mask) == mask) {
					query_result.push_back(i);
				}
			}

			return query_result;
		}

	// Private ECS related methods: helpers / internal definitions.
	private:
		template<typename T>
		void enable_component(handle_type handle) {
			turn_bit(this->es.entities[handle].masks, utils::metaprog::index<T, Cs...>());
		}

		template<
			typename ...Ts>
		u64 get_components_mask(){ 
			u64 mask = 0;
			this->_get_components_mask<Ts...>(mask);
			return mask;
		}

		template<
			typename First,
			typename ...Rest>
		void _get_components_mask(u64& history){ 
			turn_bit(history, utils::metaprog::index<First, Cs...>());
			if constexpr (sizeof...(Rest) == 0)
				return;
			else
				this->_get_components_mask<Rest...>(history);
		}

		/*
		 * UNUSED
		template<
			typename First,
			typename ...Ts>
		std::vector<handle_type> component_indexes() {
			std::vector<handle_type> is;
			this->component_indexes_internal<First, Ts...>(is);
			return is;
		}

		template<
			typename First,
			typename ...Ts>
		void component_indexes_internal(std::vector<handle_type>& history) {
			history.push_back(utils::metaprog::index<First, Cs...>());
			if constexpr (sizeof...(Ts) == 0)
				return;
			else
				this->component_indexes_internal<Ts...>(history);
		}
		*/

	private:
		EntityStore es;
		ComponentStore<Cs...> cs;
		
	private:
	};
};

template<typename T>
void prettyprint_vector(const std::vector<T>& v) {
	std::cout << "[";
	for (const auto& x : v) {
		std::cout << x;
		std::cout << ", ";
	}
	std::cout << "]";
}

int main() {

	struct PositionComponent {
		PODPair<i64> pos;
	};

	struct VelocityComponent {
		PODPair<i64> vel;
	};

	struct SpriteComponent {
		i64 sprite_handle;
	};
	
	ecs::System<PositionComponent, VelocityComponent, SpriteComponent> mysys;

	std::vector<u64> with_components;

	for (size_t i = 0; i < 100; i++) {
		mysys.spawn_entity();
		if (std::rand() % 2 == 0) {
			with_components.push_back(i);
			mysys.enable_components<PositionComponent, VelocityComponent, SpriteComponent>(i);
		}
	}

	auto qr = mysys.query<PositionComponent, VelocityComponent, SpriteComponent>();

	assert(qr == with_components);

	return 0;
}
