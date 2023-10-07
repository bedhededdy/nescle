/*
 * Copyright 2023 Edward C. Pinkston
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <SDL_events.h>

#include <functional>
#include <queue>
#include <set>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace NESCLE {
// TODO: MAYBE I SHOULD HAVE MY OWN EVENT TYPE SO THAT I CAN
//       HANDLE MY OWN EVENTS
class EventManager {
private:
    struct Subscriber {
        std::string key;
        // FIXME: See if this can handle something more generic using a template
        //        If you have to, you can use a void*
        // NOTE:  ANOTHER APPROACH IS HAVING A MAP OF EVENTTYPS TO CALLBACKS HERE
        std::function<int(SDL_Event&)> callback;
    };

    // NOTE: WE CAN EVEN USE THIS TO HAVE PEOPLE SUBSCRIBE TO THE QUIT EVENT
    //       TO ENSURE THAT ALL STATIC CLASSES FREE THEIR MEMORY (ALTHOUGH,
    //       ANY CLASSES WITH A STD::VECTOR WILL STILL HOLD ONTO THEIR MEMORY)
    // Using unordered_map/set allows our framework to handle a large number
    // of events and subscriptions. We do expect a relatively low number of
    // event subscriptions though, so we could potentially actually optimize
    // by using the ordered ones as they use a tree structure in exchange
    // for O(lg n) lookup time instead of O(1) lookup time
    std::unordered_map<SDL_EventType, std::vector<Subscriber>> events_to_subscribers;
    std::unordered_map<std::string, std::unordered_set<SDL_EventType>> key_event_index;

    std::queue<SDL_Event> event_queue;

    // We need the event to create a copy here
    // TODO: MAKE SURE THIS HOLDS NO POINTERS OR WE ARE FUCKED
    void Publish(SDL_Event event);

public:
    void Init();
    bool Subscribe(SDL_EventType event_type, std::string key, std::function<int(SDL_Event&)> callback);
    bool Unsubscribe(SDL_EventType event_type, std::string key);
    void ProcessEvents();
    void Quit();
};
}
