#include "gui.h"
#include <GL/glut.h>
#include "wx_icon.xpm"
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

// MyGLCanvas ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
	EVT_SIZE(MyGLCanvas::OnSize)
	EVT_PAINT(MyGLCanvas::OnPaint)
	//EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
	EVT_MOTION(MyGLCanvas::mouseMoved)
	EVT_LEFT_DOWN(MyGLCanvas::mouseDown)
	EVT_LEFT_UP(MyGLCanvas::mouseReleased)
	EVT_RIGHT_DOWN(MyGLCanvas::rightClick)
	EVT_LEAVE_WINDOW(MyGLCanvas::mouseLeftWindow)
	//EVT_SIZE(MyGLCanvas::resized)
	EVT_KEY_DOWN(MyGLCanvas::keyPressed)
	EVT_KEY_UP(MyGLCanvas::keyReleased)
	EVT_MOUSEWHEEL(MyGLCanvas::mouseWheelMoved)
	//EVT_PAINT(MyGLCanvas::render)
END_EVENT_TABLE()
  
int wxglcanvas_attrib_list[5] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

MyGLCanvas::MyGLCanvas(wxWindow *parent, wxWindowID id, monitor* monitor_mod, names* names_mod, const wxPoint& pos, 
		       const wxSize& size, long style, const wxString& name, const wxPalette& palette):
  wxGLCanvas(parent, id, wxglcanvas_attrib_list, pos, size, style, name, palette)
  // Constructor - initialises private variables
{
  context = new wxGLContext(this);
  mmz = monitor_mod;
  nmz = names_mod;
  init = false;
  pan_x = 0;
  pan_y = 0;
  zoom = 1.0;
  cyclesdisplayed = -1;
  
  generateSignals();//generate fake signals
  
  GetClientSize(&width, &height);// get the size of the frame
  
  //set default signal dimensions
  signal_height = 30.0;
  signal_width = 30.0;
  
  //set default signal start and end times
  start_signal_time = 0;
  end_signal_time = 100;
  
	max_number_to_print = floor((float)height/(signal_height + space_between_signals));
	
	start_signal = 0;
	end_signal = start_signal + max_number_to_print;
	
	showGrid = true;
}

void MyGLCanvas::Render(wxString example_text, int cycles)
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter and the first monitor
  // trace is displayed.
{
  float y;
  unsigned int i;
  asignal s;
  
  int w, h;
  GetClientSize(&w, &h);
  
   max_number_to_print = floor(height/(signal_height + space_between_signals));

    end_signal = start_signal + max_number_to_print;
  if (cycles >= 0) cyclesdisplayed = cycles;

  SetCurrent(*context);
  if (!init) {
    InitGL();
    init = true;
  }
  glClear(GL_COLOR_BUFFER_BIT);
  
  if(showGrid == true) printGrid();

  if ((cyclesdisplayed >= 0) && (mmz->moncount() > 0)) { // draw the first monitor signal, get trace from monitor class

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for (i=0; i<cyclesdisplayed; i++) {
      if (mmz->getsignaltrace(0, i, s)) {
		if (s==low) y = 10.0;
		if (s==high) y = 30.0;
		glVertex2f(20*i+10.0, y); 
		glVertex2f(20*i+30.0, y);
      }
    }
    glEnd();

  } else { // draw an artificial trace

	//set color
    glColor3f(0.0, 1.0, 0.0);
    
    wxString text;//text to print component name

  	wxPaintDC dc(this); // required for correct refreshing under MS windows


    int max = floor(h/(height));
    
    for(int i = start_signal; i < end_signal; i++)
    {
    	glColor3f(0.0, 0.0, 0.0); // color for text
    	glRasterPos2f(10,  (signal_height+space_between_signals)/2 + (i-start_signal)*(signal_height+space_between_signals));
    	wxString name;
    	name.Printf( "Dev %d", i);
    	
    	for (int k = 0; k < name.Len(); k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, name[k]);
    	
    	glColor3f(0.25, 0.25, 1.0);
    	glBegin(GL_LINE_STRIP);//draw line segments
    	for(int j = 0; j < 100; j++)
    	{
    		int l = 10.0;
    		if(signls[i][j] == 1) l += signal_height;
    		
    		glVertex2f(signal_width*j+60.0, l+((i-start_signal)*(signal_height+space_between_signals))); 
		  	glVertex2f(signal_width*j+60.0 + signal_width, l+((i-start_signal)*(signal_height+space_between_signals)));
		}    
		glEnd();	
	}
	/*
    
    for(int j = 0; j < 100; j++)
    {
    	glBegin(GL_LINE_STRIP);//draw line segments
		for (i=0; i<5; i++) {
		  if (i%2) y = 10.0;
		  else y = 30.0;
		  glVertex2f(20*i+10.0, y+(j*height)); 
		  glVertex2f(20*i+30.0, y+(j*height));
		}
		 
	}*/
   
    
  }

  // Example of how to use GLUT to draw text on the canvas
  //glColor3f(0.0, 0.0, 1.0);
  //glRasterPos2f(10, 100);
  //for (i = 0; i < example_text.Len(); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, example_text[i]);

  // We've been drawing to the back buffer, flush the graphics pipeline and swap the back buffer to the front
  glFlush();
  SwapBuffers();
}

