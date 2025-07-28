#pragma once

#include "../ecs/entity.h"

struct TriggerEnterEvent {
	Entity trigger;
	Entity entity;

	TriggerEnterEvent(Entity t, Entity e) : trigger(t), entity(e) {};
};

struct TriggerStayEvent {
	Entity trigger;
	Entity entity;

	TriggerStayEvent(Entity t, Entity e) : trigger(t), entity(e) {};
};

struct TriggerExitEvent {
	Entity trigger;
	Entity entity;

	TriggerExitEvent(Entity t, Entity e) : trigger(t), entity(e) {};
};