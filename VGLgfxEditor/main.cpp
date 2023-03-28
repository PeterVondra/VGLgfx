#include "VGLgfxEditor.h"

int main()
{
	vgl::AppConfig config;
	config.DefaultWindowSize = { 1280, 720 };
	config.MSAASamples = 8;
	config.Title = "VGL";
	config.ImGui_Font_Path = "../data/Fonts/OpenSans-Regular.ttf";
	auto app = new TestApp(config);
	//auto app = new vgl::Editor(config);

	app->run();
}