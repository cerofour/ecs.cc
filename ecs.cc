#include <iostream>
#include <vector>
#include <stack>
#include <tuple>
#include <numeric>
#include <cassert>
#include <chrono>
#include <thread>

#include "utils/types.hpp"
#include "ecs.hpp"

template<typename T>
void prettyprint_vector(const std::vector<T>& v) {
	std::cout << "[";
	for (const auto& x : v) {
		std::cout << x;
		std::cout << ", ";
	}
	std::cout << "]";
}

struct PositionComponent {
	i64 x;
	i64 y;
};

struct VelocityComponent {
	i64 x;
	i64 y;
};

struct SpriteComponent {
	i64 handle;
};
	
void draw(const SpriteComponent& spr, const PositionComponent& pos) {
	std::cout << "drawing " << spr.handle << "at " << pos.x << ", " << pos.y << std::endl;
}

int main() {

	ecs::System<PositionComponent, VelocityComponent, SpriteComponent> mysys;

	for (u64 i = 0; i < 10; i++) {
		auto e = mysys.spawn_entity();
		if (std::rand() % 2 == 0) {
			mysys.enable_components<PositionComponent, VelocityComponent, SpriteComponent>(e);
			auto& pos = mysys.component<PositionComponent>(i);
			pos.x = std::rand() % 640;
			pos.y = std::rand() % 480;
			auto& vel = mysys.component<VelocityComponent>(i);
			vel.x = 3;
			vel.y = 3;
		}
	}

	mysys.set_update_hooks({
			[&mysys]() { // movement
				auto qr {mysys.query<PositionComponent, VelocityComponent>()};
				for (const auto& e : qr) {
					auto& pos {mysys.component<PositionComponent>(e)};
					auto& vel {mysys.component<VelocityComponent>(e)};
					pos.x += vel.x;
					pos.y += vel.y;
				}
			},
			[&mysys]() {
				auto qr {mysys.query<PositionComponent, SpriteComponent>()};
				for (const auto& e : qr) {
					auto& spr {mysys.component<SpriteComponent>(e)};
					auto& pos {mysys.component<PositionComponent>(e)};
					draw(spr, pos);
				}
			}
			});

	auto qr = mysys.query<PositionComponent, VelocityComponent>();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		/* auto pos = mysys.get_component<PositionComponent>(0); */
		/* 	std::cout << "Position of entity 0: "; */
		/* 	std::cout << "[" << pos.x << ", " << pos.y << "]\n"; */

		for (const auto& i : qr) {
			auto& pos = mysys.component<PositionComponent>(i);
			std::cout << "Position of entity: " << i << ": ";
			std::cout << "[" << pos.x << ", " << pos.y << "]\n";
		}

		mysys.update();
	}

	return 0;
}
