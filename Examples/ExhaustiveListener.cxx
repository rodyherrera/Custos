#include <iostream>
#include "Custos.hxx"

int main(int argc, char* argv[]){
    // ? With "." We indicate that we want to listen to the 
    // ? movements of the current directory where the program 
    // ? will be executed. Another directory could be for 
    // ? example /home/myuser/Desktop/MyFolder/
    auto Watcher = Custos(".");

    // ! We can listen to two or more directories at the same 
    // ! time, it will only be enough to pass them as parameters.
    // * auto Watcher = Custos("FolderOne/", "FolderTwo/", "FolderThree/");

    // ! Directory listeners
    Watcher.On(Custos::Event::DIRECTORY_CLOSED, [](auto &Event){
        std::cout << "Directory closed: " << Event.Path << std::endl;
    });

    Watcher.On(Custos::Event::DIRECTORY_CREATED, [](auto &Event){
        std::cout << "Directory created: " << Event.Path << std::endl;
    });

    Watcher.On(Custos::Event::DIRECTORY_DELETED, [](auto &Event){
        std::cout << "Directory deleted: " << Event.Path << std::endl;
    });

    Watcher.On(Custos::Event::DIRECTORY_MODIFIED, [](auto &Event){
        std::cout << "Directory modified: " << Event.Path << std::endl;
    });

    Watcher.On(Custos::Event::DIRECTORY_OPENED, [](auto &Event){
        std::cout << "Directory opened: " << Event.Path << std::endl;
    });

    // ! File listeners
    Watcher.On(Custos::Event::FILE_CLOSED, [](auto &Event){
        std::cout << "File closed: " << Event.Path << std::endl;
    });

    Watcher.On(Custos::Event::FILE_CREATED, [](auto &Event){
        std::cout << "File created: " << Event.Path << std::endl;
    });

    Watcher.On(Custos::Event::FILE_DELETED, [](auto &Event){
        std::cout << "File deleted: " << Event.Path << std::endl; 
    });

    Watcher.On(Custos::Event::FILE_MODIFIED, [](auto &Event){
        std::cout << "File modified: " << Event.Path << std::endl;
    });

    Watcher.On(Custos::Event::FILE_OPENED, [](auto &Event){
        std::cout << "File openeded: " << Event.Path << std::endl; 
    });

    try{
        // ! Starting the Watcher, this will go into a loop.
        Watcher.Start();
    }catch(const std::runtime_error& RuntimeError){
        // ! If any error occurs, we will display the message in the terminal.
        std::cout << RuntimeError.what() << std::endl;
    }

    return 0;
}