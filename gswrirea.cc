// the inputter/outputter functions of class gbuf together with a few helpers

#include "gstream"
#include <string.h>

using namespace std;

#define MAX_CHAR_SIZE 6

char chr_to_str_buf[MAX_CHAR_SIZE + sizeof(EMPTY_STRING)];

/* char_to_str:
 *  Converts the given character to a null-terminated string and returns a
 *  pointer to this. Useful for constructions such as
 *
 *    text_length(my_font, char_to_str(my_character));
 *
 *  As the string is kept in a static buffer, the next call to char_to_str will
 *  overwrite it.
 */   
inline char *chr_to_str(int chr)
{
   int chr_size = usetc(chr_to_str_buf, chr);
   usetc(chr_to_str_buf + chr_size, '\0');
   return chr_to_str_buf;
}


// this layer outputs the chars, calling a custom dirty rectangle system if it
// exists 
void gbuf::output_chars(char const *chars) 
{
  textout(gs_bmp, gs_font, chars, gs_x, gs_y, gs_color);

  if (gs_dirty_rectangle_marker != 0) {
    gs_dirty_rectangle_marker(gs_x, gs_y, text_length(gs_font, chars),
			      text_height(gs_font));
  }
}

char *write_string;

// put a string into write_string
inline void qualify_string(char const *from, char const *to)
{
  memcpy(write_string, from, to - from);
  usetc(write_string + (to - from), '\0');
}

// this layer is responsible for wrapping the text if necessary
void gbuf::place_chars(char const *from, char const *to)
{
  if (from == to)
    return;
  
  write_string = new char[(to - from) + sizeof(EMPTY_STRING)]; // create string (and remember '\0')

  if (!gs_wrap) {		// no wrapping so simply blast the chars away
    qualify_string(from, to);
    output_chars(write_string);
    gs_x += text_length(gs_font, write_string);
  }
  else {
    if (gs_x >= gs_bmp->w) {
      gs_x = gs_margin;
      gs_y += gs_line_spacing;
      // restore();		
    }

    char const *b = from, *l = from, *c = from; // base, last space, current pointers
    int space_width = text_length(gs_font, chr_to_str(' '));
    
    do {
      if (ugetxc(&c) == ' ') {
	qualify_string(b, c);

	if (text_length(gs_font, write_string) - space_width + gs_x > gs_bmp->w) {
	  if (b == l) {		// at beginning of too long word at beginning of line
	    output_chars(write_string);

	    gs_x = gs_margin;	// next line
	    gs_y += gs_line_spacing;

	    b = c;
	  }
	  else {
	    qualify_string(b, l);
	    output_chars(write_string);

	    gs_x = gs_margin;	// next line
	    gs_y += gs_line_spacing;
	  
	    b = l;		// change the base now the chars are outputted
	  }
	}
	l = c;
      }
    }
    while (c != to);

    if (l != b) {		// we have some characters pending
      qualify_string(b, l);
      if (text_length(gs_font, write_string) - space_width + gs_x > gs_bmp->w) {
	gs_x = gs_margin;	// next line
	gs_y += gs_line_spacing;
	b = l;
      }
    }
    
    qualify_string(b, c);	// output the rest
    output_chars(write_string);
    gs_x += text_length(gs_font, write_string);

  }
  
  delete [] write_string;
}

