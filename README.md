# Custos
##### A lightweight file system watcher written in C++17, An easy to use Single-Header library to include in your projects.

This library will allow you to listen to different movements in the file system, such as the creation of a new directory, the modification of a file, the deletion of both etc..., this library is written in CPP17, easy to integrate in your CPP projects as it is a Single-Header.

Many times it is necessary to be listening to the movements that are made in a certain directory, with this library you will not only be able to listen to the movements of a directory, but also multiple ones, in this way you will be able to execute different instructions depending on the movement that is made, for example restart a server when detecting that a new file has been edited, deleted or created in a certain anatomy of any directory that you are listening to, this way you will not have to do it manually every time you perform an action in the aforementioned directory.

### Installation
Before we explain how to use this library, we must first start by installing it, it is quite easy and you only have to execute a line of code in your terminal

### The examples folder
Like past installments, this library has example files, files where you can learn how you can use this library, deducing its use, in the same way it will not take you any time to give you a little reading about this README, then to go to the example files and in this way have a knowledge base to be able to integrate this library in your projects.

```bash
# Cloning the repository where the source code of the library is located.
git clone https://github.com/CodeWithRodi/Custos/
# Entering the directory that generated the cloning of the repository, directory that contains the source code of the library, in the file 'Custos.hxx'
cd Custos
```
Once the repository has been cloned and the folder has been generated, you will have the source code of the library on your computer. Inside the folder generated after cloning, you will have the file 'Custos.hxx', a file that you must include in your projects in order to use this tool.

### Basic example
To begin to familiarize ourselves with the library, we are going to see a very basic example of use, pay attention.
```c++
#include <iostream>
// ! Including the library
#include "Custos.hxx"

int main(int argc, char* argv[]){
    // ! Creating an instance of Custos to start listening to the current directory where the program is running (Current working directory), we do this by sending it as a "." parameter, Custos() can receive multiple parameters, which must be directory paths in where will the listening be done
    auto Watcher = Custos(".");

    // ! As the first parameter we will indicate the type of movement that we want to listen to, the Custos::Event enumeration contains all the movements that can be listened to, it must be sent as the first parameter while as the second the callback that will be executed when the movement is detected must be sent in the directories that you have entered when instantiating <Watcher>, this callback receives a parameter which will be the event information, we receive this parameter as <Event> and it is a structure with two members, the first will be <Event.Path> that will be the path of the file or directory where the movement has been detected followed by <Event.Type> which will be the type of movement, for this example <Event.Type> will correspond to a <Custos::Event::FILE_CREATED>.
    Watcher.On(Custos::Event::FILE_CREATED, [](auto &Event){
        std::cout << "File created: " << Event.Path << std::endl;
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
```
The previous example is a basic use that you can give to this library, where you will be listening when a file is created in the current working directory, which we have indicated by sending Custos "." as a parameter.

### Handling of multiple movements
We can listen to various movements in the directory, now we will do an example of a somewhat more advanced use in this library, next we will listen when a file is created and deleted together with when a directory is created and deleted.
```c++
#include <iostream>
#include "Custos.hxx"

int main(int argc, char* argv[]){
    auto Watcher = Custos(".");

    // ! Listening when a file is created and deleted at the same time.
    Watcher.On({Custos::Event::FILE_CREATED, Custos::Event::FILE_DELETED}, [](auto &Event){
        // ! We in the <Event> variable that we receive as a parameter have two members, one called <Path> that indicates the  path of the file where the movement has been detected along with the <Type> member that indicates the type of movement that was detected, we will use the <Type> member to indicate what action has been detected, whether the file has been created or deleted.
        if(Event.Type == Custos::Event::FILE_CREATED)
            std::cout << "File created: " << Event.Path << std::endl;
        else
            std::cout << "File deleted: " << Event.Path << std::endl; 
    });

    // ! Listening when a directory is created and deleted at the same time.
    Watcher.On({Custos::Event::DIRECTORY_CREATED, Custos::Event::DIRECTORY_DELETED}, [](auto &Event){
        // ! As we did to listen to the multiple actions of the files, we will do the same with the directories, as previously mentioned, the <Type> member that contains the <Event> structure will have the movement that has been detected, with the help of an IF we will do match to execute the corresponding instructions according to the movement.
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
```
This example shows you how you can listen to several movements in the same callback, you can listen to all the movements you want, in this case we separate the movements of files and directories to have a bit of order.
### Events types
| Event | Description |
| ------ | ------ |
| Custos::Event::FILE_CREATED | Event to be fired when a file is created in one of the listening directories. |
| Custos::Event::FILE_OPENEDED | Event to fire when a file is opened in one of the listening directories. |
| Custos::Event::FILE_CREATED | Event to fire when a file is created in one of the listening directories. |
| Custos::Event::FILE_MODIFIED | Event to be fired when a file in one of the listening directories is modified. |
| Custos::Event::FILE_CLOSED | Event to be fired when a file is closed after being opened in one of the listening directories. |
| Custos::Event::FILE_DELETED | Event to fire when a file is deleted in one of the listening directories. |
| Custos::Event::DIRECTORY_CREATED | Event to fire when a directory is created in one of the listening directories. |
| Custos::Event::DIRECTORY_OPENED |Event to fire when a directory is opened in one of the listening directories. |
| Custos::Event::DIRECTORY_MODIFIED | Event to fire when a directory is modified in one of the listening directories. |
| Custos::Event::DIRECTORY_CLOSED | Event to fire when a directory is closed after being opened in one of the listening directories. |
| Custos::Event::DIRECTORY_DELETED | Event to fire when a directory is removed in one of the listening directories. |

### Event parameter in the callback
| Parameter | Description |
| ------ | ------ |
| Event.Type | Contains the event that was fired, this parameter is useful when working within a callback listening to multiple events, Event.Type can contain Custos::Event::FILE_CREATED - Custos::Event::DIRECTORY_OPENED - Custos::Event::DIRECTORY_MODIFIED - Custos::Event::FILE_CREATED... etc, all available events named above.|
| Event.Path | Contains a string which will contain the path of the file or directory in which the event was fired, for example "Public/Layout/Base.html". |

### Contributions and future versions
This library is open to the public, under the MIT license, open to new contributions and possible improvements, the important thing is to develop and learn, use it as you want.

### Remember to drink water bby.
