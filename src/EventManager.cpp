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
#include "EventManager.h"

namespace NESCLE {
void EventManager::Init() {
    // Init events map by making an empty vector for each event
}

void EventManager::Publish(SDL_Event event) {
    event_queue.push(event);
}

bool EventManager::Subscribe(SDL_EventType event_type, std::string key, std::function<int(SDL_Event&)> callback) {
    if (key_event_index.find(key) == key_event_index.end()) {
        std::unordered_set<SDL_EventType> set;
        key_event_index[key] = set;
        key_event_index[key].insert(event_type);
        events_to_subscribers[event_type].push_back({key, callback});
        return true;
    }
    if (key_event_index[key].find(event_type) == key_event_index[key].end()) {
        key_event_index[key].insert(event_type);
        events_to_subscribers[event_type].push_back({key, callback});
        return true;
    }
    return false;
}

bool EventManager::Unsubscribe(SDL_EventType event_type, std::string key) {
    if (key_event_index.find(key) != key_event_index.end()
        && key_event_index[key].find(event_type) != key_event_index[key].end()) {

        key_event_index[key].erase(event_type);

        // We will eat the time complexity here to avoid maintaining another index of key to subscriber
        // as we do not anticipate many unsubscriptions
        events_to_subscribers[event_type].erase(std::remove_if(events_to_subscribers[event_type].begin(), events_to_subscribers[event_type].end(), [key](Subscriber& sub) {
            return sub.key == key;
        }), events_to_subscribers[event_type].end());
        return true;
    }
    return false;
}

void EventManager::ProcessEvents() {
    // Process SDL events
    // Publish them to subscribers
    // Run subscriber callbacks
}

void EventManager::Quit() {

}
}
