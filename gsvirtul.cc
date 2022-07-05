// contains the overridden virtual functions from streambuf

#include "gstream"

using namespace std;


int gbuf::overflow(int c)
{
  setg(0, 0, 0);		// reset get area

  if (pptr() == 0) // make sure there is a put area
    setp(base(), ebuf());

  if (c != EOF && ucwidth(c) <= epptr() - pptr()) {
    pbump(usetc(pptr(), c));
    return c;
  } else {
    if (gwrite(pbase(), out_waiting()) >= 0) { // consume characters
      setp(base(), ebuf());	// if succeded set up put area
    } else {
      setp(0, 0);
      return EOF;		// indicate error
    }
    if (c != EOF) {
      pbump(usetc(pptr(), c));
      return c;
    }
  }
  return 0;
}

int gbuf::underflow()
{
  if (pptr() != 0 && pptr() > pbase()) // if there are characters waiting for output
    overflow(EOF);		// send them

  setp(0, 0);			// reset put area

  setg(base(), base(), ebuf()); // set get area

  int len = gread(base(), blen()); // produce characters

  if (len != 0)
    return (ugetc(base()) == EOF) ? 0 : ugetc(base()); // don't return EOF
  else {			// :-( we failed 
    setg(0, 0, 0);		// reset get area
    return EOF;
  }
}

int gbuf::sync()
{
  if (pptr() != 0 && pptr() > pbase())
    return overflow(EOF);       // flush waiting output

  return 0;                     // else nothing to do
}

