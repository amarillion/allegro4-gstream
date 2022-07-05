// gbuf's interface implementation

#include "gstream"
#include <string.h>

using namespace std;

// hack to make the library work with MSVC
#ifdef ALLEGRO_MSVC 
  int const gbuf::DYNAMIC_CURSOR_SIZE = -1;
  int const gbuf::NO_COLOR_CHANGE = -2;
  int const gbuf::INSERT_STRING_START_SIZE = 128;
#endif

void gbuf::reset()
{
  gs_x = 0;
  gs_y = 0;
  set_dirty_rectangle_marker(0);
  set_font(font);
  gs_color = -1;
  gs_margin = 0;
  gs_tab_size = 8;
  gs_max_input_length = -1;
  remove_all_tabs();
  gs_insert = true;
  set_input_approver(ia_allow_everything);
  set_cursor_drawer(cd_winconsole);
  set_cursor_dimensions(1, DYNAMIC_CURSOR_SIZE, DYNAMIC_CURSOR_SIZE, DYNAMIC_CURSOR_SIZE);
  set_cursor_blink(0);
  set_input_error_handler(ieh_never_complain);
}

void gbuf::remove_all_tabs()
{
  gs_tab_list.clear();
  gs_tab_list.push_back(tab(-1,NO_COLOR_CHANGE, 0)); // insert standard information
}

void gbuf::set_tab(int x, int color, FONT *fnt)
{
  if (x < 0) // x should be above or equal to 0
    return;

  list<tab>::iterator i = gs_tab_list.begin();
  while ((i != gs_tab_list.end()) && (x > i->x)) // find the correct gab between the tabs
    ++i;
  if (x == i->x)
    {
      i->color = color;                      // if x == i->x then just override
      i->fnt = fnt;
    }
  else
    gs_tab_list.insert(i, tab(x, color, fnt)); // else insert new tab
}

void gbuf::remove_tab(int x)
{
  if (x < 0)
    return;
  for (list<tab>::iterator i = gs_tab_list.begin(); i != gs_tab_list.end(); i++)
    if (x == i->x)
      {
	gs_tab_list.erase(i);
	break;
      }
}

void gbuf::restore()
{
  if (gs_tab_list.begin()->color != NO_COLOR_CHANGE)
    gs_color = gs_tab_list.begin()->color;
  if (gs_tab_list.begin()->fnt != 0)
    gs_font = gs_tab_list.begin()->fnt;
}

void gbuf::set_input_string(char const *s)
{
  int len = ustrsizez(s);
 
  gs_input_string_len = ABS(gs_input_string_len); // set the sign flag

  if (len >= gs_input_string_len)  // if bigger than old string
    {
      delete [] gs_input_string;    // make as big as the new
      gs_input_string = new char[len];
      gs_input_string_len = len;
    }

  memcpy(gs_input_string, s, len); // now ready to copy
}

