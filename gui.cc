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
//events commented out were used for debugging and some functionality
//the mouse events were merged int a single mouse event
BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
	EVT_SIZE(MyGLCanvas::OnSize)
	EVT_PAINT(MyGLCanvas::OnPaint)
	EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
	//EVT_SIZE(MyGLCanvas::resized)
	EVT_KEY_DOWN(MyGLCanvas::keyPressed)
	EVT_KEY_UP(MyGLCanvas::keyReleased)
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
  frame = f;		//set parent frame
  init = false;
  pan_x = 0;
  pan_y = 0;
  zoom = 1.0; //set default zoom
  cyclesdisplayed = -1;
  
  kState = new wxKeyboardState(true,true,true,true); //monitor certain key states(shift,alt,ctrl)
  
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
	
	//set show grid to true by default and current size to not small
	showGrid = true;
	isSmall = false;
	
	//set to false to hide the grey box initially
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
  GetClientSize(&width, &height); //get width and height
  
	
  
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
		
		int max_len = 8;
		if(nam.Len() < 8) max_len = nam.Len();
		if(signal_height/SIGNAL_HEIGHT < 0.65)
		{
    		for (int k = 0; k < max_len; k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, nam[k]);
    	}
    	else
    	{
    		for (int k = 0; k < max_len; k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, nam[k]);
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
	
	//Used for debugging and testing
	//draw the artificial signals that can he created by setting the parameters i the header file
	
	
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



  // We've been drawing to the back buffer, flush the graphics pipeline and swap the back buffer to the front  
  glFlush();
  SwapBuffers();
}

//run the simulation for n cycles
void MyGLCanvas::run(int cycles)
{
	
	mons currentMonitor; //create a current monitor point
	asignal s;			//create a variable for the current state
	currentTime = 0;	//set the current time to zero
	
	monitoring.clear();//clear the previous monitoring points
	sigs.clear();		//clear the previous singlas
	montr();			//get new monitors
	vector<asignal> sv;
	
	//run through all the monitor points
	for(int i=0; i<nmonitor; i++)
	{
		currentMonitor = monitoring[i];
		sv.clear(); //clear the previous signal
		for(int j = 0; j < cycles; j++)
		{
			if(mmz->getsignaltrace(i,j,s))
			{
				sv.push_back(s); //push back signal level
			}
			else
			{
				sv.push_back(invalid_signal); //if some error occured
			}
			//cout << "j = " << j << " , s = " << s << endl;
		}
		sigs.push_back(sv);//add the signal trace o the monitored signal to the signals vector
	}
	
	currentTime = cycles; //set the current time to cycles
	
	endAt = currentTime;	//set the end at the last simulated cycle
	
	startAt = endAt - floor((width-NAME_SPACE)/signal_width);	//set start at the smallest number that would
																//fit in the screen
	if(startAt < 0) startAt = 0;
	
	Render();
}

//prints the stored signals on the command line, used for debugging
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

//function called on continue
void MyGLCanvas::cont(int cycles)
{
	//same as in run
	mons currentMonitor;
	asignal s;
	vector<asignal> sv;
	sigs.clear();
	
	
	//bool monitored_before[old_m.size()];
	
	//get new signals same way as in run
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
	
	
	currentTime += cycles;		//update start at
	
	GetClientSize(&width, &height);

	endAt += cycles;	//update end at
	
	if(endAt - floor((width-NAME_SPACE)/signal_width) > startAt) startAt = endAt -floor((width-NAME_SPACE)/signal_width);
	
	
	Render();

}

//get monitros
void MyGLCanvas::montr()
{
	nmonitor = mmz->moncount();
	mons currentMonitor;
	
	//vector<mons> old_m = monitoring;
	
	vector<mons> newMons; //vector of new monitoring points
	
	//get new monitoring points and set the start time of all of them at the current time
	for(int i = 0; i < nmonitor; i++)
	{
		currentMonitor.number = i;
		mmz->getmonname(currentMonitor.number, currentMonitor.devId, currentMonitor.pinId);
		
		currentMonitor.nme = nmz->getName(currentMonitor.devId);
		
		currentMonitor.startTime = currentTime;
		
		
		newMons.push_back(currentMonitor);
	}
	
	//check if the device was monitored on the previous run/continue
	//and if it was monitored set the monitor start time as that of the previous state
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
	//clear the previous monitor list and set it to the new list
	monitoring.clear();
	monitoring = newMons;
	
}

