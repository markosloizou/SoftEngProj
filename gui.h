#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/cshelp.h>
#include <string>
#include <vector>
#include <fstream>

#include "names.h"
#include "devices.h"
#include "monitor.h"
#include "parser.h"

typedef struct 
{
	name devId;
	name pinId;
	int number;
	string nme;
	int startTime;
} mons;

typedef struct
{
	name devId;
	name pinId;
} monDev;

enum { 
	MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  	MY_TEXTCTRL_ID,
  	MY_RUN_BUTTON_ID,
  	MY_FILE_RUN_ID,
  	MY_FILE_CONTINUE_ID,
  	MY_FILE_SAVE_ID,
  	TOOLBAR_ID,
  	RUN_TOOLBAR_ID,
  	CONTINUE_TOOLBAR_ID,
  	START_TOOLBAR_ID,
  	END_TOOLBAR_ID,
  	MY_CONTINUE_BUTTON_ID,
  	VERT_SLIDER_ID,
  	HORZ_SLIDER_ID,
  	SWITCH_LIST_ID,
  	SWITCH_BUTTON_ID,
  	SWITCH_LISTBOX_ID,
  	MONITOR_LISTBOX_ID,
  	SHOW_GRID_ID,
  	SHOW_SETTINGS_ID,
  	SHOW_DIALOG_ID,
  	SAVE_TOOLBAR_ID,
  	START_MENU_ID,
  	END_MENU_ID,
  	SAVE_MENU_ID,
  	ZOOM_IN_MENU_ID,
  	ZOOM_OUT_MENU_ID,
  	HELP_MENU_ID,
}; // widget identifiers

class MyGLCanvas;

class MyFrame: public wxFrame
{
 	public:
  	MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, names *names_mod = NULL, devices *devices_mod = NULL, monitor *monitor_mod = NULL, parser * parser_mod = NULL, long style = wxDEFAULT_FRAME_STYLE); // constructor
  	void print_action(string message);
  	//void mirror_char(unsigned char *pixels, int width, int height);
  	void zoom(int n);
  	
  	
 	private:
  	MyGLCanvas *canvas;                     // OpenGL drawing area widget to draw traces
  	names *nmz;                             // pointer to names class
  	devices *dmz;                           // pointer to devices class
  	monitor *mmz;                           // pointer to monitor class
  	parser *prs;
	
	wxWindow *left_canvas_window = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN, "Canvas");
	wxWindow *dialog_window = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, "Dialog");
	wxWindow *right_button_window = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN, "Settings");
		
	wxMenu *fileMenu = new wxMenu;
	wxMenu *viewMenu = new wxMenu;
	wxMenu *windowMenu = new wxMenu;	
		
  	wxBoxSizer *frame_sizer = new wxBoxSizer(wxHORIZONTAL);
  	wxBoxSizer *left_canvas_sizer = new wxBoxSizer(wxVERTICAL);
  	wxBoxSizer *canvas_sizer = new wxBoxSizer(wxVERTICAL);
  	wxBoxSizer *toolbar_sizer = new wxBoxSizer(wxHORIZONTAL);
  	wxBoxSizer *right_button_sizer = new wxBoxSizer(wxVERTICAL);
  	wxBoxSizer *dialog_sizer = new wxBoxSizer (wxVERTICAL);
  	wxSpinCtrl *spin = new wxSpinCtrl(right_button_window, MY_SPINCNTRL_ID, wxString("10"));  	
  	wxSlider *vert_zoom_slider;
  	wxSlider *horz_zoom_slider;
  	wxStaticText *vert_zoom_value = new wxStaticText(right_button_window, wxID_ANY, "x100%");
  	wxStaticText *horz_zoom_value = new wxStaticText(right_button_window, wxID_ANY, "x100%");
  	int action_num = 2;
  	
  	wxString *switch_list;
  	wxCheckListBox *toggle_list;
  	wxString *monitor_list;
  	wxCheckListBox *monitor_list1;
  	wxString *action_list;
	wxListBox *action_list1;
	
  	wxKeyboardState *kState;  
  	wxDialog *helpD;
  	
  	bool show_grid = true;
  	bool show_settings = true;
  	bool show_dialog = true;
  	bool run_flag = false;
  	
  	int cyclescompleted;                    // how many simulation cycles have been completed
  	
  	void OnExit(wxCommandEvent& event);     // event handler for exit menu item
  	void OnAbout(wxCommandEvent& event);    // event handler for about menu item
  	void ShowGrid(wxCommandEvent &event);
  	void ShowSettings(wxCommandEvent &event);
  	void ShowDialog(wxCommandEvent &event);
  	void OnSpin(wxSpinEvent& event);        // event handler for spin control
  	void SwitchList(wxCommandEvent &event);
  	void MonitorList(wxCommandEvent &event);
  	void OnVertZoomRelease(wxCommandEvent &event);
  	void OnHorzZoomRelease(wxCommandEvent &event);
  	void OnRunButton(wxCommandEvent& event);   // event handler for push button
  	void runnetwork(int ncycles);           // function to run the logic network
  	void OnContinueButton(wxCommandEvent &event);
  	void SaveCanvas(wxCommandEvent &event);
  	void OnStart(wxCommandEvent &event);
  	void OnEnd(wxCommandEvent &event);
  	void OnZoomIn(wxCommandEvent &event);
  	void OnZoomOut(wxCommandEvent &event);
  	void OnMouse(wxMouseEvent &event);
  	void ShowHelp(wxCommandEvent &event);
  	void closedHelp(wxCloseEvent &event);
  	
  	vector<dev> devList;
  	vector<string> switchNames;
  	vector<monDev> MList;
  	
  DECLARE_EVENT_TABLE()
};
    
