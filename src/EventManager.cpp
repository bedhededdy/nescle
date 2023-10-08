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

#include <imgui_impl_sdl2.h>

#include "Emulator.h"

namespace NESCLE {
std::unordered_map<SDL_EventType, std::vector<EventManager::Subscriber>> EventManager::events_to_subscribers;
std::unordered_map<std::string, std::unordered_set<SDL_EventType>> EventManager::key_event_index;
std::queue<SDL_Event> EventManager::event_queue;

void EventManager::Init() {
    // Init events map by making an empty vector for each event
    events_to_subscribers.insert({SDL_QUIT, std::vector<Subscriber>()});
    events_to_subscribers.insert({SDL_KEYDOWN, std::vector<Subscriber>()});
    events_to_subscribers.insert({SDL_WINDOWEVENT, std::vector<Subscriber>()});
    events_to_subscribers.insert({SDL_JOYBUTTONDOWN, std::vector<Subscriber>()});
    events_to_subscribers.insert({SDL_JOYDEVICEADDED, std::vector<Subscriber>()});
    events_to_subscribers.insert({SDL_JOYDEVICEREMOVED, std::vector<Subscriber>()});
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

/**
 * @brief Polls and processes all events, including updating emu KB state
 *
 */
void EventManager::ProcessEvents(Emulator& emu) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        Publish(event);
    }

    // So KeyHeld doesn't depend on the order in which keys were received
    emu.RefreshKeyboardState();

    // In case anybody else decides to publish events in the future we will
    // maintain the event queue, push all SDL events to it, and then process,
    // it out here
    while (!event_queue.empty()) {
        SDL_Event& curr_event = event_queue.front();
        event_queue.pop();
        auto event_type = static_cast<SDL_EventType>(curr_event.type);
        if (events_to_subscribers.find(event_type) != events_to_subscribers.end()) {
            for (Subscriber& sub : events_to_subscribers[event_type]) {
                // For now ignore the return value, but in the future we can maybe
                // pass it back to the subscriber so that they know what the
                // problem was
                sub.callback(curr_event);
            }
        }
    }
}

void EventManager::Quit() {
    while (!event_queue.empty())
        event_queue.pop();
    events_to_subscribers.clear();
    key_event_index.clear();
}
}
