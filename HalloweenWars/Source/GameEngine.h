#pragma once

namespace Core
{
	class GameEngine
	{
	public:
		static void GameStart();
		static void GameLoop();
		static void GameEnd();

		bool static GameLoopInitial(float deltaTime);

		bool GameLoopLobby(float deltaTime);
		bool GameLoopSinglePlayer(float deltaTime);
		bool GameLoopMultiPlayer(float deltaTime);
		bool GameCleanup();

		// Utility methods
		int checkWinCondition();
		void generateHouses(int num);
		void assignCurrentPlayersToHouses(int num);

	};
}