class MyGLCanvas: public wxGLCanvas
{
 public:
  MyGLCanvas(wxWindow *parent,MyFrame *f ,wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
	     const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
	     const wxString& name = "MyGLCanvas", const wxPalette &palette=wxNullPalette); // constructor
  void Render(); // function to draw canvas contents
  
  void ShowGrid(bool show);
  void ZoomVert(int zoom);
  void ZoomHor(int zoom);
  void save_canvas();
  void mirror_char(unsigned char *pixels, int width, int height);
  void run(int cycles);
  void cont(int cycles);
  void montr();
  void goToStart();
  void goToEnd();
  
  
 private:
  MyFrame *frame;
  
  wxGLContext *context;              // OpenGL rendering context
  bool init;                         // has the OpenGL context been initialised?
  int pan_x;                         // the current x pan
  int pan_y;                         // the current y pan
  double zoom;                       // the current zoom
  int cyclesdisplayed;               // how many simulation cycles have been displayed
  monitor *mmz;                      // pointer to monitor class, used to extract signal traces
  names *nmz;                        // pointer to names class, used to extract signal names
  void InitGL();                     // function to initialise OpenGL context
  void OnSize(wxSizeEvent& event);   // event handler for when canvas is resized
  void OnPaint(wxPaintEvent& event); // event handler for when canvas is exposed
  void OnMouse(wxMouseEvent& event); // event handler for mouse events inside canvas
  
  void printGrid();
  void printTime();
  void maxNumber();

  void printSignals();
  
  // events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void dClick(wxKeyEvent& event);
	
	//keep track of monitored signals
	int nmonitor = 0;
	int currentTime = 0;
	vector< vector<asignal> > sigs; 
	vector<mons> monitoring;
	bool * monitored[]; 
	
	//used for testing
	int signals = 100;
	int SignalLength = 100;
	vector< vector<int> > signls; 
	void generateSignals();
	
	
	//time to start dispaying the signal
	int start_signal = 0;
	int end_signal = 10;
	
	bool showGrid;
	
	//variables used for printng
	int height;
	int width;
	
	float signal_height;
	float signal_width;
	
	int start_signal_time;
	int end_signal_time;
	
	int max_number_to_print;
	float space_between_signals = 20.0;
	
	int cycles;
	bool isSmall;
	
	int startAt = 0;
	int endAt = 10; 
	
	//for moving the start and end position of the signal
	bool mouse_left;
	bool mouse_right;
	
	void printRectangle();
	
	wxKeyboardState *kState;  

	
  DECLARE_EVENT_TABLE()
};
    
    
#endif /* gui_h */


