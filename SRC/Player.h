#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "GameUtil.h"

#include "GameObject.h"
#include "GameObjectType.h"
#include "IPlayerListener.h"
#include "IGameWorldListener.h"

class Player : public IGameWorldListener
{
public:
	Player() { mLives = 3; }
	virtual ~Player() {}

	void OnWorldUpdated(GameWorld* world) {}

	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}

	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
	{
		if (object->GetType() == GameObjectType("Spaceship")) {
			mLives -= 1;
			FirePlayerKilled();
		}
	}

	// Called by the Extra Life power-up to grant one additional life
	void AddLife()
	{
		mLives += 1;
		FireLivesChanged();
	}

	int GetLives() const { return mLives; }

	void AddListener(shared_ptr<IPlayerListener> listener)
	{
		mListeners.push_back(listener);
	}

	void FirePlayerKilled()
	{
		for (PlayerListenerList::iterator lit = mListeners.begin();
			lit != mListeners.end(); ++lit) {
			(*lit)->OnPlayerKilled(mLives);
		}
	}

	// Fires when lives change without the ship being destroyed (e.g. extra life pickup)
	void FireLivesChanged()
	{
		for (PlayerListenerList::iterator lit = mListeners.begin();
			lit != mListeners.end(); ++lit) {
			(*lit)->OnPlayerKilled(mLives); // reuses OnPlayerKilled to refresh the GUI label
		}
	}

private:
	int mLives;

	typedef std::list< shared_ptr<IPlayerListener> > PlayerListenerList;
	PlayerListenerList mListeners;
};

#endif