// a collection of premade, ready-to-use functions for the hooks in gstream

// if you make a good, not too specialised one yourself, don't hesitate
// to send it to me so it can be shared with others. If you don't know
// whether the function is in the category "good, not too specialised"
// send it anyway and I'll judge it :-)

#include "gstream"
#include <cctype> // isdigit, isspace and such

using namespace std;

// -
// input approvers


// for inputting integers; note that allow x means allow _only_ x
bool ia_allow_integer(int c)
{
 return isdigit(c);
}

// for inputting decimal numbers
bool ia_allow_decimal(int c)
{
 return (isdigit(c) || c == '.'); // add || c == ',' if you're international
}

// for inputting hexadecimal numbers
bool ia_allow_hexadigits(int c)
{
 return isxdigit(c);
}

// don't be choosy about the character
bool ia_allow_everything(int c)
{
 return true;
}

// don't allow any whitespace (or blackspace, depending on your background colour)
bool ia_block_spaces(int c)
{
 return (!isspace(c));
}

// allow chars that can make up a word (note '_' is included)
bool ia_allow_word_chars(int c)
{
 if (c <= 47 || (c >= 58 && c <= 64) || (c >= 91 && c <= 94) || c == 96 ||
     (c >= 124 && c <= 180) || (c >= 182 && c <= 191) || c == 215 || c == 247)
   return false; // not a character that can make up a word
 else
   return true;
}


// -
// cursor drawers (if you have always thought that cd was short for compact
// disc, you're obviously wrong :-). The preferred dimensions can be used
// directly with a call to set_cursor_dimensions


/* mixture of a win cursor and console cursor
   - the insert cursor looks like a win cursor (a vertical line: | ), whereas
     the overwrite cursor is like the console overwrite cursor: it covers the
     letter
   - preferred dimensions:
       (1, gbuf::DYNAMIC_CURSOR_SIZE,
        gbuf::DYNAMIC_CURSOR_SIZE, gbuf::DYNAMIC_CURSOR_SIZE)
     but the first argument can be bigger for a broader cursor
*/
void cd_winconsole(BITMAP *bmp, int x, int y, int w, int h, int clr, bool ins)
{
 rectfill(bmp, x, y, x + w - 1, y + h - 1, clr == -1 ? makecol_depth(bitmap_color_depth(bmp),0,0,0) : clr);
}

/* a normal console cursor
   - the insert cursor is like an _, and the overwrite cursor covers the letter
   - preferred dimensions:
       (gbuf::DYNAMIC_CURSOR_SIZE, gbuf::DYNAMIC_CURSOR_SIZE,
        gbuf::DYNAMIC_CURSOR_SIZE, gbuf::DYNAMIC_CURSOR_SIZE)
     the second argument can be set manually to control where the _ will be
     placed - low numbers and the cursor will fly above the letters, high
     numbers and the cursor will float at the bottom
*/
void cd_console(BITMAP *bmp, int x, int y, int w, int h, int clr, bool ins)
{
 if (ins)
   rectfill(bmp, x, y + h - 2, x + w - 1, y + h - 1, clr == -1 ? makecol_depth(bitmap_color_depth(bmp),0,0,0) : clr);
 else
   rectfill(bmp, x, y, x + w - 1, y + h - 1, clr == -1 ? makecol_depth(bitmap_color_depth(bmp),0,0,0) : clr);
}


// -
// input error handlers
// note that doing nothing often is the best thing to do


// basic, default handler - the best in many situations :-)
void ieh_never_complain(int e)
{
}

// fancy handler that flashes the keyboard leds
void ieh_led_flasher(int e)
{
 switch (e)
   {
    case gbuf::IE_UNRECOGNIZED_KEY:
    case gbuf::IE_CURSOR_EXCEEDING_LIMITS:
         set_leds(KB_SCROLOCK_FLAG | KB_NUMLOCK_FLAG | KB_CAPSLOCK_FLAG);
         rest(80); // instead of rest, a one-shot timer would probably be better
         set_leds(-1);
         break;
   }
}

