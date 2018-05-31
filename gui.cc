#include "gui.h"
#include <GL/glut.h>
#include "wx_icon.xpm"
#include <iostream>
#include <cstdlib>
#include <string>

#define SIGNAL_HEIGHT 30
#define SIGNAL_WIDTH 30
#define SIGNAL_SPACE 20
#define NAME_SPACE 120
#define INVALID_SIGNAL -1
using namespace std;

// MyGLCanvas ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
	//EVT_LEFT_DCLICK(MyGLCanvas::dClick)
	EVT_SIZE(MyGLCanvas::OnSize)
	EVT_PAINT(MyGLCanvas::OnPaint)
	EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
	//EVT_MOTION(MyGLCanvas::mouseMoved)
	//EVT_LEFT_DOWN(MyGLCanvas::mouseDown)
	//EVT_LEFT_UP(MyGLCanvas::mouseReleased)
	//EVT_RIGHT_DOWN(MyGLCanvas::rightClick)
	//EVT_LEAVE_WINDOW(MyGLCanvas::mouseLeftWindow)
	//EVT_SIZE(MyGLCanvas::resized)
	EVT_KEY_DOWN(MyGLCanvas::keyPressed)
	EVT_KEY_UP(MyGLCanvas::keyReleased)
	//EVT_MOUSEWHEEL(MyGLCanvas::mouseWheelMoved)
	//EVT_PAINT(MyGLCanvas::render)
	
END_EVENT_TABLE()
  
int wxglcanvas_attrib_list[5] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

MyGLCanvas::MyGLCanvas(wxWindow *parent,MyFrame *f ,wxWindowID id, monitor* monitor_mod, names* names_mod, const wxPoint& pos, 
		       const wxSize& size, long style, const wxString& name, const wxPalette& palette):
  wxGLCanvas(parent, id, wxglcanvas_attrib_list, pos, size, style, name, palette)
  // Constructor - initialises private variables
{
  context = new wxGLContext(this);
  mmz = monitor_mod;
  nmz = names_mod;
  frame = f;
  init = false;
  pan_x = 0;
  pan_y = 0;
  zoom = 1.0;
  cyclesdisplayed = -1;
  
  kState = new wxKeyboardState(true,true,true,true);
  
  montr(); // get monitors
  
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
	isSmall = false;
	
	mouse_left = false;
	mouse_right = false;
}

void MyGLCanvas::Render()
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter and the first monitor
  // trace is displayed.
{
  float y;
  unsigned int i;
  asignal s;
  
  int w, h;
  GetClientSize(&width, &height);
  
	
  
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
  if(mouse_left == true || mouse_right == true) printRectangle();
  printTime();

  if (currentTime!= 0) 
  { // draw the first monitor signal, get trace from monitor class

	glColor3f(0.0, 1.0, 0.0);
    
    wxString text;//text to print component name

  	wxPaintDC dc(this); // required for correct refreshing under MS windows


    int max = floor(h/(height));
    
    for(int i = start_signal; i < nmonitor; i++) //TODO change start and end signal!!!!!!!
    {
    	glColor3f(0.0, 0.0, 0.0); // color for text
    	if(signal_height/SIGNAL_HEIGHT < 0.37)
    	{
    	glRasterPos2f(10,  (signal_height+space_between_signals)/2 + (i-start_signal)*(signal_height+space_between_signals)+5);
    	}
    	else
    	{
    	glRasterPos2f(10,  (signal_height+space_between_signals)/2 + (i-start_signal)*(signal_height+space_between_signals));
    	}
    	wxString nam;
    	
    	if(monitoring[i].pinId == nmz->cvtname("Q"))
    	{
    		nam.Printf("%s%s", monitoring[i].nme , ".Q");
		}
    	else if(monitoring[i].pinId == nmz->cvtname("QBAR"))
    	{
    		nam.Printf("%s%s",monitoring[i].nme , ".QBAR");
		}
		else
		{
			nam.Printf("%s",monitoring[i].nme);
		}
		
		
		if(signal_height/SIGNAL_HEIGHT < 0.65)
		{
    		for (int k = 0; k < nam.Len(); k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, nam[k]);
    	}
    	else
    	{
    		for (int k = 0; k < nam.Len(); k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, nam[k]);
    	}
    	glColor3f(0.25, 0.25, 1.0);
    
    	//TODO put glBegin in for loop for changing monitors
    	glBegin(GL_LINE_STRIP);//draw line segments
    	for(int j = startAt; j < currentTime; j++) 
    	{
    		glColor3f(0.25, 0.25, 1.0);
    		int l = 10.0;
    		if(sigs.size() < i) break;
    		if(sigs[i].size() < j) continue;
    		if(sigs[i][j] == high) l += signal_height;
    		if(j < monitoring[i].startTime) continue;
    		
    		glVertex2f(signal_width*(j-startAt)+NAME_SPACE, l+((i-start_signal)*(signal_height+space_between_signals))); 
		  	glVertex2f(signal_width*(j-startAt)+ NAME_SPACE + signal_width, l+((i-start_signal)*(signal_height+space_between_signals)));
		  	
		}    
		glEnd();	
	}
	
	} 
	/*
  else { // draw an artificial trace

	//set color
    glColor3f(0.0, 1.0, 0.0);
    
    wxString text;//text to print component name

  	wxPaintDC dc(this); // required for correct refreshing under MS windows


    int max = floor(h/(height));
    int i = start_signal;
    if(i < 0) start_signal = 0;
    for(i ; i < end_signal; i++)
    {
    	glColor3f(0.0, 0.0, 0.0); // color for text
    	glRasterPos2f(10,  (signal_height+space_between_signals)/2 + (i-start_signal)*(signal_height+space_between_signals));
    	wxString name;
    	name.Printf( "Dev %d", i);
    	
    	for (int k = 0; k < name.Len(); k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, name[k]);
    	
    	glColor3f(0.25, 0.25, 1.0);
    	glBegin(GL_LINE_STRIP);//draw line segments
    	for(int j = startAt; j < 100; j++)
    	{
    		int l = 10.0;
    		cout << "i = " << i << " ,j = " << j << endl;
    		if(signls[i][j] == 1) l += signal_height;
    		
    		glVertex2f(signal_width*(j-startAt)+NAME_SPACE, l+((i-start_signal)*(signal_height+space_between_signals))); 
		  	glVertex2f(signal_width*(j-startAt)+ NAME_SPACE + signal_width, l+((i-start_signal)*(signal_height+space_between_signals)));
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
		 
	}
   
    
  }*/

  // Example of how to use GLUT to draw text on the canvas
  //glColor3f(0.0, 0.0, 1.0);
  //glRasterPos2f(10, 100);
  //for (i = 0; i < example_text.Len(); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, example_text[i]);

  // We've been drawing to the back buffer, flush the graphics pipeline and swap the back buffer to the front
  
  
  
  glFlush();
  SwapBuffers();
}


