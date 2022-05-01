#include <iostream>
#include "Custos.hxx"

int main(int argc, char* argv[]){
    auto Watcher = Custos(".");

    // ! Listening when a file is created and deleted at the same time.
    Watcher.On({Custos::Event::FILE_CREATED, Custos::Event::FILE_DELETED}, [](auto &Event){
        // ! We in the <Event> variable that we receive as a parameter 
        // ! have two members, one called <Path> that indicates the 
        // ! path of the file where the movement has been detected along with 
        // ! the <Type> member that indicates the type of movement that was 
        // ! detected, we will use the <Type> member to indicate what action 
        // ! has been detected, whether the file has been created or deleted.
        if(Event.Type == Custos::Event::FILE_CREATED)
            std::cout << "File created: " << Event.Path << std::endl;
        else
            std::cout << "File deleted: " << Event.Path << std::endl; 
    });

    // ! Listening when a directory is created and deleted at the same time.
    Watcher.On({Custos::Event::DIRECTORY_CREATED, Custos::Event::DIRECTORY_DELETED}, [](auto &Event){
        // ! As we did to listen to the multiple actions of the files, we will 
        // ! do the same with the directories, as previously mentioned, the 
        // ! <Type> member that contains the <Event> structure will have the 
        // ! movement that has been detected, with the help of an IF we will 
        // ! do match to execute the corresponding instructions according to the movement.
        if(Event.Type == Custos::Event::DIRECTORY_DELETED)
            std::cout << "Directory deleted: " << Event.Path << std::endl;
        else
            std::cout << "Directory created: " << Event.Path << std::endl;
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