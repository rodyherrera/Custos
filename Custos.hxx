/***
 * Copyright (C) Rodolfo Herrera Hernandez. All rights reserved.
 * Licensed under the MIT license. See LICENSE file in the project root 
 * for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * For related information - https://github.com/codewithrodi/Custos/
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/

#pragma once
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#ifdef __linux__
    #include <errno.h>
    #include <limits.h>
    #include <signal.h>
    #include <sys/inotify.h>
    #include <sys/types.h>
    #include <unistd.h>
    // ! Max number of events to process at one go
    #define MAX_EVENTS 1024
    // ! Assuming that the length of the filename won't exceed 16 bytes
    #define LENGTH_NAME 16
    // ! Size of one event
    #define EVENT_SIZE (sizeof(struct inotify_event))
    // ! Buffer to store the data of events
    #define EVENT_BUFFER_LENGTH (MAX_EVENTS * (EVENT_SIZE + LENGTH_NAME))
    #define WATCH_FLAGS (IN_CREATE | IN_MODIFY | IN_DELETE | IN_OPEN | IN_CLOSE)
    // ! Keep going while Run == true, or, in other words, unitl user hits ctrl-c
    static bool Run = true;
    void SigCallback(int Sig){
        Run = false;
    }

    class Watch{
        struct WDElement{
            int PD;
            std::string Name;
            bool operator()(const WDElement &Left, const WDElement &Right) const{
                return Left.PD < Right.PD ? (true) : 
                    (Left.PD == Right.PD && Left.Name < Right.Name ? true : false);
            }
        };
        std::map<int, WDElement> Watch;
        std::map<WDElement, int, WDElement> RightWatch;

        public:
            // ! Insert event information, used to create new watch, into Watch object
            void Insert(int PD, const std::string &Name, int WD){
                WDElement Element = {PD, Name};
                Watch[WD] = Element;
                RightWatch[Element] = WD;
            }

            // ! Erase watch specified by PD(Parent watch descriptor) and name from
            // ! watch list, returns full name(For display etc), and WD, which is required
            // ! for <inotify_rm_watch>.
            std::string Erase(int PD, const std::string &Name, int *WD){
                WDElement ParentElement = {PD, Name};
                *WD = RightWatch[ParentElement];
                RightWatch.erase(ParentElement);
                const WDElement &Element = Watch[*WD];
                std::string Directory = Element.Name;
                Watch.erase(*WD);
                return Directory;
            }

            // ! Given a watch descriptor, return the full directory name as string.
            // ! Recurses up parent WDs to assemble name, an idea borrowed from Windows change journals
            std::string Get(int WD){
                const WDElement &Element = Watch[WD];
                return Element.PD == -1 ? Element.Name : this->Get(Element.PD) + "/" + Element.Name;
            }

            // ! Given a parameter WD and Name (Provided in IN_DELETE events), return the watch
            // ! descriptor, main purpose is to help remove directories from watch list.
            int Get(int PD, std::string Name){
                WDElement Element = {PD, Name};
                return RightWatch[Element];
            }

            void Stats(){
                std::cout << "Number of watches = " << Watch.size() << " & reverse watches = " << RightWatch.size() << std::endl;
            }
    };
#endif

class Custos{
    public:
        enum class Event{
            FILE_CREATED,
            FILE_OPENED,
            FILE_MODIFIED,
            FILE_CLOSED,
            FILE_DELETED,
            DIRECTORY_CREATED,
            DIRECTORY_OPENED,
            DIRECTORY_MODIFIED,
            DIRECTORY_CLOSED,
            DIRECTORY_DELETED
        };
        
        struct EventInformation{
            Event Type;
            std::filesystem::path Path;
        };

        Custos(){}

        Custos(const std::string &Directory){
            AppendToPath(Directory);
        }

        template <class ... Arguments>
        Custos(Arguments... Args){
            AppendToPath(Args...);
        }

        void AppendToPath(const std::string &Path){
            Paths.push_back(Expand(std::filesystem::path(Path)));
            if(Path.length() == 0)
                Paths[Paths.size() - 1] = Expand(std::filesystem::path("."));
        }

        template <class... Args>
        void AppendToPath(const std::string &Head, Args... Tail){
            AppendToPath(Head);
            AppendToPath(Tail...);
        }

        void On(const Event &Event, const std::function<void(const EventInformation &)> &Action){
            Callbacks[Event] = Action;
        }

        void On(const std::vector<Event> &Events, const std::function<void(const EventInformation &)> &Action){
            for(auto &Event : Events)
                Callbacks[Event] = Action;
        }

        #ifdef __linux__
        void Stop(){
            Run = false;
        }

        void Start(){
            Watch Watch;
            fd_set WatchSet;
            char Buffer[EVENT_BUFFER_LENGTH];
            std::string CurrentDirectory, NewDirectory;
            int TotalFileEvents = 0;
            int TotalDirectoryEvents = 0;
            Event CurrentEvent;
            // ! Call SigCallback if user hits ctrl-c
            signal(SIGINT, SigCallback);
            #ifdef IN_NONBLOCK
                int FD = inotify_init1(IN_NONBLOCK);
            #else 
                int FD = inotify_init();
            #endif
            // ! Checking for error
            if(FD < 0)
                throw std::runtime_error("inotify_init failed.");
            // ! Use select watch list for non-blocking inotify read
            FD_ZERO(&WatchSet);
            FD_SET(FD, &WatchSet);
            int WD;
            for(auto &Path : Paths){
                auto PathString = Path.string();
                const char* Root = PathString.c_str();
                WD = inotify_add_watch(FD, Root, WATCH_FLAGS);
                // ! Add WD and directory name to Watch map
                Watch.Insert(-1, Root, WD);
            }

            // ! Continue until Run == false, see signal and SigCallback above
            while(Run){
                select(FD + 1, &WatchSet, NULL, NULL, NULL);
                // ! Read events from non-blocking inotify fd
                int Length = read(FD, Buffer, EVENT_BUFFER_LENGTH);
                if(Run && Length < 0)
                    throw std::runtime_error("Failed to read event(s) from <inotify fd>.");
                // ! Loop through event buffer
                for(int Iterator = 0; Iterator < Length;){
                    struct inotify_event *Event = (struct inotify_event *) &Buffer[Iterator];
                    // ! Never actually seen this
                    if(Event->wd == -1)
                        throw std::runtime_error("<inotify IN_Q_OVERFLOW> Event queue overflowed.");
                    // ! Never seen this either
                    if(Event->mask & IN_Q_OVERFLOW)
                        throw std::runtime_error("<inotify IN_Q_OVERFLOW> Event queue overflowed.");
                    if(Event->len){
                        if(Event->mask & IN_IGNORED)
                            // ! Watch was removed explicitly(inotify_rm_watch) or automatically
                            // ! (file was deleted, or filesystem was unmounted)
                            throw std::runtime_error(
                                "<inotify IN_IGNORED> Watch was removed explicitly "
                                "(inotify_rm_watch) or automatically (file was deleted, or "
                                "filesystem was unmounted)"
                            );
                        if(Event->mask & IN_CREATE){
                            CurrentDirectory = Watch.Get(Event->wd);
                            if(Event->mask & IN_ISDIR){
                                NewDirectory = CurrentDirectory + "/" + Event->name;
                                WD = inotify_add_watch(FD, NewDirectory.c_str(), WATCH_FLAGS);
                                Watch.Insert(Event->wd, Event->name, WD);
                                TotalDirectoryEvents++;
                                RunCallback(Event::DIRECTORY_CREATED, CurrentDirectory, Event->name);
                            }else{
                                TotalFileEvents++;
                                RunCallback(Event::FILE_CREATED, CurrentDirectory, Event->name);
                            }
                        }else if(Event->mask & IN_MODIFY){
                            if(Event->mask & IN_ISDIR)
                                RunCallback(Event::DIRECTORY_MODIFIED, CurrentDirectory, Event->name);
                            else
                                RunCallback(Event::FILE_MODIFIED, CurrentDirectory, Event->name);
                        }else if(Event->mask & IN_DELETE){
                            if(Event->mask & IN_ISDIR){
                                // ! Directory was deleted
                                NewDirectory = Watch.Erase(Event->wd, Event->name, &WD);
                                inotify_rm_watch(FD, WD);
                                TotalDirectoryEvents--;
                                RunCallback(Event::DIRECTORY_DELETED, CurrentDirectory, Event->name);
                            }else{
                                // ! File was deleted
                                CurrentDirectory = Watch.Get(Event->wd);
                                TotalFileEvents--;
                                RunCallback(Event::FILE_DELETED, CurrentDirectory, Event->name);
                            }
                        }else if(Event->mask & IN_OPEN){
                            CurrentDirectory = Watch.Get(Event->wd);
                            if(Event->mask & IN_ISDIR){
                                // ! Directory was opened
                                RunCallback(Event::DIRECTORY_OPENED, CurrentDirectory, Event->name);
                            }else{
                                // ! File was openeded
                                RunCallback(Event::FILE_OPENED, CurrentDirectory, Event->name);
                            }
                        }else if(Event->mask & IN_CLOSE){
                            CurrentDirectory = Watch.Get(Event->wd);
                            if(Event->mask & IN_ISDIR){
                                // ! Directory was closed
                                RunCallback(Event::DIRECTORY_CLOSED, CurrentDirectory, Event->name);
                            }else{
                                // ! File was closed
                                RunCallback(Event::FILE_CLOSED, CurrentDirectory, Event->name);
                            }
                        }
                    }
                    Iterator += EVENT_SIZE + Event->len;
                }
            }

            // ! Exit when Run = false or user press ctrl-c
            exit(0);
        }
    #endif

    private:
        // ! Root directory of the file watcher
        std::vector<std::filesystem::path> Paths;
        // ! Callback functions based on file status
        std::map<Event, std::function<void(const EventInformation &)>> Callbacks;
        std::filesystem::path Expand(std::filesystem::path Input){
            const char* Home = getenv("HOME");
            if(!Home)
                throw std::invalid_argument("HOME environment variable not set.");
            std::string Result = Input.c_str();
            if(Result.length() > 0 && Result[0] == '~'){
                Result = std::string(Home) + Result.substr(1, Result.size() - 1);
                return std::filesystem::path(Result);
            }
            return Input;
        }

        bool IsCallbackRegistered(const Event &Event){
            return (Callbacks.find(Event) != Callbacks.end());
        }

        void RunCallback(const Event &Event, const std::string &CurrentDirectory, const std::string &Filename){
            if(IsCallbackRegistered(Event))
                Callbacks[Event](EventInformation{Event, std::filesystem::path(CurrentDirectory + "/" + Filename)});
        }
};