void MyGLCanvas::run(int cycles)
{
	
	mons currentMonitor;
	asignal s;
	currentTime = 0;
	
	monitoring.clear();
	sigs.clear();
	montr();
	vector<asignal> sv;
	
	for(int i=0; i<nmonitor; i++)
	{
		currentMonitor = monitoring[i];
		sv.clear();
		for(int j = 0; j < cycles; j++)
		{
			if(mmz->getsignaltrace(i,j,s))
			{
				sv.push_back(s); 
			}
			else
			{
				sv.push_back(invalid_signal);
			}
			//cout << "j = " << j << " , s = " << s << endl;
		}
		sigs.push_back(sv);
	}
	
	currentTime = cycles;
	
	endAt = currentTime;
	
	startAt = endAt - floor((width-NAME_SPACE)/signal_width);
	if(startAt < 0) startAt = 0;
	
	Render();
}

void MyGLCanvas::printSignals()
{	
	for(int i = 0; i < nmonitor; i++)
	{
		cout << "signal "  << i << endl;
		for(int j = 0; j < currentTime; j++)
		{
			cout << sigs[i][j] << ", ";
		}
		cout << endl;
	}
}

void MyGLCanvas::cont(int cycles)
{

	mons currentMonitor;
	asignal s;
	vector<asignal> sv;
	sigs.clear();
	
	
	//bool monitored_before[old_m.size()];
	
	
	for(int i=0; i<nmonitor; i++)
	{
		currentMonitor = monitoring[i];
		sv.clear();
		
		
		for(int j = 0; j < currentTime + cycles; j++)
		{
			
			if(mmz->getsignaltrace(i,j,s))
			{
				sv.push_back(s); 
			}
		}
		
	sigs.push_back(sv);
	}
	
	
	currentTime += cycles;
	
	GetClientSize(&width, &height);
	
	
	//endAt = currentTime;
	
	//startAt = endAt - floor((width-NAME_SPACE)/signal_width);
	
	//if(startAt < 0) startAt = 0;
	endAt += cycles;
	
	if(endAt - floor((width-NAME_SPACE)/signal_width) > startAt) startAt = endAt -floor((width-NAME_SPACE)/signal_width);
	
	
	Render();

}

void MyGLCanvas::montr()
{
	nmonitor = mmz->moncount();
	mons currentMonitor;
	
	//vector<mons> old_m = monitoring;
	
	vector<mons> newMons;
	
	for(int i = 0; i < nmonitor; i++)
	{
		currentMonitor.number = i;
		mmz->getmonname(currentMonitor.number, currentMonitor.devId, currentMonitor.pinId);
		
		currentMonitor.nme = nmz->getName(currentMonitor.devId);
		
		currentMonitor.startTime = currentTime;
		
		
		
		/*if(i < monitoring.size())
		{
			if(monitoring[i].number == currentMonitor.number) continue;
		}*/
		
		newMons.push_back(currentMonitor);
	}
	
	for(int i = 0; i < newMons.size();i++)
	{
		for(int j = 0; j < monitoring.size();j++)
		{
			if((newMons[i].nme == monitoring[j].nme) && (newMons[i].pinId == monitoring[j].pinId))
			{
				newMons[i].startTime = monitoring[j].startTime;
			}
		}	
	}
	monitoring.clear();
	monitoring = newMons;
	
	/*
	monitored = new bool[monitoring.size()];
	for(int i = 0; i < monitoring.size())
	{
		monitored[i] = false;
	}*/
	
	/*
	for(int i = 0; i < old_m.size();i++)
	{
		for(int j = 0; j < monitoring.size(); j++)
		{
			if(old[i].nme == monitoring[j].nme) 
			{	
				monitored[i] = true;
				break;
			}
		}
	}*/
	
}

