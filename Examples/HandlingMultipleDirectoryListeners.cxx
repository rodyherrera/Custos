/***
 * Copyright (C) Rodolfo Herrera Hernandez. All rights reserved.
 * Licensed under the MIT license. See LICENSE file in the project root 
 * for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * For related information - https://github.com/rodyherrera/Custos/
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/

#include <iostream>
#include "Custos.hxx"

int main(int argc, char* argv[]){
    auto Watcher = Custos(".");

    // ! Passing different types of listeners as parameters 
    // ! so that our callback is executed when detecting any 
    // ! movement, within the callback with the help of a 
    // ! switch we will execute a series of instructions 
    // ! depending on the type of movement that has been detected, in 
    // ! this way we can reduce code when not being 
    // ! creating a callback for each move.
    Watcher.On({
        Custos::Event::DIRECTORY_CLOSED,
        Custos::Event::DIRECTORY_CREATED,
        Custos::Event::DIRECTORY_DELETED,
        Custos::Event::DIRECTORY_MODIFIED,
        Custos::Event::DIRECTORY_OPENED
    }, [](auto &Event){
        // ! Storing the Path of the file or directory 
        // ! where the movement was detected, we will use this to display it in the terminal.
        std::string Path = Event.Path;
        switch(Event.Type){
            case Custos::Event::DIRECTORY_CLOSED:
                std::cout << "Directory closed: " << Path << std::endl;
                break;
            case Custos::Event::DIRECTORY_CREATED:
                std::cout << "Directory created: " << Path << std::endl;
                break;
            case Custos::Event::DIRECTORY_MODIFIED:
                std::cout << "Directory deleted: " << Path << std::endl;
                break;
            case Custos::Event::DIRECTORY_OPENED:
                std::cout << "Directory modified: " << Path << std::endl;
                break;
            default:
                std::cout << "Directory openeded: " << Path << std::endl;
        }
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
