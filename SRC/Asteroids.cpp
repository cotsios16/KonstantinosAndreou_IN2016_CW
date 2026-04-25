#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "ExtraLifePowerup.h"
#include "InvulnerabilityPowerup.h"
#include "TeleportPowerup.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel            = 0;
	mAsteroidCount    = 0;
	mGameState        = STATE_MENU;
	mMenuSelection    = 0;
	mPowerupsEnabled  = true;
	mFinalScore       = 0;
}

Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	// Load animations
	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	// Create all GUI labels
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Spawn background asteroids for the menu screen (no spaceship yet)
	CreateAsteroids(10);

	// Show the main menu
	ShowMenu();

	// Start the game session (enters GLUT main loop)
	GameSession::Start();
}

void Asteroids::Stop()
{
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	// -------------------------------------------------------------------
	// NAME ENTRY: collect characters for the gamer tag
	if (mGameState == STATE_NAME_ENTRY)
	{
		if (key == 13) // Enter key - confirm name
		{
			FinishNameEntry();
		}
		else if (key == 8 || key == 127) // Backspace
		{
			if (!mEnteredName.empty())
				mEnteredName.pop_back();
			mNameEntryLabel->SetText("Enter your name: " + mEnteredName + "_");
		}
		else if (mEnteredName.size() < 12 && key >= 32 && key <= 126)
		{
			mEnteredName += (char)key;
			mNameEntryLabel->SetText("Enter your name: " + mEnteredName + "_");
		}
		return;
	}

	// -------------------------------------------------------------------
	// MENU: select the highlighted option with Enter
	if (mGameState == STATE_MENU)
	{
		if (key == 13) // Enter
		{
			switch (mMenuSelection)
			{
			case 0: StartGame();        break;
			case 1:
				mPowerupsEnabled = !mPowerupsEnabled;
				UpdateMenuLabel();
				break;
			case 2: ShowInstructions(); break;
			case 3: ShowHighScores();   break;
			}
		}
		return;
	}

	// -------------------------------------------------------------------
	// INSTRUCTIONS / HIGH SCORES: any key returns to menu
	if (mGameState == STATE_INSTRUCTIONS || mGameState == STATE_HIGHSCORES)
	{
		if (key == 27 || key == 13) // Esc or Enter
		{
			if (mGameState == STATE_INSTRUCTIONS) HideInstructions();
			else                                  HideHighScores();
			ShowMenu();
		}
		return;
	}

	// -------------------------------------------------------------------
	// PLAYING: spacebar shoots
	if (mGameState == STATE_PLAYING)
	{
		if (key == ' ') mSpaceship->Shoot();
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	// Navigate menu with up/down arrows
	if (mGameState == STATE_MENU)
	{
		if (key == GLUT_KEY_UP)
		{
			mMenuSelection = (mMenuSelection - 1 + 4) % 4;
			UpdateMenuLabel();
		}
		else if (key == GLUT_KEY_DOWN)
		{
			mMenuSelection = (mMenuSelection + 1) % 4;
			UpdateMenuLabel();
		}
		return;
	}

	// Ship controls during gameplay
	if (mGameState == STATE_PLAYING)
	{
		switch (key)
		{
		case GLUT_KEY_UP:    mSpaceship->Thrust(10);  break;
		case GLUT_KEY_LEFT:  mSpaceship->Rotate(90);  break;
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
		default: break;
		}
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (mGameState == STATE_PLAYING)
	{
		switch (key)
		{
		case GLUT_KEY_UP:    mSpaceship->Thrust(0); break;
		case GLUT_KEY_LEFT:  mSpaceship->Rotate(0); break;
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
		default: break;
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0 && mGameState == STATE_PLAYING)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
		// Spawn a power-up shortly after each new level starts
		if (mPowerupsEnabled)
		{
			SetTimer(5000, SPAWN_POWERUP);
		}
	}

	if (value == SHOW_GAME_OVER)
	{
		ShowGameOverScreen();
	}

	if (value == SPAWN_POWERUP)
	{
		if (mGameState == STATE_PLAYING && mPowerupsEnabled)
		{
			SpawnRandomPowerup();
			// Schedule the next power-up spawn in 15 seconds
			SetTimer(15000, SPAWN_POWERUP);
		}
	}

	if (value == SHOW_NAME_ENTRY)
	{
		ShowNameEntry();
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////

shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	mSpaceship->Reset();
	return mSpaceship;
}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount += num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

// ---------------------------------------------------------------------------
// GUI
// ---------------------------------------------------------------------------

void Asteroids::CreateGUI()
{
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	// Score label - top left, hidden until game starts
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mScoreLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mScoreLabel), GLVector2f(0.0f, 1.0f));

	// Lives label - bottom left, hidden until game starts
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mLivesLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mLivesLabel), GLVector2f(0.0f, 0.0f));

	// Game over label - centre, hidden until needed
	mGameOverLabel = make_shared<GUILabel>("GAME OVER");
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameOverLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mGameOverLabel), GLVector2f(0.5f, 0.5f));

	// Title label - top centre, shown on menu
	mTitleLabel = make_shared<GUILabel>("** ASTEROIDS **");
	mTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mTitleLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mTitleLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mTitleLabel), GLVector2f(0.5f, 0.9f));

	// Menu label - centre, shown on menu
	mMenuLabel = make_shared<GUILabel>("");
	mMenuLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mMenuLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mMenuLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mMenuLabel), GLVector2f(0.5f, 0.5f));

	// Instructions label - centre, shown on instructions screen
	mInstructionsLabel = make_shared<GUILabel>(
		"CONTROLS\n"
		"UP ARROW  - Thrust\n"
		"LEFT/RIGHT - Rotate\n"
		"SPACE     - Shoot\n"
		"\n"
		"POWERUPS\n"
		"Extra Life  - gain one life\n"
		"Shield      - invulnerable 5s\n"
		"Teleport    - jump to safety\n"
		"\n"
		"Press ENTER to return"
	);
	mInstructionsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mInstructionsLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsLabel), GLVector2f(0.5f, 0.5f));

	// High score label - centre, shown on high scores screen
	mHighScoreLabel = make_shared<GUILabel>("HIGH SCORES\n(none yet)");
	mHighScoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mHighScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mHighScoreLabel), GLVector2f(0.5f, 0.5f));

	// Name entry label - centre, shown after game over
	mNameEntryLabel = make_shared<GUILabel>("Enter your name: _");
	mNameEntryLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mNameEntryLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mNameEntryLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mNameEntryLabel), GLVector2f(0.5f, 0.4f));
}