//prints gray rectangle with small orange arrows on left and right for
//moving left and right in time
void MyGLCanvas::printRectangle()
{
	GetClientSize(&width, &height);

	float x1,y1,x2,y2;
	//if mouse is left print the shape to the left of the canvas, same for right
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
	glRectf(x1,y1,x2,y2); //draw rectangle
	
	//set color for the arrows
	glColor4f(0.945,0.561,0.408,0.25);
	
	//draw the arrows
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

//generate 100 fake numbers
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

	//update end time
	endAt = currentTime;

	startAt = endAt - floor((width-NAME_SPACE)/signal_width);
	if(startAt < 0) startAt = 0;

}


//called to change the state for showing the grid or not
void MyGLCanvas::ShowGrid(bool show)
{
	showGrid = show;
	Render();
}

//displays time at the top of the canvas
void MyGLCanvas::printTime()
{
	glColor3f(240.0/255.0, 12.0/255.0, 39.0/255.0); //bright red color
	for(int i = startAt; i < floor(((float)width-NAME_SPACE)/signal_width)+1+startAt; i ++)
	{
		if(isSmall == true && (i%5) != 0) continue; //continue to next cycle if the size is small and i is not a multiple of 5
		if(i > 100  && (i%5) != 0 && signal_width/SIGNAL_WIDTH < 1.5) continue; //if zoom is small display every fifth number to avoid one going over the next
		if(i>1000  && (i%5) != 0 ) continue;	//if numbers are large print every fifth to avoid the previous problem
		if(isSmall) glRasterPos2f(NAME_SPACE + (i-startAt)*signal_width - 7.5, height - 10); //set print position
		else glRasterPos2f(NAME_SPACE + (i-startAt)*signal_width - 7.5, height - 10);
		
		wxString name;
		name.Printf( "%d", i);
		for (int k = 0; k < name.Len(); k++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, name[k]);//print number
	}
}

