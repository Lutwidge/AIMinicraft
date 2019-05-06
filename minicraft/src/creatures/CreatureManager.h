#pragma once

#include <unordered_map>
#include <vector>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include "../SimpleList.h"

class AICreature;

class CreatureManager {
public:

	std::unordered_map<std::type_index, SimpleList<AICreature*>*> creatures;

	CreatureManager() {
		
	}

	~CreatureManager() {
		// TODO cleanup creatures map
	}

	void registerCreature(AICreature* creature) {
		SimpleList<AICreature*>* typeVec;
		auto search = creatures.find(std::type_index(typeid(creature)));
		if (search != creatures.end()) {
			typeVec = search->second;
		} else {
			typeVec = new SimpleList<AICreature*>(16, 16);
		}
		typeVec->add(creature);
	}

	void unregisterCreature(AICreature* creature) {
		SimpleList<AICreature*>* typeVec;
		auto search = creatures.find(std::type_index(typeid(creature)));
		if (search != creatures.end()) {
			typeVec = search->second;
			for (int i = 0; i < typeVec->count; i++) {
				if (typeVec->arr[i] == creature) {
					typeVec->remove(i);
					return;
				}
			}
		}
	}

};
