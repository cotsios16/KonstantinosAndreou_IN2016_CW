#ifndef __EXTRALIFEPOWERUP_H__
#define __EXTRALIFEPOWERUP_H__

#include "PowerupBase.h"
#include "Spaceship.h"
#include <functional>

// When collected by the spaceship, grants the player one extra life.
// Uses a callback so it can notify Asteroids to update the Player and GUI
// without needing a direct reference to the Asteroids class.
class ExtraLifePowerup : public PowerupBase
{
public:
	// Callback type: called when the power-up is collected
	typedef std::function<void()> ExtraLifeCallback;

	ExtraLifePowerup(ExtraLifeCallback callback)
		: PowerupBase("ExtraLifePowerup"), mCallback(callback)
	{}

	virtual ~ExtraLifePowerup() {}

	void ApplyEffect(shared_ptr<GameObject> spaceship)
	{
		// Fire the callback - this triggers Player::AddLife() in Asteroids
		if (mCallback) mCallback();
	}

private:
	ExtraLifeCallback mCallback;
};

#endif