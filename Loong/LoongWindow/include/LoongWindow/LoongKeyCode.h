//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

namespace Loong::Window {

enum class LoongKeyCode {
    // clang-format off
    kKeySpace                 = 32,
    kKeyApostrophe            = 39,  /* ' */
    kKeyComma                 = 44,  /* , */
    kKeyMinus                 = 45,  /* - */
    kKeyPeriod                = 46,  /* . */
    kKeySlash                 = 47,  /* / */
    kKey0                     = 48,
    kKey1                     = 49,
    kKey2                     = 50,
    kKey3                     = 51,
    kKey4                     = 52,
    kKey5                     = 53,
    kKey6                     = 54,
    kKey7                     = 55,
    kKey8                     = 56,
    kKey9                     = 57,
    kKeySemicolon             = 59,  /* ; */
    kKeyEqual                 = 61,  /* = */
    kKeyA                     = 65,
    kKeyB                     = 66,
    kKeyC                     = 67,
    kKeyD                     = 68,
    kKeyE                     = 69,
    kKeyF                     = 70,
    kKeyG                     = 71,
    kKeyH                     = 72,
    kKeyI                     = 73,
    kKeyJ                     = 74,
    kKeyK                     = 75,
    kKeyL                     = 76,
    kKeyM                     = 77,
    kKeyN                     = 78,
    kKeyO                     = 79,
    kKeyP                     = 80,
    kKeyQ                     = 81,
    kKeyR                     = 82,
    kKeyS                     = 83,
    kKeyT                     = 84,
    kKeyU                     = 85,
    kKeyV                     = 86,
    kKeyW                     = 87,
    kKeyX                     = 88,
    kKeyY                     = 89,
    kKeyZ                     = 90,
    kKeyLeftBracket           = 91,  /* [ */
    kKeyBackslash             = 92,  /* \ */
    kKeyRightBracket          = 93,  /* ] */
    kKeyGraveAccent           = 96,  /* ` */
    kKeyWorld1                = 161, /* non-US #1 */
    kKeyWorld2                = 162, /* non-US #2 */

    /* Function keys */
    kKeyEscape                = 256,
    kKeyEnter                 = 257,
    kKeyTab                   = 258,
    kKeyBackspace             = 259,
    kKeyInsert                = 260,
    kKeyDelete                = 261,
    kKeyRight                 = 262,
    kKeyLeft                  = 263,
    kKeyDown                  = 264,
    kKeyUp                    = 265,
    kKeyPageUp                = 266,
    kKeyPageDown              = 267,
    kKeyHome                  = 268,
    kKeyEnd                   = 269,
    kKeyCapsLock              = 280,
    kKeyScrollLock            = 281,
    kKeyNumLock               = 282,
    kKeyPrintScreen           = 283,
    kKeyPause                 = 284,
    kKeyF1                    = 290,
    kKeyF2                    = 291,
    kKeyF3                    = 292,
    kKeyF4                    = 293,
    kKeyF5                    = 294,
    kKeyF6                    = 295,
    kKeyF7                    = 296,
    kKeyF8                    = 297,
    kKeyF9                    = 298,
    kKeyF10                   = 299,
    kKeyF11                   = 300,
    kKeyF12                   = 301,
    kKeyF13                   = 302,
    kKeyF14                   = 303,
    kKeyF15                   = 304,
    kKeyF16                   = 305,
    kKeyF17                   = 306,
    kKeyF18                   = 307,
    kKeyF19                   = 308,
    kKeyF20                   = 309,
    kKeyF21                   = 310,
    kKeyF22                   = 311,
    kKeyF23                   = 312,
    kKeyF24                   = 313,
    kKeyF25                   = 314,
    kKeyKp0                   = 320,
    kKeyKp1                   = 321,
    kKeyKp2                   = 322,
    kKeyKp3                   = 323,
    kKeyKp4                   = 324,
    kKeyKp5                   = 325,
    kKeyKp6                   = 326,
    kKeyKp7                   = 327,
    kKeyKp8                   = 328,
    kKeyKp9                   = 329,
    kKeyKpDecimal             = 330,
    kKeyKpDivide              = 331,
    kKeyKpMultiply            = 332,
    kKeyKpSubtract            = 333,
    kKeyKpAdd                 = 334,
    kKeyKpEnter               = 335,
    kKeyKpEqual               = 336,
    kKeyLeftShift             = 340,
    kKeyLeftControl           = 341,
    kKeyLeftAlt               = 342,
    kKeyLeftSuper             = 343,
    kKeyRightShift            = 344,
    kKeyRightControl          = 345,
    kKeyRightAlt              = 346,
    kKeyRightSuper            = 347,
    kKeyMenu                  = 348,
    // clang-format on
    kKeyCode_Count
};

}