#include "GameUtil.h"
#include "GameWorld.h"
#include "Bullet.h"
#include "Spaceship.h"
#include "BoundingSphere.h"

using namespace std;

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

Spaceship::Spaceship()
	: GameObject("Spaceship"), mThrust(0), mInvulnerableTimer(0)
{
}

Spaceship::Spaceship(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r)
	: GameObject("Spaceship", p, v, a, h, r), mThrust(0), mInvulnerableTimer(0)
{
}

Spaceship::Spaceship(const Spaceship& s)
	: GameObject(s), mThrust(0), mInvulnerableTimer(0)
{
}

Spaceship::~Spaceship(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

void Spaceship::Update(int t)
{
	// Tick down the invulnerability timer each frame
	if (mInvulnerableTimer > 0)
	{
		mInvulnerableTimer -= t;
		if (mInvulnerableTimer < 0) mInvulnerableTimer = 0;
	}

	// Call parent update
	GameObject::Update(t);
}

void Spaceship::Render(void)
{
	// When invulnerable, flash the ship by skipping render every other 200ms window
	if (IsInvulnerable())
	{
		if ((mInvulnerableTimer / 200) % 2 == 0)
			return; // skip this frame to create a blinking effect
	}

	if (mSpaceshipShape.get() != NULL) mSpaceshipShape->Render();

	if ((mThrust > 0) && (mThrusterShape.get() != NULL))
		mThrusterShape->Render();

	GameObject::Render();
}

void Spaceship::Thrust(float t)
{
	mThrust = t;
	mAcceleration.x = mThrust * cos(DEG2RAD * mAngle);
	mAcceleration.y = mThrust * sin(DEG2RAD * mAngle);
}

void Spaceship::Rotate(float r)
{
	mRotation = r;
}

void Spaceship::Shoot(void)
{
	if (!mWorld) return;
	GLVector3f spaceship_heading(cos(DEG2RAD * mAngle), sin(DEG2RAD * mAngle), 0);
	spaceship_heading.normalize();
	GLVector3f bullet_position = mPosition + (spaceship_heading * 4);
	float bullet_speed = 30;
	GLVector3f bullet_velocity = mVelocity + spaceship_heading * bullet_speed;
	shared_ptr<GameObject> bullet
		(new Bullet(bullet_position, bullet_velocity, mAcceleration, mAngle, 0, 2000));
	bullet->SetBoundingShape(make_shared<BoundingSphere>(bullet->GetThisPtr(), 2.0f));
	bullet->SetShape(mBulletShape);
	mWorld->AddObject(bullet);
}

bool Spaceship::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() != GameObjectType("Asteroid") &&
		o->GetType() != GameObjectType("ExtraLifePowerup") &&
		o->GetType() != GameObjectType("InvulnerabilityPowerup") &&
		o->GetType() != GameObjectType("TeleportPowerup")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Spaceship::OnCollision(const GameObjectList& objects)
{
	for (GameObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it)
	{
		if ((*it)->GetType() == GameObjectType("Asteroid"))
		{
			if (!IsInvulnerable())
			{
				mWorld->FlagForRemoval(GetThisPtr());
				return;
			}
		}
	}
}