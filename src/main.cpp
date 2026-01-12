#include "include\Application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    LumX::Application app;

    if (FAILED(app.Initialize()))
    {
        return 1;
    }

    return app.Run();
}
