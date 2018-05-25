#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <string>
#include <vector>

#include "names.h"
#include "devices.h"
#include "monitor.h"

enum { 
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_RUN_BUTTON_ID,
  VERT_SLIDER_ID,
  HORZ_SLIDER_ID,
  SWITCH_LIST_ID,
  SWITCH_BUTTON_ID,
  SWITCH_LISTBOX_ID,
  MONITOR_LISTBOX_ID,
}; // widget identifiers

class MyGLCanvas;

class MyFrame: public wxFrame
{
 public:
  MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, 
	  names *names_mod = NULL, devices *devices_mod = NULL, monitor *monitor_mod = NULL, 
	  long style = wxDEFAULT_FRAME_STYLE); // constructor
 private:
  MyGLCanvas *canvas;                     // OpenGL drawing area widget to draw traces
  wxSpinCtrl *spin;                       // control widget to select the number of cycles
  names *nmz;                             // pointer to names class
  devices *dmz;                           // pointer to devices class
  monitor *mmz;                           // pointer to monitor class
  wxSlider *vert_zoom_slider;
  wxSlider *horz_zoom_slider;
  int cyclescompleted;                    // how many simulation cycles have been completed
  void runnetwork(int ncycles);           // function to run the logic network
  void OnExit(wxCommandEvent& event);     // event handler for exit menu item
  void OnAbout(wxCommandEvent& event);    // event handler for about menu item
  void OnRunButton(wxCommandEvent& event);   // event handler for push button
  void OnSpin(wxSpinEvent& event);        // event handler for spin control
  void OnText(wxCommandEvent& event);     // event handler for text entry field
  void OnVertZoomRelease(wxCommandEvent &event);
  void OnHorzZoomRelease(wxCommandEvent &event);
  void SwitchListChoice(wxCommandEvent &event);
  void OnToggle(wxCommandEvent &event);	
  void SwitchList(wxCommandEvent &event);
  void MonitorList(wxCommandEvent &event);
  wxStaticText *vert_zoom_value = new wxStaticText(this, wxID_ANY, "x100%");
  wxStaticText *horz_zoom_value = new wxStaticText(this, wxID_ANY, "x100%");
  
  DECLARE_EVENT_TABLE()
};
    
class MyGLCanvas: public wxGLCanvas
{
 public:
  MyGLCanvas(wxWindow *parent, wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
	     const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
	     const wxString& name = "MyGLCanvas", const wxPalette &palette=wxNullPalette); // constructor
  void Render(wxString example_text = "", int cycles = -1); // function to draw canvas contents
  
  void ShowGrid(bool show);
  void ZoomVert(int zoom);
  void ZoomHor(int zoom);
  
 private:
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
  void maxNumber();
  
  // events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	
	int signals = 100;
	vector< vector<int> > signls; 
	void generateSignals();
	
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
	
	
  DECLARE_EVENT_TABLE()
};
    
#endif /* gui_h */