// ---------------------------------------------------------------------------
// Menu / screen transitions
// ---------------------------------------------------------------------------

void Asteroids::UpdateMenuLabel()
{
	std::string powerupStr = mPowerupsEnabled ? "ON" : "OFF";
	std::ostringstream ss;
	ss << (mMenuSelection == 0 ? "> " : "  ") << "START GAME\n"
	   << (mMenuSelection == 1 ? "> " : "  ") << "POWERUPS: " << powerupStr << "\n"
	   << (mMenuSelection == 2 ? "> " : "  ") << "INSTRUCTIONS\n"
	   << (mMenuSelection == 3 ? "> " : "  ") << "HIGH SCORES";
	mMenuLabel->SetText(ss.str());
}

void Asteroids::ShowMenu()
{
	mGameState     = STATE_MENU;
	mMenuSelection = 0;
	UpdateMenuLabel();
	mTitleLabel->SetVisible(true);
	mMenuLabel->SetVisible(true);
}

void Asteroids::HideMenu()
{
	mTitleLabel->SetVisible(false);
	mMenuLabel->SetVisible(false);
}

void Asteroids::ShowInstructions()
{
	HideMenu();
	mGameState = STATE_INSTRUCTIONS;
	mInstructionsLabel->SetVisible(true);
}

void Asteroids::HideInstructions()
{
	mInstructionsLabel->SetVisible(false);
}

void Asteroids::ShowHighScores()
{
	HideMenu();
	UpdateHighScoreLabel();
	mGameState = STATE_HIGHSCORES;
	mHighScoreLabel->SetVisible(true);
}

void Asteroids::HideHighScores()
{
	mHighScoreLabel->SetVisible(false);
}