void MyGLCanvas::generateSignals()
{
	vector<int> v;
	for(int i = 0; i < signals; i++)
	{
		for(int j = 0; j < 100; j++)
		{
			int n = (rand()%100);
			if(n >  50) v.push_back(1);
			else v.push_back(0);
		}
		signls.push_back(v);
		v.clear();	
	}
}

void MyGLCanvas::InitGL()
  // Function to initialise the GL context
{
  int w, h;

  GetClientSize(&w, &h);
  SetCurrent(*context);
  glDrawBuffer(GL_BACK);
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glViewport(0, 0, (GLint) w, (GLint) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1, 1); 
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated(pan_x, pan_y, 0.0);
  glScaled(zoom, zoom, zoom);
}

void MyGLCanvas::OnPaint(wxPaintEvent& event)
  // Event handler for when the canvas is exposed
{
  int w, h;
  wxString text;

  wxPaintDC dc(this); // required for correct refreshing under MS windows
  GetClientSize(&w, &h);
  text.Printf("Canvas redrawn by OnPaint event handler, canvas size is %d by %d", w, h);
  Render(text);
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
  // Event handler for when the canvas is resized
{
  GetClientSize(&width, &height);
  max_number_to_print = floor((float)height/(signal_height + space_between_signals));
  //cout << "max to print: " << max_number_to_print << endl;
  end_signal = start_signal + max_number_to_print;
  init = false;; // this will force the viewport and projection matrices to be reconfigured on the next paint
  
}

/*
void MyGLCanvas::OnMouse(wxMouseEvent& event)
  // Event handler for mouse events inside the GL canvas
{
  wxString text;
  int w, h;;
  static int last_x, last_y;

  GetClientSize(&w, &h);
  if (event.ButtonDown()) {
    last_x = event.m_x;
    last_y = event.m_y;
    text.Printf("Mouse button %d pressed at %d %d", event.GetButton(), event.m_x, h-event.m_y);
  }
  if (event.ButtonUp()) text.Printf("Mouse button %d released at %d %d", event.GetButton(), event.m_x, h-event.m_y);
  if (event.Dragging()) {
    pan_x += event.m_x - last_x;
    pan_y -= event.m_y - last_y;
    last_x = event.m_x;
    last_y = event.m_y;
    init = false;
    text.Printf("Mouse dragged to %d %d, pan now %d %d", event.m_x, h-event.m_y, pan_x, pan_y);
  }
  if (event.Leaving()) text.Printf("Mouse left window at %d %d", event.m_x, h-event.m_y);
  if (event.GetWheelRotation() < 0) {
    zoom = zoom * (1.0 - (double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
    init = false;
    text.Printf("Negative mouse wheel rotation, zoom now %f", zoom);
  }
  if (event.GetWheelRotation() > 0) {
    zoom = zoom / (1.0 + (double)event.GetWheelRotation()/(20*event.GetWheelDelta()));
    init = false;
    text.Printf("Positive mouse wheel rotation, zoom now %f", zoom);
  }

  if (event.GetWheelRotation() || event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render(text);
}
*/

void MyGLCanvas::ShowGrid(bool show)
{
	showGrid = show;
}

void MyGLCanvas::printGrid()
{
	  
	GetClientSize(&width, &height);
	for(int i = 0; i < floor(((float)width-60.0)/signal_width)+1; i ++)
	{
		glColor3f(240.0/255.0, 12.0/255.0, 39.0/255.0); //light gray color
		glRasterPos2f(60.0 + i*signal_width - signal_width/2, height - 10);
    	wxString name;
    	name.Printf( "%d", i);
    	for (int k = 0; k < name.Len(); k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, name[k]);
    	
    	glColor3f(0.90, 0.90, 0.90); //light gray color
		//draw time ticks
		glBegin(GL_LINE_STRIP);//draw line segments
		glVertex2f(60.0 + i*signal_width,0);
		glVertex2f(60.0 + i*signal_width,height);
		glEnd();
	}
	
	for(int j = 0; j < max_number_to_print;    j++)
	{
		//draw horizontal grid
		glBegin(GL_LINE_STRIP);//draw line segments
		glVertex2f(0, 10.0 + j *(signal_height + space_between_signals));
		glVertex2f(width, 10.0 + j * (signal_height + space_between_signals));
		glEnd();
	}
	
	
}

void MyGLCanvas::ZoomVert(int zoom)
{
	float scale;
	scale = (float)zoom/100.0;
	signal_height = signal_height*scale;
	space_between_signals = space_between_signals*scale;
	wxString str;
	str.Printf("");
	Render();
}

void MyGLCanvas::ZoomHor(int zoom)
{
	float scale;
	scale = (float)zoom/100.0;
	signal_width = signal_width*scale;

	wxString str;
	str.Printf("");
	Render();
}

// some useful events to use
void MyGLCanvas::mouseMoved(wxMouseEvent& event) {}
void MyGLCanvas::mouseDown(wxMouseEvent& event) {}

void MyGLCanvas::mouseWheelMoved(wxMouseEvent& event) 
{
	int r = event.GetWheelRotation();
	if(r<0) 
	{
		end_signal += 1;
		start_signal += 1;
	}
	else
	{
		 end_signal -=1;
		 start_signal -=1;
	 }
	
	
	if(start_signal < 0) 
	{
		start_signal = 0;
		end_signal = start_signal+max_number_to_print;
	}
	if(end_signal > 100)
	{
		end_signal = 100;
		start_signal = end_signal-max_number_to_print;
	}
	
	
	
	Render("Mouse Wheel Event", 20);
}

void MyGLCanvas::mouseReleased(wxMouseEvent& event) {}
void MyGLCanvas::rightClick(wxMouseEvent& event) {}
void MyGLCanvas::mouseLeftWindow(wxMouseEvent& event) {}
void MyGLCanvas::keyPressed(wxKeyEvent& event) {}
void MyGLCanvas::keyReleased(wxKeyEvent& event) {}

// MyFrame ///////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MyFrame::OnExit)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_BUTTON(MY_RUN_BUTTON_ID, MyFrame::OnRunButton)
  EVT_SPINCTRL(MY_SPINCNTRL_ID, MyFrame::OnSpin)
  EVT_TEXT_ENTER(MY_TEXTCTRL_ID, MyFrame::OnText)
  EVT_SLIDER(VERT_SLIDER_ID, MyFrame::OnVertZoomRelease)
  EVT_SLIDER(HORZ_SLIDER_ID, MyFrame::OnHorzZoomRelease)
  EVT_CHOICE(SWITCH_LIST_ID, MyFrame::SwitchListChoice)
  EVT_RADIOBUTTON(SWITCH_BUTTON_ID, MyFrame::OnToggle)
  EVT_CHECKLISTBOX(SWITCH_LISTBOX_ID, MyFrame::SwitchList)
  EVT_CHECKLISTBOX(MONITOR_LISTBOX_ID, MyFrame::MonitorList)
END_EVENT_TABLE()
 
MyFrame::MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size,
   	  names *names_mod, devices *devices_mod, monitor *monitor_mod, long style):
  wxFrame(parent, wxID_ANY, title, pos, size, style)
  // Constructor - initialises pointers to names, devices and monitor classes, lays out widgets
  // using sizers
{
  SetIcon(wxIcon(wx_icon));

  cyclescompleted = 0;
  nmz = names_mod;
  dmz = devices_mod;
  mmz = monitor_mod;
  if (nmz == NULL || dmz == NULL || mmz == NULL) {
	cout << "Cannot operate GUI without names, devices and monitor classes" << endl;
	exit(1);
  }

  wxMenu *fileMenu = new wxMenu;
  wxMenu *editMenu = new wxMenu;
  fileMenu->Append(wxID_ABOUT, "&About");
  fileMenu->Append(wxID_EXIT, "&Quit");
  editMenu->Append(wxID_ABOUT, "&About");
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  SetMenuBar(menuBar);

  wxBoxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
  canvas = new MyGLCanvas(this, wxID_ANY, monitor_mod, names_mod);
  topsizer->Add(canvas, 1, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 10);

  wxBoxSizer *button_sizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *run_cont_sizer = new wxBoxSizer(wxVERTICAL);


  //wxBoxSizer *vert_slider_sizer = new wxBoxSizer(wxHORIZONTAL);

  //button_sizer->Add(new wxStaticText(this, wxID_ANY, "                                       "), 0, wxEXPAND);

  button_sizer->Add(new wxStaticText(this, wxID_ANY, "Number of Cycles"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

  spin = new wxSpinCtrl(this, MY_SPINCNTRL_ID, wxString("10"));
  button_sizer->Add(spin, 0 , wxALIGN_CENTER_HORIZONTAL|wxALL, 10);

  //vector<wxChar*> vectest;

  //vectest.push_back(wxT("SW1"));
  //vectest.push_back(wxT("SW2"));

  button_sizer->Add(new wxStaticText(this, wxID_ANY, "Switch State"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

  wxString *switch_list = new wxString[6];
  //switch_list[0] = vectest[0];
  //switch_list[1] = vectest[1];
  switch_list[0] = wxT("SW1");
  switch_list[1] = wxT("SW2");
  switch_list[2] = wxT("SW3");
  switch_list[3] = wxT("SW4");
  switch_list[4] = wxT("SW5");
  switch_list[5] = wxT("SW6");


  wxString *monitor_list = new wxString[6];
  monitor_list[0] = wxT("M1");
  monitor_list[1] = wxT("M2");
  monitor_list[2] = wxT("M3");
  monitor_list[3] = wxT("M4");
  monitor_list[4] = wxT("M5");
  monitor_list[5] = wxT("M6");


  wxCheckListBox *toggle_list = new wxCheckListBox(this, SWITCH_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), 6, switch_list);

  button_sizer->Add(toggle_list, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

  button_sizer->Add(new wxStaticText(this, wxID_ANY, "Monitor Points"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

  wxCheckListBox *monitor_list1 = new wxCheckListBox(this, MONITOR_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), 6, monitor_list);

  button_sizer->Add(monitor_list1, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);


  button_sizer->Add(new wxStaticText(this, wxID_ANY, "Vertical Zoom"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);
   
  button_sizer->Add(vert_zoom_value, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

  vert_zoom_slider = new wxSlider(this, VERT_SLIDER_ID, 50, 0, 100, 
      wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
  
  button_sizer->Add(vert_zoom_slider, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 10);

  button_sizer->Add(new wxStaticText(this, wxID_ANY, "Horizontal Zoom"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);
  
  button_sizer->Add(horz_zoom_value, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

  horz_zoom_slider = new wxSlider(this, HORZ_SLIDER_ID, 50, 0, 100, 
      wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

  button_sizer->Add(horz_zoom_slider, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 10);

  run_cont_sizer->Add(new wxButton(this, MY_RUN_BUTTON_ID, "Run"), 0, wxALL, 10);

  run_cont_sizer->Add(new wxButton(this, wxID_ANY, "Continue"), 0, wxALL, 10);
	 
  button_sizer->Add(run_cont_sizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM|wxBOTTOM, 10);

  //button_sizer->Add(new wxTextCtrl(this, MY_TEXTCTRL_ID, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0 , wxALL, 10);

  topsizer->Add(button_sizer, 0, wxALIGN_TOP|wxEXPAND);

  SetSizeHints(400, 400);
  SetSizer(topsizer);
}

void MyFrame::OnExit(wxCommandEvent &event)
  // Event handler for the exit menu item
{
  Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &event)
  // Event handler for the about menu item
{
  wxMessageDialog about(this, "Example wxWidgets GUI\nAndrew Gee\nJune 2014", "About Logsim", wxICON_INFORMATION | wxOK);
  about.ShowModal();
}


void MyFrame::OnRunButton(wxCommandEvent &event)
  // Event handler for the push button
{
  int n, ncycles;

  cyclescompleted = 0;
  dmz->initdevices ();
  mmz->resetmonitor ();
  runnetwork(spin->GetValue());
  canvas->Render("Run button pressed", cyclescompleted);
}

void MyFrame::OnSpin(wxSpinEvent &event)
  // Event handler for the spin control
{
  wxString text;

  text.Printf("New spinctrl value %d lsdkjfls", event.GetPosition());
  canvas->Render(text);
}

void MyFrame::OnText(wxCommandEvent &event)
  // Event handler for the text entry field
{
  wxString text;

  text.Printf("New text entered %s", event.GetString().c_str());
  canvas->Render(text);
}

void MyFrame::runnetwork(int ncycles)
  // Function to run the network, derived from corresponding function in userint.cc
{
  bool ok = true;
  int n = ncycles;

  while ((n > 0) && ok) {
	dmz->executedevices (ok);
	if (ok) {
  	n--;
  	mmz->recordsignals ();
	} else
  	cout << "Error: network is oscillating" << endl;
  }
  if (ok) cyclescompleted = cyclescompleted + ncycles;
  else cyclescompleted = 0;
}

void MyFrame::OnVertZoomRelease(wxCommandEvent &event)

{

  int vert_sliderpos = vert_zoom_slider->GetValue(); 

  float vert_zoom_fl;
  int vert_zoom_int;

  if(vert_sliderpos <= 50) {

    vert_zoom_fl = (1.5*vert_sliderpos)+25.0;
    vert_zoom_int = floor(vert_zoom_fl);

  }

  if(vert_sliderpos > 50) {

    vert_zoom_fl = 6.0*((1.0*vert_sliderpos)-50.0)+100.0;
    vert_zoom_int = floor(vert_zoom_fl);

  }

  string vert_zoom_str = "x" + to_string(vert_zoom_int) + "%";
  wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);

  vert_zoom_value->SetLabel(vert_zoom_wxstr);

}

void MyFrame::OnHorzZoomRelease(wxCommandEvent &event)

{

  int horz_sliderpos = horz_zoom_slider->GetValue(); 

  float horz_zoom_fl;
  int horz_zoom_int;

  if(horz_sliderpos <= 50) {

    horz_zoom_fl = (1.5*horz_sliderpos)+25.0;
    horz_zoom_int = floor(horz_zoom_fl);

  }

  if(horz_sliderpos > 50) {

    horz_zoom_fl = 6.0*((1.0*horz_sliderpos)-50.0)+100.0;
    horz_zoom_int = floor(horz_zoom_fl);

  }

  string horz_zoom_str = "x" + to_string(horz_zoom_int) + "%";
  wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);

  horz_zoom_value->SetLabel(horz_zoom_wxstr);

}

void MyFrame::SwitchListChoice(wxCommandEvent &event)
{

}

void MyFrame::OnToggle(wxCommandEvent &event)
{

} 

void MyFrame::SwitchList(wxCommandEvent &event)
{

} 


void MyFrame::MonitorList(wxCommandEvent &event)
{

}