// the outputter, handles special chars such as '\n', then sends the output to
// the next layer, place_chars, which takes care of wrapping the lines if
// necessary. place_chars in turn calls output_chars to actually consume the
// characters
int gbuf::gwrite(char const *text, int n)
{
  // the pointers delimit the currently outputable range
  char const *to_ptr = text;
  char const *from_ptr = text;

  int chr;			// the actual character
  int i = 0;

  do {
  finish:
    chr = ugetc(to_ptr);
    i += ucwidth(chr);

    if (chr == '\n') {		// if newline, goto to beginning of the next line
      place_chars(from_ptr, to_ptr);
      from_ptr = to_ptr = text + i;

      gs_x = gs_margin;
      gs_y += gs_line_spacing;
      restore();		// and restore settings
    }
    else if (chr == '\r') {	// carriage return: goto beginning of this line
      place_chars(from_ptr, to_ptr);
      from_ptr = to_ptr = text + i;

      gs_x = gs_margin;
      restore();		// and restore settings
    }
    else if (chr == '\t') {	// tab encountered
      place_chars(from_ptr, to_ptr);
      from_ptr = to_ptr = text + i;

      for (list<tab>::iterator it = gs_tab_list.begin(); it != gs_tab_list.end(); ++it )
	if (gs_x < it->x) {	// if an appropriate smart tab is found
	  gs_x = it->x;		// move to tab position

	  restore();		// and restore the settings

	  if (it->color != NO_COLOR_CHANGE) {
	    if (gs_tab_list.begin()->color == NO_COLOR_CHANGE)
	      gs_tab_list.begin()->color = gs_color; // backup color
	    gs_color = it->color; // change color if necessary
	  }

	  if (it->fnt != 0) {
	    if (gs_tab_list.begin()->fnt == 0)
	      gs_tab_list.begin()->fnt = gs_font; // backup font
	    gs_font = it->fnt;	// change font if necessary
	  }

	  goto finish;		// quit loop
	}
      // else if no inserted tab was found, stick to the default
      gs_x += gs_tab_size * text_length(gs_font, chr_to_str(' ')) -
	(gs_x % (gs_tab_size * text_length(gs_font, chr_to_str(' '))));
      restore();		// settings
    }
    else
      to_ptr = text + i;
  }
  while (i < n);

  if (to_ptr != from_ptr)	// if we did not process a special character
    place_chars(from_ptr, to_ptr); // write the remaining part
  
  return i;
}


volatile int cursor_blink_state;
int cursor_on;

static void cursor_blink_int()
{
  --cursor_blink_state;
}
END_OF_FUNCTION(cursor_blink_int);

/* The InputObject class is the base class of a smart char class which is used
   for representing the characters in the input string, and a cursor class which
   represents the cursor. This is done to make them able to draw, save and recover
   the background, and reposition themselves. */
class InputObject // ugly name, I'll have to find a better one someday
{
public:				// the y parameter is currently really not needed
  InputObject(gbuf& b, int px, int py, int pw, int ph)
    : buf(b), posx(px), posy(py)
    {
      bmp = create_bitmap(pw, ph);
      save();
    }

  ~InputObject() { destroy_bitmap(bmp); }

  void clear()
    {
      blit(bmp, buf.get_bitmap(), 0, 0, posx, posy, bmp->w, bmp->h);
    }

  void save()			// save background
    {
      blit(buf.get_bitmap(), bmp, posx, posy, 0, 0, bmp->w, bmp->h);
    } 

  int width() const                { return bmp->w; }
  int x() const                    { return posx; }

protected: // it is easier with direct access to these in the derived classes
  gbuf& buf;			// the gbuffer to operate on
  int posx, posy;		// position in pixels
  BITMAP *bmp;			// the background bitmap
};

class Schar : public InputObject
{
public:
  Schar(gbuf& b, int px, int py, int c)
    : InputObject(b, px, py, text_length(b.get_font(), chr_to_str(c)), text_height(b.get_font())),
      chr(c)
    { draw(); }

  // copy constructor
  Schar(Schar const& other) :
    InputObject(other.buf, other.posx, other.posy, other.bmp->w, other.bmp->h),
    chr(other.chr)
    {
      blit(other.bmp, bmp, 0, 0, 0, 0, bmp->w, bmp->h);
    }

  void move(int x) { posx += x; save(); draw(); } // move relative

  void draw()
  {
    textout(buf.get_bitmap(), buf.get_font(), chr_to_str(chr), posx, posy, buf.get_color());
  }

  int get_char() const { return chr; }

private:
  int chr; // the glyph
};

class Cursor : public InputObject
{
public:
  // the constructor looks a bit ugly
  Cursor(gbuf& b, int px, int py, Schar *scu = 0) :
    InputObject(b, px, py,
		 // compute width
		 b.get_cursor_w() == gbuf::DYNAMIC_CURSOR_SIZE ?
		 (scu != 0 ? scu->width() : text_length(b.get_font(), chr_to_str(' '))) :
		 b.get_cursor_w(),       // defaults to size of a space
		 // compute height
		 b.get_cursor_h() == gbuf::DYNAMIC_CURSOR_SIZE ?
		 text_height(b.get_font()) :
		 b.get_cursor_h()
		 ),
    schar_under(scu)
    { draw(); }