void MyGLCanvas::printRectangle()
{
	GetClientSize(&width, &height);

	float x1,y1,x2,y2;
	
	if(mouse_left)
	{
		x1 = 0;
		y1 = 0;
		x2 = NAME_SPACE;
		y2 = height;
	}
	else
	{
		x1 = width - NAME_SPACE;
		y1 = 0;
		x2 = width;
		y2 = height;
	}
	
	
	glColor4f(0.9,0.9 ,0.9,0.25); // set low opacity for transparency
	glRectf(x1,y1,x2,y2);
	
	glColor4f(0.945,0.561,0.408,0.25);
	if(mouse_left)
	{
		x1 = 40;
		y1 = height / 2 - 5;
		x2 = 25;
		y2 = height/2 + 5;
		
		glRectf(x1,y1,x2,y2);
		
		glBegin(GL_TRIANGLES);
		
		glVertex2f(x2, y1-5);
		glVertex2f(x2, y2+5);
		glVertex2f(x2-10,height/2);
		
		glEnd();
		
	}
	else
	{
		x1 = width - 40;
		y1 = height / 2 - 5;
		x2 = width - 25;
		y2 = height/2 + 5;
		
		glRectf(x1,y1,x2,y2);
		
		glBegin(GL_TRIANGLES);
		
		glVertex2f(x2, y1-5);
		glVertex2f(x2, y2+5);
		glVertex2f(x2+10,height/2);
		
		glEnd();
	}
	
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
  Render();
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
  // Event handler for when the canvas is resized
{
  GetClientSize(&width, &height);
  max_number_to_print = floor((float)height/(signal_height + space_between_signals));
  //cout << "max to print: " << max_number_to_print << endl;
  end_signal = start_signal + max_number_to_print;
  if(end_signal>nmonitor) end_signal = nmonitor;
  init = false;; // this will force the viewport and projection matrices to be reconfigured on the next paint
  
  	
	endAt = currentTime;
	
	startAt = endAt - floor((width-NAME_SPACE)/signal_width);
	if(startAt < 0) startAt = 0;

}



void MyGLCanvas::ShowGrid(bool show)
{
	showGrid = show;
	Render();
}

void MyGLCanvas::printTime()
{
	glColor3f(240.0/255.0, 12.0/255.0, 39.0/255.0); //light gray color
	for(int i = startAt; i < floor(((float)width-NAME_SPACE)/signal_width)+1+startAt; i ++)
	{
		if(isSmall == true && (i%5) != 0) continue; //continue to next cycle if the size is small and i is not a multiple of 5
		
		if(isSmall) glRasterPos2f(NAME_SPACE + (i-startAt)*signal_width - 7.5, height - 10);
		else glRasterPos2f(NAME_SPACE + (i-startAt)*signal_width - 7.5, height - 10);
		
		wxString name;
		name.Printf( "%d", i);
		for (int k = 0; k < name.Len(); k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, name[k]);
	}
}

void MyGLCanvas::printGrid()
{
	  
	GetClientSize(&width, &height);
	for(int i = 0; i < floor(((float)width-NAME_SPACE)/signal_width)+1; i ++)
	{
		
    	glColor3f(0.90, 0.90, 0.90); //light gray color
    	if( i%5 == 0 && isSmall == true) glColor3f(0.5, 0.5, 0.5);
		//draw time ticks
		glBegin(GL_LINE_STRIP);//draw line segments
		glVertex2f(NAME_SPACE + i*signal_width,0);
		glVertex2f(NAME_SPACE + i*signal_width,height);
		glEnd();
	}
	
	//glColor3f(0.70, 0.70, 0.70); //darker light gray color
	for(int j = 0; j < max_number_to_print+1;    j++)
	{
		//draw horizontal grid
		glBegin(GL_LINE_STRIP);//draw line segments
		glVertex2f(0, 10.0 + j *(signal_height + space_between_signals));
		glVertex2f(width, 10.0 + j * (signal_height + space_between_signals));
		glEnd();
	}
	
	
}


// Function to create a wxImage from the canvas contents and save it as a PNG image in a location defined by the user using a wxFileDialog

void MyGLCanvas::save_canvas()
{		
	unsigned char *pixels;
  	wxImage *save_im;

  	GetClientSize(&width, &height);
  	
  	// Extract pixels from the canvas
  	
  	pixels = (unsigned char *) malloc(3 * width * height);
  	glPixelStorei(GL_PACK_ALIGNMENT, 1);
  	glReadBuffer(GL_BACK_RIGHT);
  	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// This mirrors the unsigned char pixels to ensure that the resulting image is in the right orientation after the subsequent mirroring

	mirror_char(pixels, width, height);
	
  	save_im = new wxImage(width, height);
  	save_im->SetData(pixels, width, height, false);
  	
  	// Removing this mirror would cause a segmentation fault
  	
  	save_im->Mirror(false);
  	
  	wxFileDialog *save_path_dialog = new wxFileDialog(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE);
  	save_path_dialog->ShowModal();
  	
  	// Only attempt to save the image if a name is given by the user
  	
  	wxString save_path;
  	save_path.Printf("");
  	//wxString *save_path = new wxString(wxT(""));
  	save_path = save_path_dialog->GetPath();
  	string path = string(save_path.mb_str());
  	size_t pos;
  	if(!(save_path.IsEmpty()))
  	{
  		pos = path.find(".png",0);
  		if(pos == string::npos)
  		{
  			pos = path.find(".",0);
  			if(pos == string::npos)
  			{
  				path = path + ".png";
  			}
  			else
  			{
  				int len = path.length();
  				path.erase(pos, len-pos);
  				path = path + ".png";
  			}
  		}
  		wxString s_path(path);
  		save_im->SaveFile(s_path, wxBITMAP_TYPE_PNG);
  	}
}

// Mirror the image by flipping the unsigned char *

void MyGLCanvas::mirror_char(unsigned char *pixels, int width, int height)
{
    int rows = height / 2;
    int row_stride = width * 3;
    unsigned char* temp_row = (unsigned char*)malloc(row_stride);

    int source_offset, target_offset;

    for (int rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        source_offset = rowIndex * row_stride;
        target_offset = (height - rowIndex - 1) * row_stride;

        memcpy(temp_row, pixels + source_offset, row_stride);
        memcpy(pixels + source_offset, pixels + target_offset, row_stride);
        memcpy(pixels + target_offset, temp_row, row_stride);
    }
    free(temp_row);
    temp_row = NULL;
}


void MyGLCanvas::ZoomVert(int zoom)
{
	float scale;
	scale = (float)zoom/100.0;
	signal_height = SIGNAL_HEIGHT*scale;
	space_between_signals = SIGNAL_SPACE*scale;
	wxString str;
	str.Printf("");
	
	GetClientSize(&width, &height);
 	 max_number_to_print = floor((float)height/(signal_height + space_between_signals));
  	//cout << "max to print: " << max_number_to_print << endl;
  	end_signal = start_signal + max_number_to_print;
  	
  	if(start_signal < 0) 
	{
		start_signal = 0;
		end_signal = start_signal+max_number_to_print;
	}
	if(end_signal > nmonitor)
	{
		end_signal = nmonitor;
		start_signal = end_signal-max_number_to_print;
	}
	if(start_signal<0)
	{
		start_signal = 0;
	}
  
	Render();
}

void MyGLCanvas::ZoomHor(int zoom)
{
		
	
	float scale;
	scale = (float)zoom/100.0;
	signal_width = SIGNAL_WIDTH*scale;
	
	if(scale <= 0.6)
	{
		isSmall = true;
	}
	else
	{
		isSmall = false;
	}
	
	wxString str;
	str.Printf("");
	
	
	Render();
}

void MyGLCanvas::goToStart()
{
	GetClientSize(&width, &height);
 	
 	startAt = 0;
 	endAt = startAt + floor((width-NAME_SPACE)/signal_width);;
 	if(endAt > currentTime)
 	{
 		endAt = currentTime;
 	}
 	Render();
}

void MyGLCanvas::goToEnd()
{
	GetClientSize(&width, &height);

 	endAt = currentTime;
 	startAt = currentTime - floor((width-NAME_SPACE)/signal_width);;
 	if(startAt < 0) startAt = 0;
 	
 	Render();
}


void MyGLCanvas::OnMouse(wxMouseEvent& event)
  // Event handler for mouse events inside the GL canvas
{
	wxString str;
	str.Printf("");	
	int w, h;;
	static int last_x, last_y;

	bool dc = event.LeftDClick();
	if(dc)
	{
		if(mouse_right)
		{
			startAt+=9;
		}
		if(mouse_left)
		{
			startAt-=9;
		}
	}

	bool sc = event.LeftDown();
	if(sc)
	{
		if(mouse_right)
		{
			startAt++;
		}
		if(mouse_left)
		{
			startAt--;
		}
	}
	
	
	
	int r = event.GetWheelRotation();
	if(r > 0 && event.AltDown() == true)
	{
		startAt++;
	}
	else if(r < 0 && event.AltDown() == true)
	{
		startAt--;
	}
	else if(r>0 &&  event.ControlDown() == true) 
	{
		end_signal += 1;
		start_signal += 1;
	}
	else if(r < 0  &&  event.ControlDown() == true)
	{
		 end_signal -=1;
		 start_signal -=1;
	}
	else if(r > 0)
	{
		frame->zoom(2);
		
		endAt = currentTime;
		startAt = endAt - floor((width-NAME_SPACE)/signal_width);
		if(startAt < 0) startAt = 0;
	}
	else if(r < 0)
	{
		frame->zoom(-2);
		
		endAt = currentTime;
		startAt = endAt - floor((width-NAME_SPACE)/signal_width);
		if(startAt < 0) startAt = 0;
	}
	
	
	
	if(end_signal > nmonitor)
	{
		end_signal = nmonitor;
		start_signal = end_signal-max_number_to_print;
	}
	if(start_signal < 0) 
	{
		start_signal = 0;
		end_signal = start_signal+max_number_to_print;
	}
	bool mv = event.Moving();
	 
	if(mv)
	{
		static int last_x, last_y;
	 
		last_x = event.m_x;
		last_y = event.m_y;
		
		if(last_x < width * 0.15) 
		{
			mouse_left = true;
			mouse_right = false;
		}
		else if(last_x > width*0.85) 
		{
			mouse_left = false;
			mouse_right = true;
		}
		else
		{
			mouse_left = false;
			mouse_right = false;
		}
	}
	
	 if(event.Leaving())
	 {
	 	mouse_left = false;
		mouse_right = false;
	 }
	
	 Render();
/*
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

  if (event.GetWheelRotation() || event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render(text);*/
}

/*

// some useful events to use
void MyGLCanvas::mouseMoved(wxMouseEvent& event) 
{
	wxString str;
	str.Printf("");	
	
	GetClientSize(&width, &height);
	
	static int last_x, last_y;
	 
	last_x = event.m_x;
    last_y = event.m_y;
    
    if(last_x < width * 0.15) 
    {
    	mouse_left = true;
    	mouse_right = false;
    	Render(str,-1);
	}
    else if(last_x > width*0.85) 
    {
    	mouse_left = false;
    	mouse_right = true;
    	Render(str, -1);
	}
    else
    {
    	mouse_left = false;
    	mouse_right = false;
    	Render(str,-1);
    }
}
void MyGLCanvas::mouseDown(wxMouseEvent& event) 
{

}



void MyGLCanvas::mouseWheelMoved(wxMouseEvent& event) 
{
	int r = event.GetWheelRotation();
	if(r>0) 
	{
		end_signal += 1;
		start_signal += 1;
	}
	else if(r > 0)
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

void MyGLCanvas::mouseReleased(wxMouseEvent& event) {
	wxString str;
	str.Printf("");	
	bool dc = event.LeftDClick();
	if(dc)
	{
		if(mouse_right)
		{
			startAt+=10;
		}
		if(mouse_left)
		{
			startAt-=10;
		}
	}
	else
	{
		if(mouse_right)
		{
			startAt++;
		}
		if(mouse_left)
		{
			startAt--;
		}
	}
    
    if(startAt<0) startAt = 0;
    else if(startAt > SignalLength) startAt = SignalLength;
    
    Render(str,-1);
}
void MyGLCanvas::rightClick(wxMouseEvent& event) {}
*/
void MyGLCanvas::mouseLeftWindow(wxMouseEvent& event) {}
void MyGLCanvas::keyPressed(wxKeyEvent& event) {}
void MyGLCanvas::keyReleased(wxKeyEvent& event) {}
// MyFrame ///////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MyFrame::OnExit)
	EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
	EVT_MENU(SHOW_GRID_ID, MyFrame::ShowGrid)
	EVT_MENU(SHOW_SETTINGS_ID, MyFrame::ShowSettings)
	EVT_MENU(SHOW_DIALOG_ID, MyFrame::ShowDialog)
	EVT_MENU(MY_FILE_RUN_ID, MyFrame::OnRunButton)
	EVT_MENU(MY_FILE_CONTINUE_ID, MyFrame::OnContinueButton)
	EVT_MENU(MY_FILE_SAVE_ID, MyFrame::SaveCanvas)
	EVT_MENU(START_MENU_ID, MyFrame::OnStart)
	EVT_MENU(END_MENU_ID, MyFrame::OnEnd)
	EVT_MENU(ZOOM_IN_MENU_ID, MyFrame::OnZoomIn)
	EVT_MENU(ZOOM_OUT_MENU_ID, MyFrame::OnZoomOut)
	EVT_MENU(HELP_MENU_ID, MyFrame::ShowHelp)
	EVT_SPINCTRL(MY_SPINCNTRL_ID, MyFrame::OnSpin)
	EVT_CHECKLISTBOX(SWITCH_LISTBOX_ID, MyFrame::SwitchList)
	EVT_CHECKLISTBOX(MONITOR_LISTBOX_ID, MyFrame::MonitorList)
	EVT_SLIDER(VERT_SLIDER_ID, MyFrame::OnVertZoomRelease)
	EVT_SLIDER(HORZ_SLIDER_ID, MyFrame::OnHorzZoomRelease)  
	EVT_BUTTON(MY_RUN_BUTTON_ID, MyFrame::OnRunButton)
	EVT_TOOL(RUN_TOOLBAR_ID, MyFrame::OnRunButton)
	EVT_BUTTON(MY_CONTINUE_BUTTON_ID, MyFrame::OnContinueButton)
	EVT_TOOL(CONTINUE_TOOLBAR_ID, MyFrame::OnContinueButton)
	EVT_TOOL(SAVE_TOOLBAR_ID, MyFrame::SaveCanvas)
	EVT_TOOL(START_TOOLBAR_ID, MyFrame::OnStart)
	EVT_TOOL(END_TOOLBAR_ID, MyFrame::OnEnd)
	EVT_MOUSEWHEEL(MyFrame::OnMouse)
END_EVENT_TABLE()
 
MyFrame::MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size,
   	  names *names_mod, devices *devices_mod, monitor *monitor_mod,parser* parser_mod, long style):
	wxFrame(parent, wxID_ANY, title, pos, size, style)
	// Constructor - initialises pointers to names, devices and monitor classes, lays out widgets
  	// using sizers
{
	
  	SetIcon(wxIcon(wx_icon));
	wxInitAllImageHandlers();

	kState = new wxKeyboardState(true,true,true,true);
  	cyclescompleted = 0;
  	nmz = names_mod;
  	dmz = devices_mod;
  	mmz = monitor_mod;
  	prs = parser_mod;
  	
  	
  	if (nmz == NULL || dmz == NULL || mmz == NULL|| prs == NULL) {
		cout << "Cannot operate GUI without names, devices, parser and monitor classes" << endl;
		exit(1);
  	}
  	
  	devList = prs->getDevList();
  	
  	//create list of switches
  	int c = 0;
  	for(int i = 0; i < devList.size();i++)
  	{
  		if(devList[i].kind == aswitch) c++ ;
  	}
  	switch_list = new wxString[c];
  	c = 0;
  	for(int i = 0; i < devList.size();i++)
  	{
  		if(devList[i].kind == aswitch)
  		{
  			switch_list[c] =  wxString(devList[i].Name);
  			c++;
  		}
  	}
  	
  	//create toggle list
  	toggle_list = new wxCheckListBox(right_button_window, SWITCH_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), c, switch_list);
  	
  	//check switches that have initial state = 0
  	c = 0;
  	for(int i = 0; i < devList.size();i++)
  	{
  		if(devList[i].kind == aswitch)
  		{
  			if(devList[i].initState == 1)
  			{
  				toggle_list->Check(c,true);
  			}
  			c++;
  		}
  	}	
  	
  	//create device list for monitoring
  	c = 0;
  	for(int i= 0; i<devList.size(); i++)
  	{
  		c++;
  		if(devList[i].kind == dtype) c++;
  	}
  	
  	monitor_list = new wxString[c];
  	
  	
  	
  	for(int i = 0; i < devList.size();i++)
  	{
  		monDev m;
  		if(devList[i].kind != dtype)
  		{
  			monitor_list[i] = wxString(devList[i].Name);
  			m.devId = nmz->cvtname(devList[i].Name);
  			m.pinId = -1;
  			MList.push_back(m);
		}
		else
		{
			string s;
			string sbar;
			
			s = devList[i].Name;
			sbar = s + ".QBAR";
			s = s + ".Q";
			monitor_list[i] = wxString(s);
			monitor_list[i+1] = wxString(sbar);
			m.devId = nmz->cvtname(devList[i].Name);
			m.pinId = nmz->cvtname("Q");
			MList.push_back(m);
			
			m.pinId = nmz->cvtname("QBAR");
			MList.push_back(m);
			i++;
		}
		
  	}
  	
  	
  	monitor_list1 = new wxCheckListBox(right_button_window, MONITOR_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), devList.size(), monitor_list);
  	
  	
  	//check monitored devices
  	for(int i =0; i< devList.size();i++)
  	{
  		if(devList[i].isMonitored == true)
  		{
  			if(devList[i].kind != dtype)
  			{
  				monitor_list1 ->Check(i,true);
			}
			else
			{
				if( devList[i].bar == false)
				{
					monitor_list1 ->Check(i,true);
				}
				if(devList[i].bar == true)
				{
					monitor_list1 ->Check(i+1,true);
				}
				i++;
			}
  		}
  		else if(devList[i].kind == dtype)
  		{
  			i++;
  		}
  	}
  	
  	action_list = new wxString[1]{wxT("1: Initialised program")};
  	action_list1 = new wxListBox(dialog_window, wxID_ANY, wxDefaultPosition, wxSize(-1, 72), 1, action_list);
  	
  	/*switch_list = new wxString[6]{wxT("SW1"), wxT("SW2"), wxT("SW3"), wxT("SW4"), wxT("SW5"), wxT("SW6")};
  	toggle_list = new wxCheckListBox(right_button_window, SWITCH_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), 6, switch_list);  	
  	monitor_list = new wxString[6]{wxT("M1"), wxT("M2"), wxT("M3"), wxT("M4"), wxT("M5"), wxT("M6")};
	monitor_list1 = new wxCheckListBox(right_button_window, MONITOR_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), 6, monitor_list);
  	action_list = new wxString[1]{wxT("1: Initialised program")};
  	action_list1 = new wxListBox(dialog_window, wxID_ANY, wxDefaultPosition, wxSize(-1, 72), 1, action_list);*/
  	
	devList = prs->getDevList();
	// Set up top menu bar
	
	fileMenu->Append(MY_FILE_RUN_ID, "&Run \tCtrl-R");
	fileMenu->Append(MY_FILE_CONTINUE_ID,"&Continue\tCtrl-C");
	fileMenu->Append(MY_FILE_SAVE_ID,"&Save As...\tCtrl-S");
	fileMenu->AppendSeparator();
	fileMenu->Append(HELP_MENU_ID, "&Help\tCtrl-H");
	fileMenu->Append(wxID_ABOUT, "&About");
	fileMenu->Append(wxID_EXIT, "&Quit\tCtrl-Q");
	
	viewMenu->Append(START_MENU_ID,"Go To &Start\tShift-Ctrl-S");
	viewMenu->Append(END_MENU_ID,"Go To &End\tShift-Ctrl-E");
	viewMenu->AppendSeparator();
	viewMenu->Append(ZOOM_IN_MENU_ID,"Zoom In\tCtrl-Z");
	viewMenu->Append(ZOOM_OUT_MENU_ID,"Zoom Out\tShift-Ctrl-Z");
	viewMenu->AppendSeparator();
	viewMenu->Append(SHOW_GRID_ID, "Hide &Grid\tCtrl-G");
	
	windowMenu->Append(SHOW_SETTINGS_ID, "&Hide Settings Window");
	windowMenu->Append(SHOW_DIALOG_ID, "&Hide Dialog Window");
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(viewMenu, "&View");
	menuBar->Append(windowMenu, "&Window");
	SetMenuBar(menuBar);
	
	// Set up canvas
  
	canvas = new MyGLCanvas(left_canvas_window,this ,wxID_ANY, monitor_mod, names_mod);
	
	canvas_sizer->Add(canvas, 1, wxEXPAND);
	
	left_canvas_window->SetSizer(canvas_sizer);
	
	// Set up dialog box to output changes made 
  	
  	dialog_sizer->Add(action_list1, 0, wxEXPAND);
  	
  	dialog_window->SetBackgroundColour(*wxWHITE);
  	
  	dialog_window->SetSizer(dialog_sizer);

	// Set up controls for entering the number of cycles displayed

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, "Number of Cycles"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

	right_button_sizer->Add(spin, 0 , wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	// Set up controls for toggling switches

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, "Switch State"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

	right_button_sizer->Add(toggle_list, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	
	// Set up controls for toggling monitor points

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, "Monitor Points"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

	right_button_sizer->Add(monitor_list1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	
	// Set up controls for vertical zoom slider

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, "Vertical Zoom"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);
   
	right_button_sizer->Add(vert_zoom_value, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

	vert_zoom_slider = new wxSlider(right_button_window, VERT_SLIDER_ID, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
  
	right_button_sizer->Add(vert_zoom_slider, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);
	
	// Set up controls for horizontal zoom slider

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, "Horizontal Zoom"), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);
  
  	right_button_sizer->Add(horz_zoom_value, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

  	horz_zoom_slider = new wxSlider(right_button_window, HORZ_SLIDER_ID, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

  	right_button_sizer->Add(horz_zoom_slider, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);
  	
  	// Set up control for run button

  	right_button_sizer->Add(new wxButton(right_button_window, MY_RUN_BUTTON_ID, "Run"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  	
  	// Set up control for continue button  
  
  	right_button_sizer->Add(new wxButton(right_button_window, MY_CONTINUE_BUTTON_ID, "Continue"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);   	
  	right_button_window->SetSizer(right_button_sizer);
  	
	// Set up toolbar
	
	wxBitmap play_icon(wxT("play.png"), wxBITMAP_TYPE_PNG);
	wxBitmap continue_icon(wxT("continue.png"), wxBITMAP_TYPE_PNG);
	wxBitmap save_icon(wxT("save.png"), wxBITMAP_TYPE_PNG);
	wxBitmap start_icon(wxT("start.png"),wxBITMAP_TYPE_PNG);
	wxBitmap end_icon(wxT("end.png"),wxBITMAP_TYPE_PNG);
	wxToolBar *top_toolbar = new wxToolBar(this, TOOLBAR_ID);
	
	top_toolbar->AddTool(RUN_TOOLBAR_ID, wxT("Play"), play_icon,"Run" , wxITEM_NORMAL );
	top_toolbar->AddTool(CONTINUE_TOOLBAR_ID, wxT("Continue"), continue_icon, "Continue", wxITEM_NORMAL );
	top_toolbar->AddTool(START_TOOLBAR_ID,wxT("Go To Start"), start_icon, "Go To Start",  wxITEM_NORMAL );
	top_toolbar->AddTool(END_TOOLBAR_ID,wxT("Go To End"), end_icon, "Go To End",  wxITEM_NORMAL );
	top_toolbar->AddTool(SAVE_TOOLBAR_ID, wxT("Save Circuit"), save_icon, "Save As",  wxITEM_NORMAL);
	top_toolbar->Realize();
	
	toolbar_sizer->Add(top_toolbar, 0, wxEXPAND);	
  	
  	// Finalise application layout
  	
  	left_canvas_sizer->Add(toolbar_sizer, 0, wxALIGN_TOP|wxEXPAND);
  	
  	left_canvas_sizer->Add(left_canvas_window, 1, wxEXPAND);
  	
  	left_canvas_sizer->Add(dialog_window, 0, wxALIGN_BOTTOM|wxEXPAND|wxTOP, 5); 	
  
  	frame_sizer->Add(left_canvas_sizer, 1, wxALIGN_TOP|wxEXPAND|wxALL, 5);

  	frame_sizer->Add(right_button_window, 0, wxEXPAND|wxALIGN_TOP|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxRIGHT, 5);
  	
  	// Set the main sizer 

  	SetSizeHints(400, 400);
  	SetSizer(frame_sizer);
  
}

void MyFrame::OnExit(wxCommandEvent &event)
	// Event handler for the exit menu item
{
  	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &event)
  	// Event handler for the about menu item
{
  	//wxMessageDialog about(this, "Example wxWidgets GUI\nAndrew Gee\nJune 2014", "About Logsim", wxICON_INFORMATION | wxOK);
  	//about.ShowModal();
  	
}

// Show/Hide the guidance grid in the canvas

void MyFrame::ShowGrid(wxCommandEvent &event)
{
	canvas->ShowGrid(!show_grid);
	if(show_grid)
	{
	viewMenu->SetLabel(SHOW_GRID_ID, "Show &Grid\tCtrl-G");
	}
	else
	{
	viewMenu->SetLabel(SHOW_GRID_ID, "Hide &Grid\tCtrl-G");
	}
	show_grid = !show_grid;
}

// Show/Hide the right sizer containing the settings widgets

void MyFrame::ShowSettings(wxCommandEvent &event)
{
	right_button_window->Show(!show_settings);
	if(show_settings)
	{
	windowMenu->SetLabel(SHOW_SETTINGS_ID, "&Show Settings Window");
	}
	else
	{
	windowMenu->SetLabel(SHOW_SETTINGS_ID, "&Hide Settings Window");
	}
	frame_sizer->Layout();
	show_settings = !show_settings;
}

// Show/Hide the bottom diarunnetworklog box showing the actions carried out by the user since the program was started

void MyFrame::ShowDialog(wxCommandEvent &event)
{
	dialog_window->Show(!show_dialog);
	if(show_dialog)
	{
	windowMenu->SetLabel(SHOW_DIALOG_ID, "&Show Dialog Window");
	}
	else
	{
	windowMenu->SetLabel(SHOW_DIALOG_ID, "&Hide Dialog Window");
	}
	left_canvas_sizer->Layout();
	show_dialog = !show_dialog;
}

void MyFrame::OnSpin(wxSpinEvent &event)
  	// Event handler for the spin control
{	
  	string cycle_num_str = "Number of simulated cycles set to " + to_string(event.GetPosition());
  	print_action(cycle_num_str);
}

// Event handler for toggling switches on and off using a wxCheckListBox

void MyFrame::SwitchList(wxCommandEvent &event)
{
	bool ok;
	name devid;
	asignal sg;
	
	string switch_str;
	int switch_index = event.GetInt();
	string switch_choice = string(switch_list[switch_index].mb_str());
	
	devid = nmz->cvtname(switch_choice);
	
	if(toggle_list->IsChecked(switch_index))
	{
		sg = high;
	}
	else sg = low;
	
	
	dmz->setswitch(devid,sg, ok);
	
	if(ok)
	{
		if(toggle_list->IsChecked(switch_index))
		{
			switch_str = "Switch " + switch_choice + " toggled on";
		}
		else
		{
			switch_str = "Switch " + switch_choice + " toggled off";
		}
	}
	else
	{
		if(toggle_list->IsChecked(switch_index))
		{
			switch_str = "Could not toggle switch " + switch_choice + " on";
		}
		else
		{
			switch_str = "Could not toggle switch " + switch_choice + " off";
		}
	}
	print_action(switch_str);
} 

// Event handler for setting monitor points using a wxCheckListBox

void MyFrame::MonitorList(wxCommandEvent &event)
{
	string monitor_str;
	int monitor_index = event.GetInt();
	string monitor_choice = string(monitor_list[monitor_index].mb_str());
	
	bool ok;
	
	if(monitor_list1->IsChecked(monitor_index))
	{
		//device = nmz->cvtname(monitor_str);
		//deviceOut = -1; //Again, not very obvious...

		//nmz -> printEntries();  
		mmz->makemonitor(MList[monitor_index].devId,MList[monitor_index].pinId,ok);
		if(ok)
		{
			monitor_str = "Added monitor to " + monitor_choice;
		}
		else
		{
			monitor_str = "Could not add monitor to " + monitor_choice;
		}
	}
	else
	{
		mmz->remmonitor(MList[monitor_index].devId,MList[monitor_index].pinId,ok);
		if(ok)
		{
			monitor_str = "Removed monitor from " + monitor_choice;
		}
		else
		{
			monitor_str = "Could not remove monitor from " + monitor_choice;
		}
	}
	
	print_action(monitor_str);
	canvas->montr();
}

// Functions to zoom vertically and horizontally on the canvas controlled by two separate sliders

void MyFrame::OnVertZoomRelease(wxCommandEvent &event)
{
  	int vert_sliderpos = vert_zoom_slider->GetValue(); 

  	float vert_zoom_fl;
  	int vert_zoom_int;

  	if(vert_sliderpos <= 50) 
  	{
    	vert_zoom_fl = (1.5*vert_sliderpos)+25.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	if(vert_sliderpos > 50) 
  	{
    	vert_zoom_fl = 6.0*((1.0*vert_sliderpos)-50.0)+100.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	string vert_zoom_str = "x" + to_string(vert_zoom_int) + "%";
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  	canvas->ZoomVert(vert_zoom_int);
}

void MyFrame::OnHorzZoomRelease(wxCommandEvent &event)
{
  	int horz_sliderpos = horz_zoom_slider->GetValue(); 

  	float horz_zoom_fl;
  	int horz_zoom_int;

  	if(horz_sliderpos <= 50) 
  	{
    	horz_zoom_fl = (1.5*horz_sliderpos)+25.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	if(horz_sliderpos > 50) 
  	{
    	horz_zoom_fl = 6.0*((1.0*horz_sliderpos)-50.0)+100.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	string horz_zoom_str = "x" + to_string(horz_zoom_int) + "%";
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);

  	horz_zoom_value->SetLabel(horz_zoom_wxstr);
  
  	canvas->ZoomHor(horz_zoom_int);
}


void MyFrame::OnRunButton(wxCommandEvent &event)
  	// Event handler for the push button
{
  	int n, ncycles;
	run_flag = true;
  	cyclescompleted = 0;
  	dmz->initdevices ();
  	mmz->resetmonitor ();
  	runnetwork(spin->GetValue());
  	
  	string run_str = "Simulation started for " + to_string(spin->GetValue()) + " cycles";
  	print_action(run_str);
  	
  	canvas->run(spin->GetValue());
}

void MyFrame::OnContinueButton(wxCommandEvent &event)
{
	if(run_flag == true)
	{
		runnetwork(spin->GetValue());
		string continue_str = "Simulation continued for an additional " + to_string(spin->GetValue()) + " cycles";
		print_action(continue_str);
		
		canvas->cont(spin->GetValue());
	}
	else
	{
		wxMessageBox( wxT("You think you could continue before running?"), wxT("Warning"), wxICON_EXCLAMATION);
	}	
}

// Event handler for saving the canvas contents which calls a function within the GLCanvas. The image is saved as a PNG

void MyFrame::SaveCanvas(wxCommandEvent &event)
{
	canvas->save_canvas();
}

void MyFrame::OnStart(wxCommandEvent &event)
{
	canvas->goToStart();
}


void MyFrame::OnEnd(wxCommandEvent &event)
{
	canvas->goToEnd();
}

void MyFrame::OnZoomIn(wxCommandEvent &event)
{
	int vert_sliderpos = vert_zoom_slider->GetValue();
	int horz_sliderpos = horz_zoom_slider->GetValue();
	vert_sliderpos += 5;
	horz_sliderpos += 5;
	if(vert_sliderpos > 100) vert_sliderpos = 100;
	if(horz_sliderpos > 100) horz_sliderpos=100;
	
	vert_zoom_slider->SetValue(vert_sliderpos);
	horz_zoom_slider->SetValue(horz_sliderpos);
		
	float horz_zoom_fl;
  	int horz_zoom_int;

  	if(horz_sliderpos <= 50) 
  	{
    	horz_zoom_fl = (1.5*horz_sliderpos)+25.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	if(horz_sliderpos > 50) 
  	{
    	horz_zoom_fl = 6.0*((1.0*horz_sliderpos)-50.0)+100.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	string horz_zoom_str = "x" + to_string(horz_zoom_int) + "%";
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);

  	horz_zoom_value->SetLabel(horz_zoom_wxstr);
  	
  	
  	float vert_zoom_fl;
  	int vert_zoom_int;

  	if(vert_sliderpos <= 50) 
  	{
    	vert_zoom_fl = (1.5*vert_sliderpos)+25.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	if(vert_sliderpos > 50) 
  	{
    	vert_zoom_fl = 6.0*((1.0*vert_sliderpos)-50.0)+100.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	string vert_zoom_str = "x" + to_string(vert_zoom_int) + "%";
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
}

void MyFrame::OnZoomOut(wxCommandEvent &event)
{
	int vert_sliderpos = vert_zoom_slider->GetValue();
	int horz_sliderpos = horz_zoom_slider->GetValue();
	vert_sliderpos -= 5;
	horz_sliderpos -= 5;
	if(vert_sliderpos < 0) vert_sliderpos = 0;
	if(horz_sliderpos <0) horz_sliderpos=0;
	
	vert_zoom_slider->SetValue(vert_sliderpos);
	horz_zoom_slider->SetValue(horz_sliderpos);
		
	float horz_zoom_fl;
  	int horz_zoom_int;

  	if(horz_sliderpos <= 50) 
  	{
    	horz_zoom_fl = (1.5*horz_sliderpos)+25.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	if(horz_sliderpos > 50) 
  	{
    	horz_zoom_fl = 6.0*((1.0*horz_sliderpos)-50.0)+100.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	string horz_zoom_str = "x" + to_string(horz_zoom_int) + "%";
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);

  	horz_zoom_value->SetLabel(horz_zoom_wxstr);
  	
  	
  	float vert_zoom_fl;
  	int vert_zoom_int;

  	if(vert_sliderpos <= 50) 
  	{
    	vert_zoom_fl = (1.5*vert_sliderpos)+25.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	if(vert_sliderpos > 50) 
  	{
    	vert_zoom_fl = 6.0*((1.0*vert_sliderpos)-50.0)+100.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	string vert_zoom_str = "x" + to_string(vert_zoom_int) + "%";
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
  	
}

void MyFrame::OnMouse(wxMouseEvent &event)
{
	int num = 0;
	
	int r = event.GetWheelRotation();

	if(r>0 &&  event.ControlDown() == false) 
	{
		num = 2;
	}
	else if(r < 0  &&  event.ControlDown() == false)
	{
		 num = -2;
	}
	else
	{
		return;
	}
	
	int vert_sliderpos = vert_zoom_slider->GetValue();
	int horz_sliderpos = horz_zoom_slider->GetValue();
	vert_sliderpos += num;
	horz_sliderpos += num;
	if(vert_sliderpos < 0) vert_sliderpos = 0;
	if(horz_sliderpos <0) horz_sliderpos=0;
	if(vert_sliderpos > 100) vert_sliderpos = 100;
	if(horz_sliderpos > 100) horz_sliderpos= 100;
	
	
	vert_zoom_slider->SetValue(vert_sliderpos);
	horz_zoom_slider->SetValue(horz_sliderpos);
		
	float horz_zoom_fl;
  	int horz_zoom_int;

  	if(horz_sliderpos <= 50) 
  	{
    	horz_zoom_fl = (1.5*horz_sliderpos)+25.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	if(horz_sliderpos > 50) 
  	{
    	horz_zoom_fl = 6.0*((1.0*horz_sliderpos)-50.0)+100.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	string horz_zoom_str = "x" + to_string(horz_zoom_int) + "%";
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);

  	horz_zoom_value->SetLabel(horz_zoom_wxstr);
  	
  	
  	float vert_zoom_fl;
  	int vert_zoom_int;

  	if(vert_sliderpos <= 50) 
  	{
    	vert_zoom_fl = (1.5*vert_sliderpos)+25.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	if(vert_sliderpos > 50) 
  	{
    	vert_zoom_fl = 6.0*((1.0*vert_sliderpos)-50.0)+100.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	string vert_zoom_str = "x" + to_string(vert_zoom_int) + "%";
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
	
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

// Function to print out the actions carried out by the user to the dialog box at the bottom of the application

void MyFrame::print_action(string message)
{
  	string action_message = to_string(action_num) + ": " + message; 
  	
  	
  	wxString message_wxstr(action_message.c_str(), wxConvUTF8);
	
	wxString *addtolist = new wxString[1]{message_wxstr};
	action_list1->InsertItems(1, addtolist, 0);
	action_list1->EnsureVisible(0);
	
	action_num = action_num+1;	
}

void MyFrame::zoom(int n)
{
	int num = n;
	int vert_sliderpos = vert_zoom_slider->GetValue();
	int horz_sliderpos = horz_zoom_slider->GetValue();
	vert_sliderpos += num;
	horz_sliderpos += num;
	if(vert_sliderpos < 0) vert_sliderpos = 0;
	if(horz_sliderpos <0) horz_sliderpos=0;
	if(vert_sliderpos > 100) vert_sliderpos = 100;
	if(horz_sliderpos > 100) horz_sliderpos= 100;
	
	
	vert_zoom_slider->SetValue(vert_sliderpos);
	horz_zoom_slider->SetValue(horz_sliderpos);
		
	float horz_zoom_fl;
  	int horz_zoom_int;

  	if(horz_sliderpos <= 50) 
  	{
    	horz_zoom_fl = (1.5*horz_sliderpos)+25.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	if(horz_sliderpos > 50) 
  	{
    	horz_zoom_fl = 6.0*((1.0*horz_sliderpos)-50.0)+100.0;
    	horz_zoom_int = floor(horz_zoom_fl);
  	}

  	string horz_zoom_str = "x" + to_string(horz_zoom_int) + "%";
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);

  	horz_zoom_value->SetLabel(horz_zoom_wxstr);
  	
  	
  	float vert_zoom_fl;
  	int vert_zoom_int;

  	if(vert_sliderpos <= 50) 
  	{
    	vert_zoom_fl = (1.5*vert_sliderpos)+25.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	if(vert_sliderpos > 50) 
  	{
    	vert_zoom_fl = 6.0*((1.0*vert_sliderpos)-50.0)+100.0;
    	vert_zoom_int = floor(vert_zoom_fl);
  	}
  	
  	string vert_zoom_str = "x" + to_string(vert_zoom_int) + "%";
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
}

void MyFrame::ShowHelp(wxCommandEvent &event)
{
/*
		helpD = new wxSimpleHelpProvider();
		helpD-> AddHelp(this, "Set the number of clock cycles to run, the default is set to 10. Pressing continue will run the simulation for an additional number of cycles from where the initial run stopped. Pressing the run button again will start from time 0.\n\n The switch state is on when the switch is ticked and off otherwise. Monitoring points can be changed in a similar fashion.\n\n Vertical and horizontal zoom can be set independently from the sliders. Zooming can also be achieved using the mouse wheel.  By pressing control and using the mouse wheel the user can scroll through all the monitored devices. Zooming in while keeping the aspect ratio constant can also be achieved using Ctrl-Z while zooming out is possible using Ctrl-Shift-Z.\n\n The user can go to time zero or to the current time by using the toolbar buttons. \n\n The displayed signals can be saved to a PNG image using the save icon on the toolbar or using the shortcut.\n\n the grid can be toggled on or off. The settings and dialog window can also be toggled on or off to increase the signal viewing area");
		helpD->ShowHelp(this);
		*/
		ifstream hfile;
		char ch;
		string hstring, tmp;
		
		helpD = new wxDialog(this, -1, "Help", wxDefaultPosition, wxSize(350,500), wxDEFAULT_DIALOG_STYLE);
		wxTextCtrl textarea(helpD, -1,"", wxDefaultPosition, wxSize(300,400),
      wxTE_MULTILINE | wxTE_RICH | wxTE_READONLY, wxDefaultValidator, wxTextCtrlNameStr);
		
		hfile.open("help.txt");
		while(!hfile.eof())
		{
			getline(hfile,tmp);
			tmp += "\n";
			hstring += tmp;
		}
		
		textarea.SetValue(hstring);		
		//textarea.SetEditable(false);
		
		
		textarea.SetFocus();

		
		helpD->ShowModal();
		
}













