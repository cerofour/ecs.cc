#pragma once

#include <stack>

#include "utils/result.hpp"
#include "utils/types.hpp"
#include "utils/metaprog.hpp"
#include "utils/numeric.hpp"
#include "utils/bits.hpp"

namespace ecs {

#define INTERNAL_FLAG_ALIVE 0

	struct Entity {
		// flags = 0 means the entity is dead and the space allocated for it
		// will be reused.
		u64 flags;
		u64 masks;

		void setflag(u64 flag) {
			utils::bits::setbit(flag, this->flags);
		}

		bool isflag(u64 flag) {
			return utils::bits::isbiton(flag, this->flags);
		}

		bool checkmask(u64 mask) {
			return utils::bits::checkmask(this->masks, mask);
		}

		void setmask(u64 mask) {
			utils::bits::setbit(mask, this->masks);
		}
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
			Entity e {0, 0};
			e.setflag(INTERNAL_FLAG_ALIVE);
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

			if (this->cs.comps.size() == h) {
				this->cs.comps.push_back({});
			}

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
				if (e.isflag(INTERNAL_FLAG_ALIVE) && e.checkmask(mask)) {
					query_result.push_back(i);
				}
			}

			return query_result;
		}

		// TODO: implement me
		void update() {
			for (const auto& f : this->update_hooks) {
				f();
			}
		}

	public:
		void set_update_hooks(std::vector<std::function<void(void)>>&& hooks) {
			this->update_hooks = std::move(hooks);
		}

		// Get a reference to the Nth component of an entity
		template<typename C>
		C& component(handle_type h) {
			return std::get<C>(this->cs.comps[h]);
		}

	// Private ECS related methods: helpers / internal definitions.
	private:
		template<typename T>
		void enable_component(handle_type handle) {
			this->es.entities[handle].setmask(utils::metaprog::index<T, Cs...>());
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
			history |= (1 << utils::metaprog::index<First, Cs...>());
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
		std::vector<std::function<void(void)>> update_hooks;
	};
};

