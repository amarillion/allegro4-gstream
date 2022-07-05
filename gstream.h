// main -*- C++ -*- header for gstream

#ifndef GSTREAM_HEADER
#define GSTREAM_HEADER

#include <allegro.h>
#ifdef __MINGW32__
    #include <winalleg.h>
#endif
#include <iostream>  // probably need this
#include <list>      // linked list for use with the tabs

#ifdef ALLEGRO_MSVC 
  #pragma warning (disable:4355)
#endif

#define GSTREAM_VERSION      1      // some defines for library information
#define GSTREAM_SUB_VERSION  4
#define GSTREAM_VERSION_STR  "1.4"
#define GSTREAM_DATE_STR     "2002"

// the function library
bool ia_allow_integer(int c);
bool ia_allow_decimal(int c);
bool ia_allow_hexadigits(int c);
bool ia_allow_everything(int c);
bool ia_block_spaces(int c);
bool ia_allow_word_chars(int c);

void cd_winconsole(BITMAP *bmp, int x, int y, int w, int h, int clr, bool ins);
void cd_console(BITMAP *bmp, int x, int y, int w, int h, int clr, bool ins);

void ieh_never_complain(int e);
void ieh_led_flasher(int e);

using namespace std;

class gbuf: public streambuf
{
public:    // interface
  typedef bool (*input_approver) (int);
  typedef void (*cursor_drawer) (BITMAP *, int, int, int, int, int, bool);
  typedef void (*input_error_handler) (int);
  typedef void (*dirty_rectangle_marker) (int, int, int, int);
				           // sync()  works as a flush()
  void goto_xy(int x, int y)                { sync(); gs_x = x; gs_y = y; }
  void goto_x(int x)                        { sync(); gs_x = x; }
  void goto_y(int y)                        { sync(); gs_y = y; }
  void goto_row(int row)                    { sync(); gs_y = gs_line_spacing * row; }
  void move_xy(int x, int y)                { sync(); gs_x += x; gs_y += y; }
  void move_x(int x)                        { sync(); gs_x += x; }
  void move_y(int y)                        { sync(); gs_y += y; }
  void move_row(int row)                    { sync(); gs_y += gs_line_spacing * row; }
  void set_bitmap(BITMAP *bmp)              { sync(); gs_bmp = bmp; }
  void set_line_spacing(int ls)             { sync(); gs_line_spacing = text_height(gs_font) + ls; }
  void set_font(FONT *the_font)             { sync(); gs_font = the_font; set_line_spacing(1); }
  void set_color(int clr)                   { sync(); gs_color = clr; }
  void set_tab_size(int ts)                 { sync(); gs_tab_size = ts; }
  void set_margin(int m)                    { sync(); gs_margin = m; }
  void set_wrap(bool w)                     { sync(); gs_wrap = w; }

  void set_insert_mode(bool im)             { gs_insert = im; }
  void set_input_string(char const *);
  void set_max_input_length(int ml)         { gs_max_input_length = ml; }
  void set_cursor_drawer(cursor_drawer cd)  { gs_cursor_drawer = cd; }
  void set_cursor_dimensions(int iw, int ih, int ow, int oh)
  { gs_cursor_iw = iw; gs_cursor_ih = ih; gs_cursor_ow = ow; gs_cursor_oh = oh; }
  void set_cursor_blink(int cb)             { gs_cursor_blink = cb == 0 ? 0 : BPM_TO_TIMER(cb * 2); }
  void set_input_error_handler(input_error_handler ieh)
  { gs_input_error_handler = ieh; }

  void set_dirty_rectangle_marker(dirty_rectangle_marker drm)
  { gs_dirty_rectangle_marker = drm; }
  
  void set_input_approver(input_approver ia){ gs_allowed = ia; }
  void save_input_approver()                { gs_old_allowed = gs_allowed; }
  void restore_input_approver()             { gs_allowed = gs_old_allowed; }

  void reset();

  void set_tab(int x)                       { set_tab(x, NO_COLOR_CHANGE, 0); }
  void set_tab(int x, int color)            { set_tab(x, color, 0); }
  void set_tab(int x, FONT *fnt)            { set_tab(x, NO_COLOR_CHANGE, fnt); }
  void set_tab(int x, int color, FONT *fnt);
  void remove_tab(int x);
  void remove_all_tabs();
  void restore();