//print grid
void MyGLCanvas::printGrid()
{
	  
	GetClientSize(&width, &height);
	for(int i = 0; i < floor(((float)width-NAME_SPACE)/signal_width)+1; i ++)
	{
		
    	glColor3f(0.90, 0.90, 0.90); //light gray color
    	if( (i + startAt)%5 == 0 && isSmall == true) glColor3f(0.5, 0.5, 0.5); //make every fifth line darker when zoomed out
		//draw time ticks
		glBegin(GL_LINE_STRIP);//draw line segments
		glVertex2f(NAME_SPACE + i*signal_width,0);
		glVertex2f(NAME_SPACE + i*signal_width,height);
		glEnd();
	}
	glColor3f(0.90, 0.90, 0.90); //light gray color
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
  	
  	pixels = (unsigned char *) malloc(3 * width * height); // RGB uses three characters per pixel
  	glPixelStorei(GL_PACK_ALIGNMENT, 1);
  	glReadBuffer(GL_BACK_RIGHT);
  	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// This mirrors the unsigned char pixels to ensure that the resulting image is in the right orientation after the subsequent mirroring, which seems to be needed to avoid a segmentation fault

	mirror_char(pixels, width, height);
	
  	save_im = new wxImage(width, height);
  	save_im->SetData(pixels, width, height, false);		// Sets the wxImage using the pixel data
  	
  	save_im->Mirror(false);		// Removing this mirror appears to cause a segmentation fault
  	
  	// Bring up the file dialog to choose the path where the image is saved
  	
  	wxFileDialog *save_path_dialog = new wxFileDialog(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE);
  	save_path_dialog->ShowModal();
  	
  	wxString save_path;
  	save_path.Printf("");
  	//wxString *save_path = new wxString(wxT(""));
  	save_path = save_path_dialog->GetPath();
  	string path = string(save_path.mb_str());
  	size_t pos;
  	
  	// Only attempt to save the image if a filename is specified by the user
  	
  	if(!(save_path.IsEmpty()))
  	{
  		pos = path.find(".png",0);
  		
  		// Always save the image with the extension .png even if the user specifies a different format (i.e. .jpg) to avoid errors. This function could be expanded to save the image as a range of different file formats
  		
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
    int row_stride = width * 3; // RGB uses three characters per pixel
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

//zooms verticallly
void MyGLCanvas::ZoomVert(int zoom)
{
	float scale;
	scale = (float)zoom/100.0;
	signal_height = SIGNAL_HEIGHT*scale; //uptade the signal height and space between signals
	space_between_signals = SIGNAL_SPACE*scale;
	wxString str;
	str.Printf("");
	
	GetClientSize(&width, &height);	//get new dimensions
 	 max_number_to_print = floor((float)height/(signal_height + space_between_signals));
  	//cout << "max to print: " << max_number_to_print << endl;
  	end_signal = start_signal + max_number_to_print;	//calculate the new end point
  	
  	//check if start signal and end signal are valid 
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

//set horizontal zoom
void MyGLCanvas::ZoomHor(int zoom)
{
	//change the width of each time period using the new zoom	
	float scale;
	scale = (float)zoom/100.0;
	signal_width = SIGNAL_WIDTH*scale;
	
	if(scale <= 0.6)
	{
		isSmall = true;	//if 
	}
	else
	{
		isSmall = false;
	}
	
	wxString str;
	str.Printf("");
	
	
	Render();
}

//set the starting position of the signal at zero
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

//sets the end position at current time
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
	
	//if there is a double click on the left or right portion of the screen, pan 10 to the left or right
	// 9 is used since the first click is identified as a single click
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
	//if single click increment by one
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
	
	
	/*if there is a wheel rotation:
	
		* cltr and alt not pressed -> zoom in/out
		* ctrl pressed, alt not -> move up and down
		* alt pressed, control not -> pan left and right
	
	*/
	int r = event.GetWheelRotation();
	if(r > 0 && event.AltDown() == true  && event.ControlDown() == false)
	{
		startAt++;
	}
	else if(r < 0 && event.AltDown() == true  && event.ControlDown() == false)
	{
		startAt--;
	}
	else if(r>0 &&  event.ControlDown() == true  && event.AltDown() == false) 
	{
		end_signal += 1;
		start_signal += 1;
	}
	else if(r < 0  &&  event.ControlDown() == true  && event.AltDown() == false)
	{
		 end_signal -=1;
		 start_signal -=1;
	}
	else if(r > 0 &&  event.ControlDown() == false  && event.AltDown() == false)
	{
		frame->zoom(2);
		
		endAt = currentTime;
		startAt = endAt - floor((width-NAME_SPACE)/signal_width);
		if(startAt < 0) startAt = 0;
	}
	else if(r < 0 &&  event.ControlDown() == false  && event.AltDown() == false)
	{
		frame->zoom(-2);
		
		endAt = currentTime;
		startAt = endAt - floor((width-NAME_SPACE)/signal_width);
		if(startAt < 0) startAt = 0;
	}
	
	//check if start and end of signal are valid positions, if not fix them
	
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
	
	// if mouse is moved to the extreme left or right display the rectangle to move
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
	// if the mouse left the GL canvas hide the rectangle for panning left and right
	 if(event.Leaving())
	 {
	 	mouse_left = false;
		mouse_right = false;
	 }
	 
	 
	 if(startAt>=currentTime) 
	 {
	 	startAt = currentTime - 1;
	 	endAt =currentTime;
 	}
	 if(endAt > currentTime)
	 {	
	 	 endAt = currentTime;
	 	 startAt =  currentTime - floor((width-NAME_SPACE)/signal_width);
	 }
	 if(startAt < 0) startAt = 0;
	 Render();
}

void MyGLCanvas::keyPressed(wxKeyEvent& event) {}
void MyGLCanvas::keyReleased(wxKeyEvent& event) {}

// MyFrame ///////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MyFrame::OnExit)
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
		cout << _("Cannot operate GUI without names, devices, parser and monitor classes") << endl;
		exit(1);
  	}
  	
  	devList = prs->getDevList(); // Gets the list of devices from the parser
  	
  	sort(devList.begin(), devList.end(), [ ]( const dev& lhs, const dev& rhs )
	{
	   return lhs.Name < rhs.Name;
	}
	);  
  	
  	// Creates a list of switches
  	
  	int c = 0;
  	
  	// Gets number of switches so that the array of wxStrings can be initialised
  	
  	for(int i = 0; i < devList.size(); i++)
  	{
  		if(devList[i].kind == aswitch) c++;
  		if(devList[i].kind == anrc) rcList.push_back(devList[i]);
  	}
  	switch_list = new wxString[c];
  	c = 0;
  	
  	// Adds each switch to the array of wxStrings
  	
  	for(int i = 0; i < devList.size(); i++)
  	{
  		if(devList[i].kind == aswitch)
  		{
  			switch_list[c] =  wxString(devList[i].Name);
  			c++;
  		}
  	}
  	
  	// Creates toggle list used to display and toggle switches
  	
  	toggle_list = new wxCheckListBox(right_button_window, SWITCH_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), c, switch_list);
  	
  	// Checks (on the displayed wxCheckListBox) the array items for which the corresponding switches start off as being switched on, as defined in the circuit file
  	
  	c = 0;
  	for(int i = 0; i < devList.size(); i++)
  	{
  		if(devList[i].kind == aswitch)
  		{
  			if(devList[i].initState == 1)
  			{
  				toggle_list->Check(c, true);
  			}
  			c++;
  		}
  	}	
  	
  	// Creates device list for monitoring
  	
  	c = 0;
  	for(int i= 0; i < devList.size(); i++)
  	{
  		c++;
  		if(devList[i].kind == dtype) c++; // Increment count twice if the device is a DType as it will have two outputs
  	}
  	
  	monitor_list = new wxString[c];
  	
  	// DTypes contain two outputs, so two separate monitors are needed
  	
  	int dtypes_added = 0;
  	for(int i = 0; i < devList.size();i++)
  	{
  		monDev m;

  		if(devList[i].kind != dtype)
  		{	
  			monitor_list[i + dtypes_added] = wxString(devList[i].Name);
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
			monitor_list[i+ dtypes_added] = wxString(s);
			monitor_list[i+1+ dtypes_added] = wxString(sbar);
			m.devId = nmz->cvtname(devList[i].Name);
			m.pinId = nmz->cvtname("Q");
			MList.push_back(m);
			
			m.pinId = nmz->cvtname("QBAR");
			MList.push_back(m);
			dtypes_added ++;
		}
  	}
  	

  	monitor_list1 = new wxCheckListBox(right_button_window, MONITOR_LISTBOX_ID, wxDefaultPosition, wxSize(125, 95), c, monitor_list);
  	
  	dtypes_added = 0;
  	//check monitored devices
  	for(int i =0; i< devList.size();i++)
  	{
  		if(devList[i].isMonitored == true)
  		{
  			if(devList[i].kind != dtype)
  			{
  				monitor_list1 ->Check(i+dtypes_added,true);
			}
			else
			{
				if( devList[i].bar == false)
				{
					monitor_list1 ->Check(i+dtypes_added,true);
				}
				if(devList[i].bar == true)
				{
					monitor_list1 ->Check(i+1+dtypes_added,true);
				}
				dtypes_added++;
			}
  		}
  		else if(devList[i].kind == dtype)
  		{
  			dtypes_added++;
  		}
  	}
  	
  	// Creates a wxListBox, which is placed at the bottom of the frame, to output a history of actions taken by the user. A wxListBox was chosen since it is easy to append to
  	
  	action_list = new wxString[1]{_("1: Initialised program")};
  	action_list1 = new wxListBox(dialog_window, wxID_ANY, wxDefaultPosition, wxSize(-1, 72), 1, action_list);
  	
	devList = prs->getDevList();
	
	// Set up top menu bar
	
	fileMenu->Append(MY_FILE_RUN_ID, _("&Run \tCtrl-R"));
	fileMenu->Append(MY_FILE_CONTINUE_ID,_("&Continue\tCtrl-C"));
	fileMenu->Append(MY_FILE_SAVE_ID,_("&Save As...\tCtrl-S"));
	fileMenu->AppendSeparator();
	fileMenu->Append(HELP_MENU_ID, _("&Help\tCtrl-H"));
	fileMenu->Append(wxID_EXIT, _("&Quit\tCtrl-Q"));
	
	viewMenu->Append(START_MENU_ID,_("Go To &Start\tShift-Ctrl-S"));
	viewMenu->Append(END_MENU_ID,_("Go To &End\tShift-Ctrl-E"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ZOOM_IN_MENU_ID,_("Zoom In\tCtrl-Z"));
	viewMenu->Append(ZOOM_OUT_MENU_ID,_("Zoom Out\tShift-Ctrl-Z"));
	viewMenu->AppendSeparator();
	viewMenu->Append(SHOW_GRID_ID, _("Hide &Grid\tCtrl-G"));
	
	windowMenu->Append(SHOW_SETTINGS_ID, _("&Hide Settings Window"));
	windowMenu->Append(SHOW_DIALOG_ID, _("&Hide Dialog Window"));
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _("&File"));
	menuBar->Append(viewMenu, _("&View"));
	menuBar->Append(windowMenu, _("&Window"));
	SetMenuBar(menuBar);
	
	// Set up canvas using GLCanvas. This is where the signals are drawn
  
	canvas = new MyGLCanvas(left_canvas_window,this ,wxID_ANY, monitor_mod, names_mod);
	
	canvas_sizer->Add(canvas, 1, wxEXPAND);
	
	left_canvas_window->SetSizer(canvas_sizer);
	
	// Set up dialog box to output changes made using a wxListBox
  	
  	dialog_sizer->Add(action_list1, 0, wxEXPAND);
  	
  	dialog_window->SetBackgroundColour(*wxWHITE);
  	
  	dialog_window->SetSizer(dialog_sizer);

	// Set up controls for entering the number of cycles displayed

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, _("Number of Cycles")), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 10);

	right_button_sizer->Add(spin, 0 , wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	// Set up controls for toggling switches

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, _("Switch State")), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

	right_button_sizer->Add(toggle_list, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	
	// Set up controls for toggling monitor points

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, _("Monitor Points")), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

	right_button_sizer->Add(monitor_list1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	
	// Set up controls for vertical zoom slider

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, _("Vertical Zoom")), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);
   
	right_button_sizer->Add(vert_zoom_value, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

	vert_zoom_slider = new wxSlider(right_button_window, VERT_SLIDER_ID, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
  
	right_button_sizer->Add(vert_zoom_slider, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);
	
	// Set up controls for horizontal zoom slider

	right_button_sizer->Add(new wxStaticText(right_button_window, wxID_ANY, _("Horizontal Zoom")), 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);
  
  	right_button_sizer->Add(horz_zoom_value, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxLEFT|wxRIGHT, 5);

  	horz_zoom_slider = new wxSlider(right_button_window, HORZ_SLIDER_ID, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

  	right_button_sizer->Add(horz_zoom_slider, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);
  	
  	// Set up control for run button

  	right_button_sizer->Add(new wxButton(right_button_window, MY_RUN_BUTTON_ID, _("Run")), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  	
  	// Set up control for continue button  
  
  	right_button_sizer->Add(new wxButton(right_button_window, MY_CONTINUE_BUTTON_ID, _("Continue")), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);   	
  	right_button_window->SetSizer(right_button_sizer);
  	
	// Set up toolbar icons
	
	//wxBitmap play_icon(wxT("play.png"), wxBITMAP_TYPE_PNG);
	//wxBitmap continue_icon(wxT("continue.png"), wxBITMAP_TYPE_PNG);
	wxBitmap save_icon(wxT("save.png"), wxBITMAP_TYPE_PNG);
	wxBitmap start_icon(wxT("start.png"),wxBITMAP_TYPE_PNG);
	wxBitmap end_icon(wxT("end.png"),wxBITMAP_TYPE_PNG);
	wxToolBar *top_toolbar = new wxToolBar(this, TOOLBAR_ID);
	
	// Add tools to the toolbar
	
	//top_toolbar->AddTool(RUN_TOOLBAR_ID, wxT("Play"), play_icon,"Run" , wxITEM_NORMAL );
	//top_toolbar->AddTool(CONTINUE_TOOLBAR_ID, wxT("Continue"), continue_icon, "Continue", wxITEM_NORMAL );
	top_toolbar->AddTool(START_TOOLBAR_ID,_("Go To Start"), start_icon, "Go To Start",  wxITEM_NORMAL );
	top_toolbar->AddTool(END_TOOLBAR_ID,_("Go To End"), end_icon, "Go To End",  wxITEM_NORMAL );
	top_toolbar->AddTool(SAVE_TOOLBAR_ID, _("Save Circuit"), save_icon, _("Save As"),  wxITEM_NORMAL);
	top_toolbar->Realize(); // Create the toolbar
	
	toolbar_sizer->Add(top_toolbar, 0, wxEXPAND);	
  	
  	// Finalise application layout
  	
  	left_canvas_sizer->Add(toolbar_sizer, 0, wxALIGN_TOP|wxEXPAND);
  	
  	left_canvas_sizer->Add(left_canvas_window, 1, wxEXPAND);
  	
  	left_canvas_sizer->Add(dialog_window, 0, wxALIGN_BOTTOM|wxEXPAND|wxTOP, 5); 	
  
  	frame_sizer->Add(left_canvas_sizer, 1, wxALIGN_TOP|wxEXPAND|wxALL, 5);

  	frame_sizer->Add(right_button_window, 0, wxEXPAND|wxALIGN_TOP|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxRIGHT, 5);
  	
  	// Allow the settings windowed to be scrolled
  	
  	right_button_window->FitInside();
  	right_button_window->SetScrollbars(20, 20, 50, 50);  	
  	
  	// Set the main sizer for the application window

  	SetSizeHints(400, 400);
  	SetSizer(frame_sizer);
  
}

// Event handler for the exit menu item

void MyFrame::OnExit(wxCommandEvent &event)
{
  	Close(true);
}

// Show/Hide the grid in the canvas

void MyFrame::ShowGrid(wxCommandEvent &event)
{
	canvas->ShowGrid(!show_grid);
	if(show_grid)
	{
		viewMenu->SetLabel(SHOW_GRID_ID, _("Show &Grid\tCtrl-G"));
	}
	else
	{
		viewMenu->SetLabel(SHOW_GRID_ID, _("Hide &Grid\tCtrl-G"));
	}
	show_grid = !show_grid;
}

// Show/Hide the right sizer containing the settings widgets

void MyFrame::ShowSettings(wxCommandEvent &event)
{
	right_button_window->Show(!show_settings);
	if(show_settings)
	{
		windowMenu->SetLabel(SHOW_SETTINGS_ID, _("&Show Settings Window"));
	}
	else
	{
		windowMenu->SetLabel(SHOW_SETTINGS_ID, _("&Hide Settings Window"));
	}
	frame_sizer->Layout();	// Causes the layout to update and actually show/hide the sizer 
	show_settings = !show_settings;
}

// Show/Hide the bottom dialog box showing the actions carried out by the user since the program was started

void MyFrame::ShowDialog(wxCommandEvent &event)
{
	dialog_window->Show(!show_dialog);
	if(show_dialog)
	{
		windowMenu->SetLabel(SHOW_DIALOG_ID, _("&Show Dialog Window"));
	}
	else
	{
		windowMenu->SetLabel(SHOW_DIALOG_ID, _("&Hide Dialog Window"));
	}
	left_canvas_sizer->Layout();	// Causes the layout to update and actually show/hide the sizer
	show_dialog = !show_dialog;
}

// Event handler for the spin control

void MyFrame::OnSpin(wxSpinEvent &event)
{	
  	string cycle_num_str = _("Number of simulated cycles set to ") + to_string(event.GetPosition());
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
	string switch_choice = string(switch_list[switch_index].mb_str()); 	// Convert wxString to a standard string
	
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
			switch_str = _("Switch ") + switch_choice + _(" toggled on");
		}
		else
		{
			switch_str = _("Switch ") + switch_choice + _(" toggled off");
		}
	}
	else
	{
		if(toggle_list->IsChecked(switch_index))
		{
			switch_str = _("Could not toggle switch ") + switch_choice + _(" on");
		}
		else
		{
			switch_str = _("Could not toggle switch ") + switch_choice + _(" off");
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
	
		// Creates a new monitor when an item is checked
	
		mmz->makemonitor(MList[monitor_index].devId,MList[monitor_index].pinId,ok);
		if(ok)
		{
			monitor_str = _("Added monitor to ") + monitor_choice;
		}
		else
		{
			monitor_str = _("Could not add monitor to ") + monitor_choice;
		}
	}
	else
	{
	
		// Removes a monitor when an item is unchecked
	
		mmz->remmonitor(MList[monitor_index].devId,MList[monitor_index].pinId,ok);
		if(ok)
		{
			monitor_str = _("Removed monitor from ") + monitor_choice;
		}
		else
		{
			monitor_str = _("Could not remove monitor from ") + monitor_choice;
		}
	}
	
	print_action(monitor_str);
	canvas->montr();
}

// Functions to zoom vertically and horizontally on the canvas controlled by two separate sliders (OnVertZoomRelease & OnHorzZoomRelease)

void MyFrame::OnVertZoomRelease(wxCommandEvent &event)
{
  	int vert_sliderpos = vert_zoom_slider->GetValue(); 

  	float vert_zoom_fl;
  	int vert_zoom_int;

	// This makes it so that the zooming out part of the slider bar takes up as much space as the zooming in part

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

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);		// Sets the value of the text string used to display the zoom value
  
  	canvas->ZoomVert(vert_zoom_int);
}

void MyFrame::OnHorzZoomRelease(wxCommandEvent &event)
{
  	int horz_sliderpos = horz_zoom_slider->GetValue(); 

  	float horz_zoom_fl;
  	int horz_zoom_int;
  	
	// This makes it so that the zooming out part of the slider bar takes up as much space as the zooming in part

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

  	horz_zoom_value->SetLabel(horz_zoom_wxstr);		// Sets the value of the text string used to display the zoom value
  
  	canvas->ZoomHor(horz_zoom_int);
}

// Event handler for the run button

void MyFrame::OnRunButton(wxCommandEvent &event)
{
  	int n, ncycles;
	
  	ncycles = spin->GetValue();
  	if(ncycles > 0)
  	{
  		for(int i = 0; i < rcList.size(); i++)
		{
			bool ok = true;
			dmz -> setswitch(rcList[i].id, high, ok);
			if(!ok)
			{
				cout << _("Could not change RC state") << endl;
			}
		}
		
  		cyclescompleted = 0;
	  	dmz->initdevices ();
	  	mmz->resetmonitor ();
	  	
	  	
	  	string run_str = _("Simulation started for ") + to_string(spin->GetValue()) + _(" cycles");
	  	print_action(run_str);
	  	
	  	
  		run_flag = true;
  		runnetwork(ncycles);
  		canvas->run(ncycles);
	}
}

// Event handler for the continue button

void MyFrame::OnContinueButton(wxCommandEvent &event)
{
	if(run_flag == true)
	{
		int ncycles = spin->GetValue();
		if(ncycles > 0)
		{
			runnetwork(spin->GetValue());
			string continue_str = _("Simulation continued for an additional ") + to_string(spin->GetValue()) + _(" cycles");
			print_action(continue_str);
		
			canvas->cont(spin->GetValue());
		}
	}
	else
	{
		wxMessageBox( _("You think you could continue before running?"), _("Warning"), wxICON_EXCLAMATION);
	}	
}

// Event handler for saving the canvas contents which calls a function within the GLCanvas. The image is saved as a PNG

void MyFrame::SaveCanvas(wxCommandEvent &event)
{
	canvas->save_canvas();
}

// Calls a function to pan the canvas to the start of the simulation

void MyFrame::OnStart(wxCommandEvent &event)
{
	canvas->goToStart();
}

// Calls a function to pan the canvas to the end of the simulation

void MyFrame::OnEnd(wxCommandEvent &event)
{
	canvas->goToEnd();
}

// Similar to the zooming functions above but handles zooming in vertically and horizontally by the same amount using the shortcuts

void MyFrame::OnZoomIn(wxCommandEvent &event)
{
	int vert_sliderpos = vert_zoom_slider->GetValue();
	int horz_sliderpos = horz_zoom_slider->GetValue();
	vert_sliderpos += 5;
	horz_sliderpos += 5;
	if(vert_sliderpos > 100) vert_sliderpos = 100;	// Ensures that the value of the slider never goes above 100
	if(horz_sliderpos > 100) horz_sliderpos=100;
	
	vert_zoom_slider->SetValue(vert_sliderpos);
	horz_zoom_slider->SetValue(horz_sliderpos);
		
	float horz_zoom_fl;
  	int horz_zoom_int;
  	
  	// As previously, this makes it so that the zooming out part of the slider bar takes up as much space as the zooming in part

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
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);	// Converts a standard string to a wxString

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
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);	// Converts a standard string to a wxString

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
}

