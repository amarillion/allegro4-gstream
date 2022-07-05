// manipulators specific for gstreams

#include "gstream"
#include "gmanip"

using namespace std;

#ifndef ALLEGRO_MSVC
template class gmanip<int>; // instantiate a manipulator class for integers as default
#endif

gstream& x(gstream& g, int px) // moves to a given x-coordinate
{
 g.goto_x(px);
 return g;
}

gstream& y(gstream& g, int py) // moves to a given y-coordinate
{
 g.goto_y(py);
 return g;
}

gstream& row(gstream& g, int prow)  // moves to a given row with respect to the line spacing
{
 g.goto_row(prow);
 return g;
}

gstream& color(gstream& g, int clr) // set font colour
{
 g.set_color(clr);
 return g;
}

gstream& tab_size(gstream& g, int ts) // set the standard tabulator size
{
 g.set_tab_size(ts);
 return g;
}

gstream& margin(gstream& g, int m)  // the left margin; number of pixels to move when '\n'ing
{
 g.set_margin(m);
 return g;
}

gstream& line_spacing(gstream& g, int ls) // number of pixels to move down when '\n'ing
{
 g.set_line_spacing(ls);
 return g;
}

gstream& max_input_length(gstream& g, int ml) // max length of inputted string
{
 g.set_max_input_length(ml);
 return g;
}


ios& wrap(ios& strm) // wrap at the right boundary
{
 dynamic_cast<gstream*>(&strm)->set_wrap(true);
 return strm;
}

ios& no_wrap(ios& strm) // don't wrap at the right boundary
{
 dynamic_cast<gstream*>(&strm)->set_wrap(false);
 return strm;
}

ios& insert(ios& strm) // use insert mode when inputting
{
 dynamic_cast<gstream*>(&strm)->set_insert_mode(true);
 return strm;
}

ios& overwrite(ios& strm) // use overwrite mode when inputting
{
 dynamic_cast<gstream*>(&strm)->set_insert_mode(false);
 return strm;
}

