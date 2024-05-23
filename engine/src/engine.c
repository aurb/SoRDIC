/*  Software Rendered Demo Engine In C
    Copyright (C) 2024 https://github.com/aurb

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "engine.h"
#include "v_renderer.h"

typedef struct {
    INT timestamp;
    KEY_CODE code;
} KEY_PRESS;

//Array storing currently pressed-down keys
KEY_PRESS key_presses[10];
//Array storing events to emit on each call to engine_poll_events
EVENT events[10];
INT events_count = 0; //number of used cells in "events"

//Internal functions forward declarations
void init_keyboard_handler();
void add_key_press(KEY_CODE code, INT timestamp);
INT erase_key_press(KEY_CODE code);
void add_quit_request_event(INT timestamp);
void add_key_press_event(KEY_CODE code, INT timestamp);
void add_key_release_event(KEY_CODE code, INT timestamp);
void add_key_hold_events(INT timestamp);
void finalize_events();
KEY_CODE decode_key_sym(SDL_Keycode keycode);

//Public functions
INT engine_init(INT window_width, INT window_height, INT window_flags, const char *window_name) {
    /** Display render buffer has Z buffer enabled by default. */
    display_init(window_width, window_height, window_flags, window_name);
    init_keyboard_handler();
    /** Init 3d rendering */
    vr_init();
    return 0;
}

INT engine_cleanup() {
    vr_cleanup();
    display_cleanup();
    return 0;
}

EVENT* engine_poll_events() {
    SDL_Event sdl_event;
    KEY_CODE key_code = K_UNSUPPORTED;
    INT timestamp = SDL_GetTicks();
    events_count = 0;

    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type)
        {
            case SDL_QUIT:
                add_quit_request_event(timestamp); //quit requested
                break;
            case SDL_KEYDOWN:
                key_code = decode_key_sym(sdl_event.key.keysym.sym);
                if (key_code != K_UNSUPPORTED && sdl_event.key.repeat == 0) {
                    if (key_code == K_ESCAPE)
                        add_quit_request_event(timestamp);
                    else
                        add_key_press_event(key_code, timestamp);
                }
                break;
            case SDL_KEYUP:
                key_code = decode_key_sym(sdl_event.key.keysym.sym);
                if (key_code != K_UNSUPPORTED) {
                    add_key_release_event(key_code, timestamp);
                }
                break;
        }
    }
    add_key_hold_events(timestamp);
    finalize_events();

    return events;
}

//Internal functions
void init_keyboard_handler() {
    for (INT i=0; i<10; i++) {
        key_presses[i].timestamp = -1;
        key_presses[i].code = K_UNSUPPORTED;
        events[i].type = NO_EVENT;
        events[i].timestamp = -1;
        events[i].hold_time = -1.;
        events[i].code = K_UNSUPPORTED;
    }
    events_count = 0;
}

void add_key_press(KEY_CODE code, INT timestamp) {
    for (INT i = 0; i < 10; i++) {
        if (key_presses[i].code == K_UNSUPPORTED) {
            key_presses[i].code = code;
            key_presses[i].timestamp = timestamp;
            return;
        }
    }
}

//If key press for code was recorded, erase it and return time when
//that key press occured
INT erase_key_press(KEY_CODE code) {
    INT ret = -1;//-1: error: no key press for code was found
    for (INT i = 0; i < 10; i++) {
        if (key_presses[i].code == code) {
            ret = key_presses[i].timestamp;
            key_presses[i].code = K_UNSUPPORTED;
            key_presses[i].timestamp = -1;
            break;
        }
    }
    return ret;
}

void add_quit_request_event(INT timestamp) {
    events[events_count].type = QUIT_REQUEST;
    events[events_count].timestamp = timestamp;
    events_count++;
}

void add_key_press_event(KEY_CODE code, INT timestamp) {
    add_key_press(code, timestamp);
    events[events_count].type = KEY_PRESSED;
    events[events_count].code = code;
    events[events_count].timestamp = timestamp;
    events[events_count].hold_time = 0;
    events_count++;
}

void add_key_release_event(KEY_CODE code, INT timestamp) {
    INT key_press_time = erase_key_press(code);
    events[events_count].type = KEY_RELEASED;
    events[events_count].code = code;
    events[events_count].timestamp = timestamp;
    events[events_count].hold_time = (FLOAT)(timestamp - key_press_time)/1000.;
    events_count++;
}

