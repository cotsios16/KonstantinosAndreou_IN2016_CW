#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h"
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include <vector>

class GameObject;
class Spaceship;
class GUILabel;

// Stores a single high score entry
struct HighScoreEntry
{
	std::string name;
	int score;
};

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener,
                  public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// IKeyboardListener
	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// IScoreListener
	void OnScoreChanged(int score);

	// IPlayerListener
	void OnPlayerKilled(int lives_left);

	// IGameWorldListener
	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// ITimerListener
	void OnTimer(int value);

private:
	// Game objects
	shared_ptr<Spaceship> mSpaceship;

	// HUD labels (visible during gameplay)
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	// Menu / screen labels
	shared_ptr<GUILabel> mTitleLabel;
	shared_ptr<GUILabel> mMenuLabel;
	shared_ptr<GUILabel> mInstructionsLabel;
	shared_ptr<GUILabel> mHighScoreLabel;
	shared_ptr<GUILabel> mNameEntryLabel;

	// Game progression
	uint mLevel;
	uint mAsteroidCount;

	// Game state machine
	enum GameState
	{
		STATE_MENU,
		STATE_INSTRUCTIONS,
		STATE_HIGHSCORES,
		STATE_PLAYING,
		STATE_GAME_OVER,
		STATE_NAME_ENTRY
	};
	GameState mGameState;
	int mMenuSelection;

	// Difficulty toggle
	bool mPowerupsEnabled;

	// High score table
	static const int MAX_HIGH_SCORES = 10;
	std::vector<HighScoreEntry> mHighScores;
	std::string mEnteredName;
	int mFinalScore;

	// Helper methods
	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();

	void ShowMenu();
	void HideMenu();
	void ShowInstructions();
	void HideInstructions();
	void ShowHighScores();
	void HideHighScores();
	void StartGame();
	void ShowGameOverScreen();
	void ShowNameEntry();
	void FinishNameEntry();
	void UpdateMenuLabel();
	void UpdateHighScoreLabel();
	void AddHighScore(const std::string& name, int score);
	void SpawnRandomPowerup();
	void SetHUDVisible(bool visible);

	// Timer values
	const static uint SHOW_GAME_OVER    = 0;
	const static uint START_NEXT_LEVEL  = 1;
	const static uint CREATE_NEW_PLAYER = 2;
	const static uint SPAWN_POWERUP     = 3;
	const static uint SHOW_NAME_ENTRY   = 4;

	ScoreKeeper mScoreKeeper;
	Player      mPlayer;
};

#endif