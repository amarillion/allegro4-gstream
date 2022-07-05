// header for manipulators for use with gstreams

#ifndef GMANIP_HEADER
#define GMANIP_HEADER

#include "gstream" // hehe, better do this

// manipulator class
template <class T> class gmanip
{
   typedef gstream& (*gmanip_func) (gstream&, T); // handy typedef
 public:
   gmanip(gmanip_func f, T v) : func(f), var(v) {} //ctor

   // overloaded operators for gstream
   inline friend gstream& operator<< (gstream& gstrm, const gmanip<T>& mc)
     {
      (*mc.func)(gstrm, mc.var); // launch the function in the manipulator class
      return gstrm;
     }
   inline friend gstream& operator>> (gstream& gstrm, const gmanip<T>& mc)
     {
      (*mc.func)(gstrm, mc.var); // launch the function
      return gstrm;
     }

   // sometimes we get an ordinary stream though it really is a gstream
   inline friend ostream& operator<< (ostream& gstrm, const gmanip<T>& mc)
     {
      (*mc.func)(*dynamic_cast<gstream*>(&gstrm), mc.var); // launch the function in the manipulator class
      return gstrm;
     }
   inline friend istream& operator>> (istream& gstrm, const gmanip<T>& mc)
     {
      (*mc.func)(*dynamic_cast<gstream*>(&gstrm), mc.var); // launch the function
      return gstrm;
     }

 private: // data
   gmanip_func func; // pointer to the actual gstream manipulator
   T var;            // a variable
};

extern template class gmanip<int>; // an int class is instantiated by default

// this macro is designed to make the declaration of manipulators a bit easier
#define GSTREAM_MANIPULATOR_MAKER(name, type) \
  extern gstream& name(gstream&, type);       \
  inline gmanip<type> name(type the_parameter) { return gmanip<type> (name, the_parameter);}

GSTREAM_MANIPULATOR_MAKER(x, int);
GSTREAM_MANIPULATOR_MAKER(y, int);
GSTREAM_MANIPULATOR_MAKER(row, int);
GSTREAM_MANIPULATOR_MAKER(color, int);
GSTREAM_MANIPULATOR_MAKER(tab_size, int);
GSTREAM_MANIPULATOR_MAKER(margin, int);
GSTREAM_MANIPULATOR_MAKER(line_spacing, int);
GSTREAM_MANIPULATOR_MAKER(max_input_length, int);

#undef GSTREAM_MANIPULATOR_MAKER // we don't need it anymore

extern ios& wrap(ios&); // wrap at the right boundary
extern ios& no_wrap(ios&); // don't wrap at the right boundary
extern ios& insert(ios&); // use insert mode when inputting
//extern gstream& overwrite(gstream&); // would be nice to get this to work
extern ios& overwrite(ios&); // use overwrite mode when inputting

#endif // !defined GMANIP_HEADER
