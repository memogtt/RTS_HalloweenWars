#pragma once

namespace Core
{
	class GameEngine
	{
	public:

		static void GameStart();
		static void GameLoop();
		static void GameEnd();

		static bool GameLoopInitial(float deltaTime);
		static bool GameLoopLobby(float deltaTime);
		static bool GameLoopSinglePlayer(float deltaTime);
		static bool GameLoopMultiPlayer(float deltaTime);
		static bool GameCleanup();

		// Utility methods
		static int checkWinCondition();
		static void generateHouses(int num);
		static void assignCurrentPlayersToHouses(int num);
	};
}