#include "myg/MYGEditor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    myg::MYGEditor editor;

    if (!editor.Initialize()) {
        std::cerr << "Failed to initialize MYG Editor" << std::endl;
        return 1;
    }

    editor.Run();
    editor.Shutdown();

    return 0;
}
