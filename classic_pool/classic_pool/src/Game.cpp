#include "Game.h"

/// <summary>
/// Game constructor.
/// </summary>
Game::Game() : window{ sf::VideoMode{ { 1920, 1080 }, 32 }, "Classic Pool", sf::Style::Resize }
{

}

/// <summary>
/// Game destructor.
/// </summary>
Game::~Game()
{

}

/// <summary>
/// Run.
/// </summary>
void Game::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	sf::Time timePerFrame = sf::seconds(1.f / 60.0f);

	while (this->window.isOpen() && !this->exitGame)
	{
		timeSinceLastUpdate += clock.restart();

		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents();
			update(timePerFrame);	
		}		

		draw();
	}
}

/// <summary>
/// Process events.
/// </summary>
void Game::processEvents()
{
	while (auto event = window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			window.close();
		}

		if (event->is<sf::Event::KeyPressed>())
		{
			const auto *key = event->getIf<sf::Event::KeyPressed>();

			if (key && key->scancode == sf::Keyboard::Scan::Escape)
			{
				exitGame = true;
			}
		}
	}
}

/// <summary>
/// Update.
/// </summary>
/// <param name="dt">Delta time.</param>
void Game::update(sf::Time dt)
{

}

/// <summary>
/// Draw.
/// </summary>
void Game::draw()
{
	this->window.clear();

	this->window.display();
}

