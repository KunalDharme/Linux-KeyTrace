// keymap.c
#include <stdio.h>
#include <linux/input-event-codes.h>
#include <string.h>
#include <ctype.h>

// Return 1 if the key is a shift key
int is_shift_key(int code) {
    return code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT;
}

// Map keycodes to strings (expanded map)
const char* keycode_to_string(int code, int shift, int caps_lock) {
    static char buffer[16]; // Buffer to hold the string representation of the key

    char c = '\0'; // Initialize character for letters

    switch (code) {
        // Letters
        case KEY_Q: c = 'q'; break; case KEY_W: c = 'w'; break;
        case KEY_E: c = 'e'; break; case KEY_R: c = 'r'; break;
        case KEY_T: c = 't'; break; case KEY_Y: c = 'y'; break;
        case KEY_U: c = 'u'; break; case KEY_I: c = 'i'; break;
        case KEY_O: c = 'o'; break; case KEY_P: c = 'p'; break;
        case KEY_A: c = 'a'; break; case KEY_S: c = 's'; break;
        case KEY_D: c = 'd'; break; case KEY_F: c = 'f'; break;
        case KEY_G: c = 'g'; break; case KEY_H: c = 'h'; break;
        case KEY_J: c = 'j'; break; case KEY_K: c = 'k'; break;
        case KEY_L: c = 'l'; break; case KEY_Z: c = 'z'; break;
        case KEY_X: c = 'x'; break; case KEY_C: c = 'c'; break;
        case KEY_V: c = 'v'; break; case KEY_B: c = 'b'; break;
        case KEY_N: c = 'n'; break; case KEY_M: c = 'm'; break;

        // Numbers and their shifted symbols (1-0)
        case KEY_1: snprintf(buffer, sizeof(buffer), "%s", shift ? "!" : "1"); return buffer;
        case KEY_2: snprintf(buffer, sizeof(buffer), "%s", shift ? "@" : "2"); return buffer;
        case KEY_3: snprintf(buffer, sizeof(buffer), "%s", shift ? "#" : "3"); return buffer;
        case KEY_4: snprintf(buffer, sizeof(buffer), "%s", shift ? "$" : "4"); return buffer;
        case KEY_5: snprintf(buffer, sizeof(buffer), "%s", shift ? "%%" : "5"); return buffer; // Escape %
        case KEY_6: snprintf(buffer, sizeof(buffer), "%s", shift ? "^" : "6"); return buffer;
        case KEY_7: snprintf(buffer, sizeof(buffer), "%s", shift ? "&" : "7"); return buffer;
        case KEY_8: snprintf(buffer, sizeof(buffer), "%s", shift ? "*" : "8"); return buffer;
        case KEY_9: snprintf(buffer, sizeof(buffer), "%s", shift ? "(" : "9"); return buffer;
        case KEY_0: snprintf(buffer, sizeof(buffer), "%s", shift ? ")" : "0"); return buffer;

        // Other common keys, punctuation, and symbols
        case KEY_SPACE:     return " ";
        case KEY_ENTER:     return "<ENTER>";
        case KEY_BACKSPACE: return "<BACKSPACE>";
        case KEY_TAB:       return "<TAB>";
        case KEY_ESC:       return "<ESC>";
        case KEY_LEFTCTRL:  return "<L_CTRL>";
        case KEY_RIGHTCTRL: return "<R_CTRL>";
        case KEY_LEFTALT:   return "<L_ALT>";
        case KEY_RIGHTALT:  return "<R_ALT>";
        case KEY_LEFTSHIFT: return "<L_SHIFT>";
        case KEY_RIGHTSHIFT:return "<R_SHIFT>";
        case KEY_CAPSLOCK:  return "<CAPS_LOCK>"; // Logs the event, not a character

        // Punctuation and Symbols
        case KEY_SEMICOLON: snprintf(buffer, sizeof(buffer), "%s", shift ? ":" : ";"); return buffer;
        case KEY_APOSTROPHE:snprintf(buffer, sizeof(buffer), "%s", shift ? "\"" : "'"); return buffer;
        case KEY_GRAVE:     snprintf(buffer, sizeof(buffer), "%s", shift ? "~" : "`"); return buffer; // Backtick / Tilde
        case KEY_MINUS:     snprintf(buffer, sizeof(buffer), "%s", shift ? "_" : "-"); return buffer;
        case KEY_EQUAL:     snprintf(buffer, sizeof(buffer), "%s", shift ? "+" : "="); return buffer;
        case KEY_LEFTBRACE: snprintf(buffer, sizeof(buffer), "%s", shift ? "{" : "["); return buffer;
        case KEY_RIGHTBRACE:snprintf(buffer, sizeof(buffer), "%s", shift ? "}" : "]"); return buffer;
        case KEY_BACKSLASH: snprintf(buffer, sizeof(buffer), "%s", shift ? "|" : "\\"); return buffer;
        case KEY_COMMA:     snprintf(buffer, sizeof(buffer), "%s", shift ? "<" : ","); return buffer;
        case KEY_DOT:       snprintf(buffer, sizeof(buffer), "%s", shift ? ">" : "."); return buffer;
        case KEY_SLASH:     snprintf(buffer, sizeof(buffer), "%s", shift ? "?" : "/"); return buffer;

        // Function Keys
        case KEY_F1:  return "<F1>"; case KEY_F2:  return "<F2>";
        case KEY_F3:  return "<F3>"; case KEY_F4:  return "<F4>";
        case KEY_F5:  return "<F5>"; case KEY_F6:  return "<F6>";
        case KEY_F7:  return "<F7>"; case KEY_F8:  return "<F8>";
        case KEY_F9:  return "<F9>"; case KEY_F10: return "<F10>";
        case KEY_F11: return "<F11>"; case KEY_F12: return "<F12>";

        // Navigation and Special Keys
        case KEY_HOME:      return "<HOME>";
        case KEY_END:       return "<END>";
        case KEY_PAGEUP:    return "<PAGE_UP>";
        case KEY_PAGEDOWN:  return "<PAGE_DOWN>";
        case KEY_DELETE:    return "<DELETE>";
        case KEY_INSERT:    return "<INSERT>";
        case KEY_UP:        return "<UP_ARROW>";
        case KEY_DOWN:      return "<DOWN_ARROW>";
        case KEY_LEFT:      return "<LEFT_ARROW>";
        case KEY_RIGHT:     return "<RIGHT_ARROW>";
        case KEY_SYSRQ:     return "<PRINTSCREEN>";
        case KEY_PAUSE:     return "<PAUSE>";
        case KEY_SCROLLLOCK:return "<SCROLL_LOCK>";
        case KEY_NUMLOCK:   return "<NUM_LOCK>";

        // Numpad keys (when Num Lock is on, these usually output digits directly)
        case KEY_KPASTERISK: return "*";
        case KEY_KPMINUS: return "-";
        case KEY_KPPLUS: return "+";
        case KEY_KPDOT: return ".";
        case KEY_KPSLASH: return "/";
        case KEY_KPENTER: return "<KP_ENTER>";
        case KEY_KP0: return "0"; case KEY_KP1: return "1";
        case KEY_KP2: return "2"; case KEY_KP3: return "3";
        case KEY_KP4: return "4"; case KEY_KP5: return "5";
        case KEY_KP6: return "6"; case KEY_KP7: return "7";
        case KEY_KP8: return "8"; case KEY_KP9: return "9";

        default:
            // For unhandled keys, return their hex code
            snprintf(buffer, sizeof(buffer), "<0x%02x>", code);
            return buffer;
    }

    // If 'c' was set (it's a letter key)
    if (c != '\0') {
        if ((caps_lock && !shift) || (!caps_lock && shift)) {
            c = toupper(c);
        }
        snprintf(buffer, sizeof(buffer), "%c", c);
        return buffer;
    }

    // Should theoretically not be reached with a comprehensive switch, but defensive
    return "";
}
