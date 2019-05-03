class Creature;

class CreatureState
{
public:
	virtual ~CreatureState() {}
	virtual void enter(Creature* creature) {}
	virtual void update(Creature* creature, float elapsed) {}
};