void add_key_hold_events(INT timestamp) {
    for (INT i = 0; i < 10; i++) {
        if (key_presses[i].code != K_UNSUPPORTED &&
            key_presses[i].timestamp < timestamp) {

            events[events_count].code = key_presses[i].code;
            events[events_count].type = KEY_HOLD;
            events[events_count].timestamp = key_presses[i].timestamp;
            events[events_count].hold_time = (FLOAT)(timestamp - key_presses[i].timestamp)/1000.;
            events_count++;
        }
    }
}

void finalize_events() {
    //add sentinel event to the end of the events array
    events[events_count].type = NO_EVENT;
    events[events_count].code = K_UNSUPPORTED;
    events[events_count].timestamp = -1;
    events[events_count].hold_time = -1.;
}

KEY_CODE decode_key_sym(SDL_Keycode keycode) {
    if (keycode >= K_MIN_PRINTABLE && keycode <= K_MAX_PRINTABLE) return keycode;
    switch (keycode) {
        case SDLK_RETURN: return K_RETURN;
        case SDLK_ESCAPE: return K_ESCAPE;
        case SDLK_BACKSPACE: return K_BACKSPACE;
        case SDLK_TAB: return K_TAB;
        case SDLK_CAPSLOCK: return K_CAPSLOCK;
        case SDLK_F1: return K_F1;
        case SDLK_F2: return K_F2;
        case SDLK_F3: return K_F3;
        case SDLK_F4: return K_F4;
        case SDLK_F5: return K_F5;
        case SDLK_F6: return K_F6;
        case SDLK_F7: return K_F7;
        case SDLK_F8: return K_F8;
        case SDLK_F9: return K_F9;
        case SDLK_F10: return K_F10;
        case SDLK_F11: return K_F11;
        case SDLK_F12: return K_F12;
        case SDLK_PRINTSCREEN: return K_PRINTSCREEN;
        case SDLK_SCROLLLOCK: return K_SCROLLLOCK;
        case SDLK_PAUSE: return K_PAUSE;
        case SDLK_INSERT: return K_INSERT;
        case SDLK_HOME: return K_HOME;
        case SDLK_PAGEUP: return K_PAGEUP;
        case SDLK_DELETE: return K_DELETE;
        case SDLK_END: return K_END;
        case SDLK_PAGEDOWN: return K_PAGEDOWN;
        case SDLK_RIGHT: return K_RIGHT;
        case SDLK_LEFT: return K_LEFT;
        case SDLK_DOWN: return K_DOWN;
        case SDLK_UP: return K_UP;
        case SDLK_NUMLOCKCLEAR: return K_NUMLOCKCLEAR;
        case SDLK_KP_DIVIDE: return K_KP_DIVIDE;
        case SDLK_KP_MULTIPLY: return K_KP_MULTIPLY;
        case SDLK_KP_MINUS: return K_KP_MINUS;
        case SDLK_KP_PLUS: return K_KP_PLUS;
        case SDLK_KP_ENTER: return K_KP_ENTER;
        case SDLK_KP_1: return K_KP_1;
        case SDLK_KP_2: return K_KP_2;
        case SDLK_KP_3: return K_KP_3;
        case SDLK_KP_4: return K_KP_4;
        case SDLK_KP_5: return K_KP_5;
        case SDLK_KP_6: return K_KP_6;
        case SDLK_KP_7: return K_KP_7;
        case SDLK_KP_8: return K_KP_8;
        case SDLK_KP_9: return K_KP_9;
        case SDLK_KP_0: return K_KP_0;
        case SDLK_KP_PERIOD: return K_KP_PERIOD;
        case SDLK_KP_COMMA: return K_KP_COMMA;
        case SDLK_MUTE: return K_MUTE;
        case SDLK_VOLUMEUP: return K_VOLUMEUP;
        case SDLK_VOLUMEDOWN: return K_VOLUMEDOWN;
        case SDLK_LCTRL: return K_LCTRL;
        case SDLK_LSHIFT: return K_LSHIFT;
        case SDLK_LALT: return K_LALT;
        case SDLK_RCTRL: return K_RCTRL;
        case SDLK_RSHIFT: return K_RSHIFT;
        case SDLK_RALT: return K_RALT;
        default: return K_UNSUPPORTED;
    }
}