void Asteroids::StartGame()
{
	HideMenu();
	mGameState     = STATE_PLAYING;
	mLevel         = 0;
	mAsteroidCount = 0;

	// Show HUD and reset labels
	SetHUDVisible(true);
	mScoreLabel->SetText("Score: 0");
	mLivesLabel->SetText("Lives: 3");

	// Create and add the spaceship to the world
	mGameWorld->AddObject(CreateSpaceship());

	// Create the first wave of asteroids
	CreateAsteroids(10);

	// Schedule first power-up spawn after 10 seconds
	if (mPowerupsEnabled)
	{
		SetTimer(10000, SPAWN_POWERUP);
	}
}

void Asteroids::SetHUDVisible(bool visible)
{
	mScoreLabel->SetVisible(visible);
	mLivesLabel->SetVisible(visible);
}

void Asteroids::ShowGameOverScreen()
{
	mGameState  = STATE_GAME_OVER;
	mFinalScore = mScoreKeeper.GetScore();
	mGameOverLabel->SetVisible(true);
	// Wait 1.5 seconds then prompt for name entry
	SetTimer(1500, SHOW_NAME_ENTRY);
}

void Asteroids::ShowNameEntry()
{
	mGameOverLabel->SetVisible(false);
	mEnteredName = "";
	mGameState   = STATE_NAME_ENTRY;
	mNameEntryLabel->SetText("Enter your name: _");
	mNameEntryLabel->SetVisible(true);
}

void Asteroids::FinishNameEntry()
{
	mNameEntryLabel->SetVisible(false);
	std::string name = mEnteredName.empty() ? "PLAYER" : mEnteredName;
	AddHighScore(name, mFinalScore);
	SetHUDVisible(false);
	ShowHighScores();
}

void Asteroids::AddHighScore(const std::string& name, int score)
{
	HighScoreEntry entry;
	entry.name  = name;
	entry.score = score;
	mHighScores.push_back(entry);

	// Sort descending by score
	std::sort(mHighScores.begin(), mHighScores.end(),
		[](const HighScoreEntry& a, const HighScoreEntry& b) {
			return a.score > b.score;
		});

	// Keep only top 10
	if (mHighScores.size() > MAX_HIGH_SCORES)
		mHighScores.resize(MAX_HIGH_SCORES);
}

void Asteroids::UpdateHighScoreLabel()
{
	std::ostringstream ss;
	ss << "--- HIGH SCORES ---\n\n";
	if (mHighScores.empty())
	{
		ss << "(No scores yet)\n";
	}
	else
	{
		for (int i = 0; i < (int)mHighScores.size(); i++)
		{
			ss << (i + 1) << ".  " << mHighScores[i].name
			   << "  " << mHighScores[i].score << "\n";
		}
	}
	ss << "\nPress ENTER to return";
	mHighScoreLabel->SetText(ss.str());
}

// ---------------------------------------------------------------------------
// Power-up spawning
// ---------------------------------------------------------------------------

void Asteroids::SpawnRandomPowerup()
{
	if (!mPowerupsEnabled) return;

	int which = rand() % 3;
	shared_ptr<GameObject> powerup;

	if (which == 0)
	{
		// Extra life: use a lambda callback so the powerup can call Player::AddLife()
		shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);
		auto callback = [thisPtr]() {
			thisPtr->mPlayer.AddLife();
		};
		powerup = make_shared<ExtraLifePowerup>(callback);
	}
	else if (which == 1)
	{
		powerup = make_shared<InvulnerabilityPowerup>();
	}
	else
	{
		powerup = make_shared<TeleportPowerup>();
	}

	powerup->SetBoundingShape(make_shared<BoundingSphere>(powerup->GetThisPtr(), 5.0f));
	mGameWorld->AddObject(powerup);
}

// ---------------------------------------------------------------------------
// Score / player event callbacks
// ---------------------------------------------------------------------------

void Asteroids::OnScoreChanged(int score)
{
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	mScoreLabel->SetText(msg_stream.str());
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	mLivesLabel->SetText(msg_stream.str());

	if (lives_left > 0)
	{
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

// ---------------------------------------------------------------------------
// Explosion helper
// ---------------------------------------------------------------------------

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}