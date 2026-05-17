#include "Logger.h"
#include "Application.h"

int main() {

	FreeConsole();	// Remove console window. Logger wont print anything if console turned off.
	Logger::getInstance().initialize("client.log");					// Initialize logger and file to output.
	Logger::getInstance().setMinimalLevel(Logger::Level::Info);		// Set level of logging.
	Logger::getInstance().setShowFileInfo(false);

	Application messenger(1080, 720, "Messenger");	// Create Application
	messenger.run();	// Run main loop

	return 0;
}