  BITMAP *get_bitmap() const                { return gs_bmp; }
  FONT *get_font() const                    { return gs_font; }
  int get_x() const                         { return gs_x; }
  int get_y() const                         { return gs_y; }
  int get_color() const                     { return gs_color; }
  int get_margin() const                    { return gs_margin; }
  int get_tab_size() const                  { return gs_tab_size; }
  int get_line_spacing() const              { return gs_line_spacing; }
  bool get_insert_mode() const              { return gs_insert; }
  input_approver get_input_approver() const { return gs_allowed; }
  cursor_drawer get_cursor_drawer() const   { return gs_cursor_drawer; }
  int get_cursor_w() const                  { return gs_insert ? gs_cursor_iw : gs_cursor_ow; }
  int get_cursor_h() const                  { return gs_insert ? gs_cursor_ih : gs_cursor_oh; }
  input_error_handler get_input_error_handler() const
    { return gs_input_error_handler; }
  dirty_rectangle_marker get_dirty_rectangle_marker() const
    { return gs_dirty_rectangle_marker; }
  
#ifndef ALLEGRO_MSVC 
  static int const
    DYNAMIC_CURSOR_SIZE = -1;	// cursor dimensions should fit the char under it
#else
  static int const DYNAMIC_CURSOR_SIZE;
#endif
  
  enum input_error		// the input error handler is called with these
  {
    IE_UNRECOGNIZED_KEY,        // didn't recognize the pressed key
    IE_CURSOR_EXCEEDING_LIMITS, // trying to move the cursor too far
    IE_NO_ROOM_FOR_CHAR         // the input string can't contain the input
  };

protected:			// virtuals derived from class streambuf
  virtual int overflow(int ch);
  virtual int underflow();
  virtual int sync();

private:			// the data
#ifndef ALLEGRO_MSVC 
  static int const NO_COLOR_CHANGE = -2;
  static int const INSERT_STRING_START_SIZE = 128; // some day I really need to switch to a standard string
#else
  static int const NO_COLOR_CHANGE;
  static int const INSERT_STRING_START_SIZE;
#endif
  
  BITMAP *gs_bmp;		// the destination bitmap
  FONT *gs_font;		// the font
  int gs_x, gs_y,		// (x,y) position
    gs_color,			// the color of the font; -1 for multicolor fonts
    gs_margin,			// the margin at left side
    gs_tab_size,		// tabulator size, normally = 8
    gs_line_spacing,		// the number of pixels to go down when newlining
    gs_max_input_length,	// number of allowed characters for the inputter
    gs_cursor_iw,		// cursor width
    gs_cursor_ih,		// and height when in insert mode
    gs_cursor_ow,		// cursor width
    gs_cursor_oh,		// and height when in overwrite mode
    gs_cursor_blink;		// cursor flashing rate, 0 for off

  bool gs_wrap,			// whether to wrap at the left //boundaries
    gs_insert;			// whether in insert or overwrite mode when inputting

  char *
    gs_input_string;		// a default, editable string for the inputter
  int gs_input_string_len;	// the length of its memory chunk

  input_approver
    gs_allowed,			// whether the inputted char is allowed
    gs_old_allowed;		// saved gs_allowed

  cursor_drawer
    gs_cursor_drawer;		// the function that draws the cursor

  input_error_handler
    gs_input_error_handler;	// is called if an error is encountered during inputting

  dirty_rectangle_marker
    gs_dirty_rectangle_marker;	// is used to mark a rectangle as "dirty"
  
  struct tab			// a tabulator stop structure, contains information about tab changes
  {
    tab(int px, int pcolor, FONT *pfnt) : x(px), color(pcolor), fnt(pfnt) {}
    int x, color;
    FONT *fnt;
  };
  list<tab> gs_tab_list;	// a linked list of tabs

public:			// constructor and destructor
  gbuf()
  {
#if (INSERT_STRING_START_SIZE != 0)
    gs_input_string = new char[INSERT_STRING_START_SIZE];
    gs_input_string_len = -INSERT_STRING_START_SIZE;
#else
    gs_input_string = 0;
    gs_input_string_len = 0;
#endif
    gbufsize = 200;
    __gbuf__ = new char[gbufsize];
  }
  ~gbuf()
  {
    delete [] gs_input_string;
    delete [] __gbuf__;
  }

private:			// member helpers
  int gwrite(char const *text, int n); // outputter (handles special chars)
  void place_chars(char const *from, char const *to); // place chars, split lines
  void output_chars(char const *chars); // write chars
  int gread(char *text, int n);	// inputter
  
  char* base() { return __gbuf__;}
  char* ebuf() { return __gbuf__ + gbufsize; }
  int out_waiting() { return  pptr() - pbase(); }
  int blen() { return gbufsize; }

  char* __gbuf__;
  int gbufsize;
};

class gstream: virtual public ios, virtual public gbuf, public iostream
{
 public:
  gstream(BITMAP *bmp = screen)	// constructor
    :
      iostream(static_cast<streambuf *>(this))
    {
      reset();			// reset everything
      set_bitmap(bmp);		// and set the chosen bitmap
    }
};

// a little hack to make the fix class capable of printing and extracting
inline ostream& operator<<(ostream& o, fix const& x) { return (o << fixtof(x.v)); }
inline istream& operator>>(istream& i, fix& x) { float ftmp; i >> ftmp; x.v = ftofix(ftmp); return i; }

#endif // !defined GSTREAM_HEADER

