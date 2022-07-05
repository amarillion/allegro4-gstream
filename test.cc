// test program for gstream

#include <allegro.h>

#include "gstream"
#include "gmanip"
#include <string>

#include "font.h"
using namespace std;


// call back
void return_negative_color(PALETTE const pal, int x, int y, RGB *rgb);

int main()
{
 allegro_init();

 install_keyboard();		// if you use the extraction functions, this
				// must be done

 if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) < 0)
   {
    // better get a better graphics card :)
    cerr << "Sorry, but... " << allegro_error;
    return 1;
   }

 install_timer();		// we later need a blinking cursor

 PALETTE a_pal;
 generate_332_palette(a_pal);
 set_palette(a_pal);

 text_mode(-1);			// for transparent text background

 DATAFILE *data = load_datafile("font.dat"); // get the font
 if (data == 0)
   {
    cerr << "Cannot find the font.dat file!\n";
    return 1;
   }

 gstream gs(screen);		// construct our gstream

 gs.set_font(static_cast<FONT *>(data[MY_FONT].dat)); // set the font

 gs.set_color(makecol8(0,0,0)); // set the drawing color to black

 clear_to_color(screen, makecol8(255, 255, 255));

 
 gs << "Hello world!\n\n";      // print some text on the screen

 // and print a few numbers, simple isn't it?
 gs << "And welcome to gstream, v." << GSTREAM_VERSION << "." << GSTREAM_SUB_VERSION << ", " << GSTREAM_DATE_STR << "\n"
   "\n"
   "This demonstrated that a gstream is capable of outputting numbers,\n"
   "just like a normal ostream.\n"
   "\n"
   "Press a key." << endl;

 // fresh page *****************************
 readkey();
 clear_to_color(screen, makecol8(255, 255, 255));
 gs.goto_xy(0,0);
 gs.set_font(font);
 gs << "\n\n\nAnd now some Unicode:\n"
   "   \xc5\xbdlut\xc3\xbd k\xc5\xaf\xc5\x88 sk\xc3\xa1kal p\xc5\x99""es oves!" "\n"
   "\n"
   "Press a key." << endl;
 gs.set_font(static_cast<FONT *>(data[MY_FONT].dat)); // set the font
 
 // fresh page *****************************
 readkey();
 clear_to_color(screen, makecol8(255, 255, 255));
 gs.goto_xy(0,0);

 
 // show that you sometimes have to flush to actually perform the drawing
 gs << "Since gstream is derived from the iostreams, you sometimes have\n"
   "flush it to make it output the pending characters.\n"
   "\n"
   "For instance, to let the computer figure out the next part of this text,\n"
   "you have to press a key so that it can be flushed.\n"
   "\n"
   "So what is really the meaning of life? " << flush;

 gs << "Sorry, that is a meaningless question.";

 readkey();

 gs << flush;
 

 // fresh page *****************************
 readkey();
 clear_to_color(screen, makecol8(255, 255, 255));
 gs.goto_xy(0,0);

 
 gs << "Now it would be good idea to take a look at the tabulator functions.\n"
   "First we have a table where the texts are separated with normal tabs,\n"
   "next a table separated with smart tabs.\n"
   "\n";

 // print a table separated with normal tabs
 gs << " Name:\t\t"        "Characteristics:\t\t"   "Job:\n"
    << " -\t\t"            "-\t\t\t"                "-\n"
    << " King Fu Li\t"     "Chinese, yellow\t\t"    "despot\n"
    << " Clever Annibal\t" "Headhunter, red\t\t"    "cook\n"
    << " Garfield\t"       "Cat, fat, orange\t\t"   "entertainer\n"
    << endl << endl;


 // set a few tab-stops; note the random order :-)
 gs.set_tab(150);
 gs.set_tab(20, makecol(255,0,0));
 gs.set_tab(330, makecol(0,0,255));

 // now print the table using single tabs with special characteristics
 gs << "\tName:\t"          "Characteristics:\t"     "Job:\n"
    << "\t-\t"              "-\t"                    "-\n"
    << "\tKing Fu Li\t"     "Chinese, yellow\t"      "despot\n"
    << "\tClever Annibal\t" "Headhunter, red\t"      "cook\n"
    << "\tGarfield\t"       "cat, fat, orange\t"     "entertainer\n"
    << endl << endl;

 gs.remove_all_tabs();		// get rid of the tab stops


 
 // fresh page *****************************
 readkey();
 clear_to_color(screen, makecol8(255, 255, 255));
 gs.goto_xy(0,0);
	

 gs << "The gstreams are also capable of wrapping the output - it will be\n"
   "wrapped at the spaces. If a word is too long to fit the line,\n"
   "it will not be wrapped, but simply outputted at the beginning of\n"
   "the next line (thus probably having the last part clipped). A\n"
   "subbitmap is used to get the small window below.\n" << endl;
    
 // let's see some wrapping
 BITMAP *small_part = create_sub_bitmap(gs.get_bitmap(), 100, gs.get_y(), 200, 200);
 gs.set_bitmap(small_part);
 gs.goto_xy(0, 0);		// goto origin
 
 gs.set_wrap(true);
 gs << "This line is a very, very, very, very, very, very, very, "
       "very, very, veeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeery, "
       "very, very, very, very, very, very, very long line... ";
 gs.set_wrap(false);

 gs << endl << endl;

 gs.set_bitmap(screen);
 destroy_bitmap(small_part);

 

 // fresh page *****************************
 readkey();
 clear_to_color(screen, makecol8(255, 255, 255));
 gs.goto_xy(0,0);
 

 gs << "Some manipulators have been designed for the gstreams, e.g. \n"
   "the positioning ones.\n"
   "\n";
 
 // demonstrate the x(int where) manipulator
 gs << x(100) << "at 100" << x(200) << "at 200";// this is the preferred syntax

 x(gs, 400) << "at 400";	// but this is also possible

 gs << endl << endl;


 
 // fresh page *****************************
 readkey();
 clear_to_color(screen, makecol8(255, 255, 255));
 gs.goto_xy(0,0);
 

 gs << "At last we'll take a look at some input situations.\n"
   "\n"
   "First an inputter that can only accept numbers, then one that\n"
   "flashes the keyboard leds upon error, and finally one that uses a\n"
   "negation effect to show the cursor.\n"
   "\n";

 // now let's try getting a number
 int a_number;
 gs << "Enter a number: ";

 gs.save_input_approver();	// first save old approver
 gs.set_input_approver(ia_allow_integer); // then set a digit approver
 gs.set_insert_mode(true);	// set insert mode
 gs.set_max_input_length(9);	// and max 9 digits

 // the actual input, hehe, how easy can it be?
 gs >> a_number;

 gs << "\nYou entered: " << a_number << endl << endl;

 gs.restore_input_approver();	// restore the previous approver


 // turn flashing on, one blink per second
 gs.set_cursor_blink(60);

 // set a fancy handler that flashes the keyboard leds upon error
 gs.set_input_error_handler(ieh_led_flasher);

 // and get a string
 char str[200];
 gs << "Enter a string: ";
 gs.set_input_string("gstream is wonderful!"); // set this truth to an editable default
 gs.getline(str, sizeof(str));	// and get the input
 gs << "\nYou entered: " << str
    << endl << endl;


 // now demonstrate a cursor with negative colour effect

 // first some uninteresting initialisation
 COLOR_MAP negative_table;
 create_color_table(&negative_table, a_pal, return_negative_color, 0);
 color_map = &negative_table;
 drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

 // creating the color table may take a while, so we kill any impatient keypresses
 clear_keybuf();

 // and now launch the inputter by getting a string once again
 gs << "Enter a string: ";
 gs.set_input_string("gstream makes me happy!");
 gs.getline(str, sizeof(str));
 gs << "\nYou entered: " << str << endl << endl;
 

 readkey();			// wait for a keypress

 return 0;
}

END_OF_MAIN();

// this function is borrowed from an example for Allegro, by Grzegorz Adam Hankiewicz
void return_negative_color(PALETTE const pal, int x, int y, RGB *rgb)
{
   rgb->r = 63-pal[y].r;
   rgb->g = 63-pal[y].g;
   rgb->b = 63-pal[y].b;
}