  void move(int x) { posx += x; save(); draw(); } // move relative

  void draw()     // the function from the gbuffer will take care of drawing
  {
    if (cursor_on)
      buf.get_cursor_drawer()(buf.get_bitmap(), posx, posy, bmp->w, bmp->h,
			      buf.get_color(), buf.get_insert_mode());
  }
  
  void recalculate_background()
  {
    int nw = buf.get_cursor_w() == gbuf::DYNAMIC_CURSOR_SIZE ?
      (schar_under != 0 ? schar_under->width() : text_length(buf.get_font(), chr_to_str(' '))) :
      buf.get_cursor_w();
    int nh = buf.get_cursor_h() == gbuf::DYNAMIC_CURSOR_SIZE ?
      text_height(buf.get_font()) :
      buf.get_cursor_h();
    if (nw != bmp->w || nh != bmp->h) { // if changed
      destroy_bitmap(bmp);
      bmp = create_bitmap(nw, nh);
    }
  }

  void set_scu(Schar *scu)
  {
    schar_under = scu;
    recalculate_background();
  }
  
  void environmental_change()
  {
    clear();
    recalculate_background();
    save();
    draw();
  }
  
private:
  Schar *schar_under;
};

// the actual inputter (not to be pronounced as greed, but g-read! :-)
int gbuf::gread(char *text, int n) // text is destination, n is max. # of chars
{
  list<Schar> o_list;		// linked list of smart characters + a cursor
  list<Schar>::iterator it;	// general purpose iterator
  list<Schar>::iterator cit;	// the schar under the cursor
  Cursor *crs;			// cursor
  int len = 0,			// length of string (in bytes)
    lenx = 0,			// length in actual pixels
    chr_len = 0;		// general purpose length container

  cursor_on = true;

  if (gs_input_string_len > 0) { // if we have to do some default characters
    char *chr_ptr = gs_input_string;
    int chr;

    while ((chr = ugetx(&chr_ptr)) != 0) {
      o_list.push_back(Schar(*this, gs_x + lenx, gs_y, chr)); // make new smart char
      lenx += text_length(gs_font, chr_to_str(chr)); // ready for next
      len += ucwidth(chr);
    }
    // make input string length negative to indicate that it is not being used
    gs_input_string_len = -gs_input_string_len;
    crs = new Cursor(*this, gs_x + lenx, gs_y, &(*(--o_list.end()))); // get cursor
  }
  else
    crs = new Cursor(*this, gs_x, gs_y, 0); // null to indicate no char

  cit = o_list.end();		// cursor is at the end

  // if no max specified or max above buffer limit, set to limit
  if (gs_max_input_length == -1 || gs_max_input_length + ucwidth('\n') > n)
    gs_max_input_length = n - ucwidth('\n'); // make room for the '\n' too

  // handle cursor blinking
  if (gs_cursor_blink > 0)
    {
      LOCK_VARIABLE(cursor_blink_state);
      LOCK_FUNCTION(cursor_blink_int);
      install_int_ex(cursor_blink_int, gs_cursor_blink);
    }

  cursor_blink_state = 0;

  bool end = false;
  while (!end) {
    if (gs_cursor_blink > 0) {
      if (cursor_blink_state < 0) {
	if (cursor_on) {	// turn off cursor
	  crs->clear();
	  cursor_on = false;
	}
	if (cursor_blink_state < -1) // or reset timer
	  cursor_blink_state = 0;
      }
      else			// else turn on cursor
	if (!cursor_on) {
	  cursor_on = true;
	  crs->draw();
	}
    }
      
    if (!keypressed())		// if no key is waiting, continue loop
      continue;
      
    int scancode;
    int chr = ureadkey(&scancode); // else get it
    bool unrecognized_key = false;
    switch (scancode)
      {
      case KEY_ENTER_PAD:
      case KEY_ENTER:		// stop if enter pressed
	if (len > 0)		// but only if we can return some characters
	  end = true;
	break;
	
      case KEY_INSERT:		// the user can actually change the insert flag
	gs_insert = !gs_insert;
	crs->environmental_change(); // inform cursor about the change
	break;
	
      case KEY_UP:		// we can't use up and down (no multiline input)
      case KEY_DOWN:		// but they are expected to move the cursor so complain
	gs_input_error_handler(IE_CURSOR_EXCEEDING_LIMITS);
	break;
	
      case KEY_BACKSPACE:	// delete previous char if backspace pressed
	if (cit == o_list.begin()) {
	  gs_input_error_handler(IE_CURSOR_EXCEEDING_LIMITS);
	  break;
	}
	it = cit;
	--it;
	chr_len = it->width();	// save the char length
	crs->clear();		// clear cursor
	it->clear();		// first clear
	len -= ucwidth(it->get_char());
	o_list.erase(it);	// then delete
	for (it = cit; it != o_list.end(); ++it)
	  {
	    it->clear();
	    it->move(-chr_len); // move the remaining part of the line back
	  }
	crs->move(-chr_len);	// step back with cursor
	break;
	
      case KEY_DEL:		// delete next char if delete pressed
	if (cit == o_list.end()) {
	  gs_input_error_handler(IE_CURSOR_EXCEEDING_LIMITS);
	  break;
	}
	chr_len = cit->width(); // save the char length
	crs->clear();
	cit->clear();		// first clear
	it = cit;
	++cit;			// move cursor to next
	if (cit == o_list.end())
	  crs->set_scu(0);	// mark "no character under cursor" with null
	else
	  crs->set_scu(&(*cit));
	len -= ucwidth(it->get_char());
	o_list.erase(it);	// then delete
	for (it = cit; it != o_list.end(); ++it) {
	  it->clear();
	  it->move(-chr_len); // move the rest of the line back
	}
	crs->save();		// redraw/reget cursor
	crs->draw();
	break;
	
      case KEY_LEFT:		// move cursor left
	if (cit == o_list.begin()) {
	  gs_input_error_handler(IE_CURSOR_EXCEEDING_LIMITS);
	  break;
	}
	if (key_shifts & KB_CTRL_FLAG) { // one word if control pressed
	  chr_len = 0;
	  bool is_in_word = false;
	  do {
	    --cit;
	    if (!ia_allow_word_chars(cit->get_char())) {
	      if (is_in_word) {
		++cit; // step back because
		break; // we were in word, but not any longer
	      }
	    }
	    else
	      is_in_word = true;
	    chr_len -= cit->width(); // keep track of move in pixels
	  }
	  while (cit != o_list.begin());

	  crs->clear();
	  crs->set_scu(&(*cit));
	  crs->move(chr_len);
	}
	else {			// else just one char
	  --cit;
	  crs->clear();
	  crs->set_scu(&(*cit));
	  crs->move(-cit->width());
	}
	break;
	
      case KEY_RIGHT:		// move cursor right
	if (cit == o_list.end()) {
	  gs_input_error_handler(IE_CURSOR_EXCEEDING_LIMITS);
	  break;
	}
	if (key_shifts & KB_CTRL_FLAG) { // one word if control pressed
	  chr_len = 0;
	  bool is_outside_word = false;
	  for (;;) { // ever
	    if (ia_allow_word_chars(cit->get_char())) {
	      if (is_outside_word) {
		crs->clear();
		crs->set_scu(&(*cit)); // update cursor
		break;	// we were outside word, but not any longer
	      }
	    }
	    else
	      is_outside_word = true;
	    chr_len += cit->width(); // keep track of move in pixels
	    if (++cit == o_list.end()) {
	      crs->clear();
	      crs->set_scu(0); // mark "no character under cursor" with null
	      break;
	    }
	  }
	  crs->move(chr_len);
	}
	else {			// else just move one char
	  crs->clear();
	  chr_len = cit->width();
	  ++cit;
	  if (cit == o_list.end())
	    crs->set_scu(0); // mark "no character under cursor" with null
	  else
	    crs->set_scu(&(*cit));
	  crs->move(chr_len);
	}
	break;
	
      case KEY_HOME:		// if home pressed, move cursor to the beginning
	if (cit == o_list.begin()) {
	  gs_input_error_handler(IE_CURSOR_EXCEEDING_LIMITS);
	  break;
	}
	chr_len = 0;
	do {
	  --cit;
	  chr_len += cit->width(); // find out how much to move cursor
	}
	while (cit != o_list.begin());
	crs->clear();		// move cursor on bitmap
	crs->set_scu(&(*cit));
	crs->move(-chr_len);
	break;
	
      case KEY_END:		// if end pressed, move cursor to the EOL
	if (cit == o_list.end()) {
	  gs_input_error_handler(IE_CURSOR_EXCEEDING_LIMITS);
	  break;
	}
	chr_len = 0;
	do {
	  chr_len += cit->width();
	  ++cit;
	}
	while (cit != o_list.end());
	crs->clear();		// move cursor on bitmap
	crs->set_scu(0);
	crs->move(chr_len);
	break;

      default:			// else: a normal character
	// catch control codes && see if char is allowed
	if (chr != 0 && gs_allowed(chr)) {
	  if (!gs_insert && cit != o_list.end()) { // if in overwrite mode, first delete
	    if (len + ucwidth(chr) - ucwidth(cit->get_char()) > gs_max_input_length) {
	      gs_input_error_handler(IE_NO_ROOM_FOR_CHAR); // too big, even with one char less
	      break;
	    }
	    len += ucwidth(chr) - ucwidth(cit->get_char());
	    chr_len = text_length(gs_font, chr_to_str(chr)) - cit->width();
	    crs->clear();
	    cit->clear();	// first clear
	    it = cit;
	    ++cit;		// move cursor to next
	    if (cit == o_list.end())
	      crs->set_scu(0);	// mark "no character under cursor" with null
	    else
	      crs->set_scu(&(*cit));
	    o_list.erase(it);	// then delete

	    if (chr_len < 0) {	// move the remaining part of the line backwards
	      for (it = cit; it != o_list.end(); ++it) {
		it->clear();
		it->move(chr_len);
	      }
	    }
	    else if (chr_len > 0) { // move the line forwards
	      it = o_list.end();
	      while (it != cit) {
		--it;
		it->clear();
		it->move(chr_len);
	      }
	    }
	    o_list.insert(cit, Schar(*this, crs->x(), gs_y, chr)); // add new
	    crs->move(text_length(gs_font, chr_to_str(chr)));
	    lenx += chr_len;
	  }
	  else {
	    if (len + ucwidth(chr) > gs_max_input_length) { // no more chars allowed
	      gs_input_error_handler(IE_NO_ROOM_FOR_CHAR);
	      break;
	    }
	    chr_len = text_length(gs_font, chr_to_str(chr));
	    crs->clear();	// remove cursor from bmp
	    it = o_list.end();
	    while (it != cit) {
	      --it;
	      it->clear();
	      it->move(chr_len);// move the characters under and after the cursor
	    }
	    o_list.insert(cit, Schar(*this, crs->x(), gs_y, chr)); // add new
	    crs->move(chr_len);	// move the cursor
	    len += ucwidth(chr); // increase string length
	    lenx += chr_len;
	  }
	  break;
	}
	else
	  unrecognized_key = true; // we couldn't use the key at all
	break;
      }
    
    if (!unrecognized_key)	
      cursor_blink_state = 1;	// delay blink
    else
      gs_input_error_handler(IE_UNRECOGNIZED_KEY);
  } // end of loop

  if (gs_cursor_blink > 0)
    remove_int(cursor_blink_int);

  crs->clear();			// we don't want the cursor to stay

  gs_x += lenx;			// update gbuf

  int j = 0;
  for (it = o_list.begin(); it != o_list.end(); ++it)
    j += usetc(&text[j], it->get_char()); // output characters to buffer

  o_list.clear();		// cleanup
  delete crs;
  gs_max_input_length = -1;

  j += usetc(&text[j],'\n');

  return j;			// bye, bye
}