// Similar to the zooming functions above but handles zooming out vertically and horizontally by the same amount using the shortcuts

void MyFrame::OnZoomOut(wxCommandEvent &event)
{
	int vert_sliderpos = vert_zoom_slider->GetValue();
	int horz_sliderpos = horz_zoom_slider->GetValue();
	vert_sliderpos -= 5;
	horz_sliderpos -= 5;
	if(vert_sliderpos < 0) vert_sliderpos = 0;	// Ensures that the value of the slider never goes below 0
	if(horz_sliderpos <0) horz_sliderpos=0;
	
	vert_zoom_slider->SetValue(vert_sliderpos);
	horz_zoom_slider->SetValue(horz_sliderpos);
		
	float horz_zoom_fl;
  	int horz_zoom_int;
  	
  	// As previously, this makes it so that the zooming out part of the slider bar takes up as much space as the zooming in part  	

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
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8);	// Converts a standard string to a wxString

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
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);	// Converts a standard string to a wxString

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
  	
}

// Similar to the zooming functions above but handles zooming in/out vertically and horizontally by the same amount using the mouse wheel

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
	if(vert_sliderpos < 0) vert_sliderpos = 0;			// Ensures that the value of the slider never goes below 0
	if(horz_sliderpos <0) horz_sliderpos = 0;			
	if(vert_sliderpos > 100) vert_sliderpos = 100; 		// Ensures that the value of the slider never goes above 100
	if(horz_sliderpos > 100) horz_sliderpos= 100;
	
	vert_zoom_slider->SetValue(vert_sliderpos);
	horz_zoom_slider->SetValue(horz_sliderpos);
		
	float horz_zoom_fl;
  	int horz_zoom_int;

  	// As previously, this makes it so that the zooming out part of the slider bar takes up as much space as the zooming in part

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
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8); 	// Converts a standard string to a wxString

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
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);	// Converts a standard string to a wxString

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
	
}


  	// Function to run the network, derived from corresponding function in userint.cc
