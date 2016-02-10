
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <string.h>
#include <libgen.h>

class Config {
	protected:
		struct {
			int winlen           = 256;
			int hop              = 64;
			double lopass        = 01.25;
			double hipass        = 10.00;
			double threshold     = 14.00;
			double floor         = 24.00;
			sf::Color winBG      = sf::Color(0xFE,0xEF,0xE0);
			sf::Color specBG     = sf::Color(0xFF,0xFF,0xFF);
			sf::Color specFG     = sf::Color(0x00,0x00,0x00);
			sf::Color threshFG   = sf::Color(0x58,0x74,0x98,0x48);
			sf::Color pointFB    = sf::Color(0x33,0x99,0xFF);
			sf::Color pointBG    = sf::Color(0x33,0x99,0xFF,0x48);
			sf::Color linesFG    = sf::Color(0xE8,0x68,0x50);
		} conf;
		struct {
			bool cursor       = false;
			bool hud          = true;
			bool overlay      = true;
		} show;
		char *fname = NULL, *name = NULL;

	public:
		Config(int, const char **);
		~Config();
};
