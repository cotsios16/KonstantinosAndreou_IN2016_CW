#ifndef __POWERUPBASE_H__
#define __POWERUPBASE_H__

#include "GameUtil.h"
#include "GameObject.h"
#include "BoundingShape.h"

// Base class for all power-ups. Power-ups float around the world and activate
// when the spaceship flies into them, then remove themselves.
class PowerupBase : public GameObject
{
public:
	PowerupBase(const char* type_name) : GameObject(type_name)
	{
		// Spawn at a random position in the world
		mPosition.x = (float)((rand() % 160) - 80);
		mPosition.y = (float)((rand() % 160) - 80);
		mPosition.z = 0.0f;
		// Slow random drift
		mVelocity.x = (float)((rand() % 5) - 2);
		mVelocity.y = (float)((rand() % 5) - 2);
		mVelocity.z = 0.0f;
		mAngle    = 0.0f;
		mRotation = 20.0f; // slowly spin so it's visible
	}

	virtual ~PowerupBase() {}

	// Only collide with the spaceship
	bool CollisionTest(shared_ptr<GameObject> o)
	{
		if (o->GetType() != GameObjectType("Spaceship")) return false;
		if (mBoundingShape.get() == NULL) return false;
		if (o->GetBoundingShape().get() == NULL) return false;
		return mBoundingShape->CollisionTest(o->GetBoundingShape());
	}

	// Each subclass defines what effect it has on the spaceship
	virtual void ApplyEffect(shared_ptr<GameObject> spaceship) = 0;

	void OnCollision(const GameObjectList& objects)
	{
		for (GameObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it)
		{
			if ((*it)->GetType() == GameObjectType("Spaceship"))
			{
				ApplyEffect(*it);
			}
		}
		// Remove the power-up from the world after it's been collected
		mWorld->FlagForRemoval(GetThisPtr());
	}
};

#endif