void MyFrame::runnetwork(int ncycles)
{
  	bool ok = true;
  	int n = ncycles;
  	
  	

  	while ((n > 0) && ok) 
  	{
  		for(int i = 0; i < rcList.size(); i++)
  		{
  			if(rcList[i].changeStateAt == cyclescompleted - n + ncycles)  				
  			{
  				dmz -> setswitch(rcList[i].id, low, ok);
  				if(!ok)
  				{
  					cout << _("Could not change RC state") << endl;
  				}
  			}
  		}
  		
  		
		dmz->executedevices (ok);
		if (ok) 
		{
	  		n--;
	  		mmz->recordsignals ();
		} 
		else cout << _("Error: network is oscillating") << endl;
  	}
  	if (ok) cyclescompleted = cyclescompleted + ncycles;
  	else cyclescompleted = 0;
	
}

// Function to print out the actions carried out by the user to the dialog box at the bottom of the application. This takes in standard strings and adds them as wxStrings to the wxListBox used for the dialog box

void MyFrame::print_action(string message)
{
  	string action_message = to_string(action_num) + ": " + message; 
  	
  	// Converts a regular string to a wxString
  	
  	wxString message_wxstr(action_message.c_str(), wxConvUTF8);
	
	wxString *addtolist = new wxString[1]{message_wxstr};
	action_list1->InsertItems(1, addtolist, 0);
	action_list1->EnsureVisible(0);
	
	action_num = action_num+1;	
}

// Updates slider values when zooming is caried out on the canvas. Zooming whilst on the canvas and zooming whilst on the frame are treated as two separate events

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
  	wxString horz_zoom_wxstr(horz_zoom_str.c_str(), wxConvUTF8); 	// Converts a standard string to a wxString

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
  	wxString vert_zoom_wxstr(vert_zoom_str.c_str(), wxConvUTF8);	// Converts a standard string to a wxString

  	vert_zoom_value->SetLabel(vert_zoom_wxstr);
  
  
  	canvas->ZoomVert(vert_zoom_int);
  	canvas->ZoomHor(horz_zoom_int);
}

// Function to display the help menu within a wxDialog when the buttons or the shortcuts are used

void MyFrame::ShowHelp(wxCommandEvent &event)
{
		ifstream hfile;		// Opens the help file, which is where the text in the dialog is read from
		char ch;
		string hstring, tmp;
		
		helpD = new wxDialog(this, -1, _("Help"), wxDefaultPosition, wxSize(350,500), wxDEFAULT_DIALOG_STYLE);
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
		textarea.SetFocus();	
		helpD->ShowModal();
}

