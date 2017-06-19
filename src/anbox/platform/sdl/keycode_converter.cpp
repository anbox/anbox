/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#include "anbox/platform/sdl/keycode_converter.h"
#pragma GCC diagnostic pop

#include <linux/input.h>

namespace anbox {
namespace platform {
namespace sdl {
std::uint16_t KeycodeConverter::convert(const SDL_Scancode &scan_code) {
  for (std::uint16_t n = 0; n < code_map.size(); n++) {
    if (code_map[n] == scan_code) return n;
  }
  return KEY_RESERVED;
}

const std::array<SDL_Scancode, 249> KeycodeConverter::code_map = {{
    SDL_SCANCODE_UNKNOWN,        /*  KEY_RESERVED        0 */
    SDL_SCANCODE_ESCAPE,         /*  KEY_ESC         1 */
    SDL_SCANCODE_1,              /*  KEY_1           2 */
    SDL_SCANCODE_2,              /*  KEY_2           3 */
    SDL_SCANCODE_3,              /*  KEY_3           4 */
    SDL_SCANCODE_4,              /*  KEY_4           5 */
    SDL_SCANCODE_5,              /*  KEY_5           6 */
    SDL_SCANCODE_6,              /*  KEY_6           7 */
    SDL_SCANCODE_7,              /*  KEY_7           8 */
    SDL_SCANCODE_8,              /*  KEY_8           9 */
    SDL_SCANCODE_9,              /*  KEY_9           10 */
    SDL_SCANCODE_0,              /*  KEY_0           11 */
    SDL_SCANCODE_MINUS,          /*  KEY_MINUS       12 */
    SDL_SCANCODE_EQUALS,         /*  KEY_EQUAL       13 */
    SDL_SCANCODE_BACKSPACE,      /*  KEY_BACKSPACE       14 */
    SDL_SCANCODE_TAB,            /*  KEY_TAB         15 */
    SDL_SCANCODE_Q,              /*  KEY_Q           16 */
    SDL_SCANCODE_W,              /*  KEY_W           17 */
    SDL_SCANCODE_E,              /*  KEY_E           18 */
    SDL_SCANCODE_R,              /*  KEY_R           19 */
    SDL_SCANCODE_T,              /*  KEY_T           20 */
    SDL_SCANCODE_Y,              /*  KEY_Y           21 */
    SDL_SCANCODE_U,              /*  KEY_U           22 */
    SDL_SCANCODE_I,              /*  KEY_I           23 */
    SDL_SCANCODE_O,              /*  KEY_O           24 */
    SDL_SCANCODE_P,              /*  KEY_P           25 */
    SDL_SCANCODE_LEFTBRACKET,    /*  KEY_LEFTBRACE       26 */
    SDL_SCANCODE_RIGHTBRACKET,   /*  KEY_RIGHTBRACE      27 */
    SDL_SCANCODE_RETURN,         /*  KEY_ENTER       28 */
    SDL_SCANCODE_LCTRL,          /*  KEY_LEFTCTRL        29 */
    SDL_SCANCODE_A,              /*  KEY_A           30 */
    SDL_SCANCODE_S,              /*  KEY_S           31 */
    SDL_SCANCODE_D,              /*  KEY_D           32 */
    SDL_SCANCODE_F,              /*  KEY_F           33 */
    SDL_SCANCODE_G,              /*  KEY_G           34 */
    SDL_SCANCODE_H,              /*  KEY_H           35 */
    SDL_SCANCODE_J,              /*  KEY_J           36 */
    SDL_SCANCODE_K,              /*  KEY_K           37 */
    SDL_SCANCODE_L,              /*  KEY_L           38 */
    SDL_SCANCODE_SEMICOLON,      /*  KEY_SEMICOLON       39 */
    SDL_SCANCODE_APOSTROPHE,     /*  KEY_APOSTROPHE      40 */
    SDL_SCANCODE_GRAVE,          /*  KEY_GRAVE       41 */
    SDL_SCANCODE_LSHIFT,         /*  KEY_LEFTSHIFT       42 */
    SDL_SCANCODE_BACKSLASH,      /*  KEY_BACKSLASH       43 */
    SDL_SCANCODE_Z,              /*  KEY_Z           44 */
    SDL_SCANCODE_X,              /*  KEY_X           45 */
    SDL_SCANCODE_C,              /*  KEY_C           46 */
    SDL_SCANCODE_V,              /*  KEY_V           47 */
    SDL_SCANCODE_B,              /*  KEY_B           48 */
    SDL_SCANCODE_N,              /*  KEY_N           49 */
    SDL_SCANCODE_M,              /*  KEY_M           50 */
    SDL_SCANCODE_COMMA,          /*  KEY_COMMA       51 */
    SDL_SCANCODE_PERIOD,         /*  KEY_DOT         52 */
    SDL_SCANCODE_SLASH,          /*  KEY_SLASH       53 */
    SDL_SCANCODE_RSHIFT,         /*  KEY_RIGHTSHIFT      54 */
    SDL_SCANCODE_KP_MULTIPLY,    /*  KEY_KPASTERISK      55 */
    SDL_SCANCODE_LALT,           /*  KEY_LEFTALT     56 */
    SDL_SCANCODE_SPACE,          /*  KEY_SPACE       57 */
    SDL_SCANCODE_CAPSLOCK,       /*  KEY_CAPSLOCK        58 */
    SDL_SCANCODE_F1,             /*  KEY_F1          59 */
    SDL_SCANCODE_F2,             /*  KEY_F2          60 */
    SDL_SCANCODE_F3,             /*  KEY_F3          61 */
    SDL_SCANCODE_F4,             /*  KEY_F4          62 */
    SDL_SCANCODE_F5,             /*  KEY_F5          63 */
    SDL_SCANCODE_F6,             /*  KEY_F6          64 */
    SDL_SCANCODE_F7,             /*  KEY_F7          65 */
    SDL_SCANCODE_F8,             /*  KEY_F8          66 */
    SDL_SCANCODE_F9,             /*  KEY_F9          67 */
    SDL_SCANCODE_F10,            /*  KEY_F10         68 */
    SDL_SCANCODE_NUMLOCKCLEAR,   /*  KEY_NUMLOCK     69 */
    SDL_SCANCODE_SCROLLLOCK,     /*  KEY_SCROLLLOCK      70 */
    SDL_SCANCODE_KP_7,           /*  KEY_KP7         71 */
    SDL_SCANCODE_KP_8,           /*  KEY_KP8         72 */
    SDL_SCANCODE_KP_9,           /*  KEY_KP9         73 */
    SDL_SCANCODE_KP_MINUS,       /*  KEY_KPMINUS     74 */
    SDL_SCANCODE_KP_4,           /*  KEY_KP4         75 */
    SDL_SCANCODE_KP_5,           /*  KEY_KP5         76 */
    SDL_SCANCODE_KP_6,           /*  KEY_KP6         77 */
    SDL_SCANCODE_KP_PLUS,        /*  KEY_KPPLUS      78 */
    SDL_SCANCODE_KP_1,           /*  KEY_KP1         79 */
    SDL_SCANCODE_KP_2,           /*  KEY_KP2         80 */
    SDL_SCANCODE_KP_3,           /*  KEY_KP3         81 */
    SDL_SCANCODE_KP_0,           /*  KEY_KP0         82 */
    SDL_SCANCODE_KP_PERIOD,      /*  KEY_KPDOT       83 */
    SDL_SCANCODE_UNKNOWN,        /*  84 */
    SDL_SCANCODE_LANG5,          /*  KEY_ZENKAKUHANKAKU  85 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_102ND       86 */
    SDL_SCANCODE_F11,            /*  KEY_F11         87 */
    SDL_SCANCODE_F12,            /*  KEY_F12         88 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_RO          89 */
    SDL_SCANCODE_LANG3,          /*  KEY_KATAKANA        90 */
    SDL_SCANCODE_LANG4,          /*  KEY_HIRAGANA        91 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_HENKAN      92 */
    SDL_SCANCODE_LANG3,          /*  KEY_KATAKANAHIRAGANA    93 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_MUHENKAN        94 */
    SDL_SCANCODE_KP_COMMA,       /*  KEY_KPJPCOMMA       95 */
    SDL_SCANCODE_KP_ENTER,       /*  KEY_KPENTER     96 */
    SDL_SCANCODE_RCTRL,          /*  KEY_RIGHTCTRL       97 */
    SDL_SCANCODE_KP_DIVIDE,      /*  KEY_KPSLASH     98 */
    SDL_SCANCODE_SYSREQ,         /*  KEY_SYSRQ       99 */
    SDL_SCANCODE_RALT,           /*  KEY_RIGHTALT        100 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_LINEFEED        101 */
    SDL_SCANCODE_HOME,           /*  KEY_HOME        102 */
    SDL_SCANCODE_UP,             /*  KEY_UP          103 */
    SDL_SCANCODE_PAGEUP,         /*  KEY_PAGEUP      104 */
    SDL_SCANCODE_LEFT,           /*  KEY_LEFT        105 */
    SDL_SCANCODE_RIGHT,          /*  KEY_RIGHT       106 */
    SDL_SCANCODE_END,            /*  KEY_END         107 */
    SDL_SCANCODE_DOWN,           /*  KEY_DOWN        108 */
    SDL_SCANCODE_PAGEDOWN,       /*  KEY_PAGEDOWN        109 */
    SDL_SCANCODE_INSERT,         /*  KEY_INSERT      110 */
    SDL_SCANCODE_DELETE,         /*  KEY_DELETE      111 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_MACRO       112 */
    SDL_SCANCODE_MUTE,           /*  KEY_MUTE        113 */
    SDL_SCANCODE_VOLUMEDOWN,     /*  KEY_VOLUMEDOWN      114 */
    SDL_SCANCODE_VOLUMEUP,       /*  KEY_VOLUMEUP        115 */
    SDL_SCANCODE_POWER,          /*  KEY_POWER       116 SC System Power Down */
    SDL_SCANCODE_KP_EQUALS,      /*  KEY_KPEQUAL     117 */
    SDL_SCANCODE_KP_MINUS,       /*  KEY_KPPLUSMINUS     118 */
    SDL_SCANCODE_PAUSE,          /*  KEY_PAUSE       119 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SCALE       120 AL Compiz Scale (Expose) */
    SDL_SCANCODE_KP_COMMA,       /*  KEY_KPCOMMA     121 */
    SDL_SCANCODE_LANG1,          /*  KEY_HANGEUL,KEY_HANGUEL 122 */
    SDL_SCANCODE_LANG2,          /*  KEY_HANJA       123 */
    SDL_SCANCODE_INTERNATIONAL3, /*  KEY_YEN         124 */
    SDL_SCANCODE_LGUI,           /*  KEY_LEFTMETA        125 */
    SDL_SCANCODE_RGUI,           /*  KEY_RIGHTMETA       126 */
    SDL_SCANCODE_APPLICATION,    /*  KEY_COMPOSE     127 */
    SDL_SCANCODE_STOP,           /*  KEY_STOP        128 AC Stop */
    SDL_SCANCODE_AGAIN,          /*  KEY_AGAIN       129 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PROPS       130 AC Properties */
    SDL_SCANCODE_UNDO,           /*  KEY_UNDO        131 AC Undo */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_FRONT       132 */
    SDL_SCANCODE_COPY,           /*  KEY_COPY        133 AC Copy */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_OPEN        134 AC Open */
    SDL_SCANCODE_PASTE,          /*  KEY_PASTE       135 AC Paste */
    SDL_SCANCODE_FIND,           /*  KEY_FIND        136 AC Search */
    SDL_SCANCODE_CUT,            /*  KEY_CUT         137 AC Cut */
    SDL_SCANCODE_HELP,           /*  KEY_HELP        138 AL Integrated Help Center */
    SDL_SCANCODE_MENU,           /*  KEY_MENU        139 Menu (show menu) */
    SDL_SCANCODE_CALCULATOR,     /*  KEY_CALC        140 AL Calculator */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SETUP       141 */
    SDL_SCANCODE_SLEEP,          /*  KEY_SLEEP       142 SC System Sleep */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_WAKEUP      143 System Wake Up */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_FILE        144 AL Local Machine Browser */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SENDFILE        145 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_DELETEFILE      146 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_XFER        147 */
    SDL_SCANCODE_APP1,           /*  KEY_PROG1       148 */
    SDL_SCANCODE_APP1,           /*  KEY_PROG2       149 */
    SDL_SCANCODE_WWW,            /*  KEY_WWW         150 AL Internet Browser */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_MSDOS       151 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_COFFEE,KEY_SCREENLOCK      152 AL Terminal
                                Lock/Screensaver */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_DIRECTION       153 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CYCLEWINDOWS    154 */
    SDL_SCANCODE_MAIL,           /*  KEY_MAIL        155 */
    SDL_SCANCODE_AC_BOOKMARKS,   /*  KEY_BOOKMARKS       156 AC Bookmarks */
    SDL_SCANCODE_COMPUTER,       /*  KEY_COMPUTER        157 */
    SDL_SCANCODE_AC_BACK,        /*  KEY_BACK        158 AC Back */
    SDL_SCANCODE_AC_FORWARD,     /*  KEY_FORWARD     159 AC Forward */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CLOSECD     160 */
    SDL_SCANCODE_EJECT,          /*  KEY_EJECTCD     161 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_EJECTCLOSECD    162 */
    SDL_SCANCODE_AUDIONEXT,      /*  KEY_NEXTSONG        163 */
    SDL_SCANCODE_AUDIOPLAY,      /*  KEY_PLAYPAUSE       164 */
    SDL_SCANCODE_AUDIOPREV,      /*  KEY_PREVIOUSSONG    165 */
    SDL_SCANCODE_AUDIOSTOP,      /*  KEY_STOPCD      166 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_RECORD      167 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_REWIND      168 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PHONE       169 Media Select Telephone */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_ISO         170 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CONFIG      171 AL Consumer Control
                                  Configuration */
    SDL_SCANCODE_AC_HOME,        /*  KEY_HOMEPAGE        172 AC Home */
    SDL_SCANCODE_AC_REFRESH,     /*  KEY_REFRESH     173 AC Refresh */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_EXIT        174 AC Exit */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_MOVE        175 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_EDIT        176 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SCROLLUP        177 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SCROLLDOWN      178 */
    SDL_SCANCODE_KP_LEFTPAREN,   /*  KEY_KPLEFTPAREN     179 */
    SDL_SCANCODE_KP_RIGHTPAREN,  /*  KEY_KPRIGHTPAREN    180 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_NEW         181 AC New */
    SDL_SCANCODE_AGAIN,          /*  KEY_REDO        182 AC Redo/Repeat */
    SDL_SCANCODE_F13,            /*  KEY_F13         183 */
    SDL_SCANCODE_F14,            /*  KEY_F14         184 */
    SDL_SCANCODE_F15,            /*  KEY_F15         185 */
    SDL_SCANCODE_F16,            /*  KEY_F16         186 */
    SDL_SCANCODE_F17,            /*  KEY_F17         187 */
    SDL_SCANCODE_F18,            /*  KEY_F18         188 */
    SDL_SCANCODE_F19,            /*  KEY_F19         189 */
    SDL_SCANCODE_F20,            /*  KEY_F20         190 */
    SDL_SCANCODE_F21,            /*  KEY_F21         191 */
    SDL_SCANCODE_F22,            /*  KEY_F22         192 */
    SDL_SCANCODE_F23,            /*  KEY_F23         193 */
    SDL_SCANCODE_F24,            /*  KEY_F24         194 */
    SDL_SCANCODE_UNKNOWN,        /*  195 */
    SDL_SCANCODE_UNKNOWN,        /*  196 */
    SDL_SCANCODE_UNKNOWN,        /*  197 */
    SDL_SCANCODE_UNKNOWN,        /*  198 */
    SDL_SCANCODE_UNKNOWN,        /*  199 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PLAYCD      200 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PAUSECD     201 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PROG3       202 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PROG4       203 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_DASHBOARD       204 AL Dashboard */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SUSPEND     205 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CLOSE       206 AC Close */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PLAY        207 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_FASTFORWARD     208 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_BASSBOOST       209 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_PRINT       210 AC Print */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_HP          211 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CAMERA      212 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SOUND       213 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_QUESTION        214 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_EMAIL       215 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CHAT        216 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SEARCH      217 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CONNECT     218 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_FINANCE     219 AL Checkbook/Finance */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SPORT       220 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SHOP        221 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_ALTERASE        222 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_CANCEL      223 AC Cancel */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_BRIGHTNESSDOWN  224 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_BRIGHTNESSUP    225 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_MEDIA       226 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SWITCHVIDEOMODE 227 Cycle between available
                             video outputs (Monitor/LCD/TV-out/etc) */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_KBDILLUMTOGGLE  228 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_KBDILLUMDOWN    229 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_KBDILLUMUP      230 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SEND        231 AC Send */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_REPLY       232 AC Reply */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_FORWARDMAIL     233 AC Forward Msg */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_SAVE        234 AC Save */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_DOCUMENTS       235 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_BATTERY     236  */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_BLUETOOTH       237 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_WLAN        238 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_UWB         239 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_UNKNOWN     240 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_VIDEO_NEXT      241 drive next video source */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_VIDEO_PREV      242 drive previous video
                             source */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_BRIGHTNESS_CYCLE    243 brightness up, after
                             max is min */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_BRIGHTNESS_ZERO 244 brightness off, use
                             ambient */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_DISPLAY_OFF     245 display device to off
                             state */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_WIMAX       246 */
    SDL_SCANCODE_UNKNOWN,        /*  KEY_RFKILL      247 Key that controls all radios
                             */
    SDL_SCANCODE_UNKNOWN         /*  KEY_MICMUTE     248 Mute / unmute the microphone */
}};
} // namespace sdl
} // namespace platform
} // namespace anbox
