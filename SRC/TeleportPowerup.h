#ifndef __TELEPORTPOWERUP_H__
#define __TELEPORTPOWERUP_H__

#include "PowerupBase.h"
#include "Spaceship.h"

// When collected, instantly moves the spaceship to a random safe position
// on screen and zeroes its velocity so it doesn't fly off uncontrollably.
class TeleportPowerup : public PowerupBase
{
public:
	TeleportPowerup() : PowerupBase("TeleportPowerup") {}

	virtual ~TeleportPowerup() {}

	void ApplyEffect(shared_ptr<GameObject> spaceship)
	{
		shared_ptr<Spaceship> ship = dynamic_pointer_cast<Spaceship>(spaceship);
		if (ship)
		{
			// Move ship to a new random position within the world bounds
			GLVector3f newPos;
			newPos.x = (float)((rand() % 160) - 80);
			newPos.y = (float)((rand() % 160) - 80);
			newPos.z = 0.0f;
			ship->SetPosition(newPos);

			// Zero out velocity and acceleration so the ship
			// doesn't carry its old momentum into the new position
			GLVector3f zero(0.0f, 0.0f, 0.0f);
			ship->SetVelocity(zero);
			ship->SetAcceleration(zero);
		}
	}
};

#endif