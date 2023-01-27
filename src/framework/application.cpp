#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "utils.h" 

int option = 0;

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->mouse_state = 0;
	this->time = 0.f;
	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(nullptr);
	this->colorpaint = Color(255,255,255);

	this->framebuffer.Resize(w, h);
	if (toolbar.LoadPNG("images/toolbar.png") == false) {
		std::cout << "Image not found" << std::endl;
	}
}

Application::~Application()
{
	SDL_DestroyWindow(window);
}

void Application::Init(void)
{
	std::cout << "Initiating app..." << std::endl;
	
	for (int i = 0; i<100; i++) {
		particles[i].x = window_width/2;
		particles[i].y = window_height/2;
		particles[i].velocityX = rand()%10 + 1;
		particles[i].velocityY = rand()%10 + 1;
		particles[i].color = Color(rand() % 255, rand() % 255, rand() % 255);

 	}

}


// Render one frame
void Application::Render(void)
{

	framebuffer.DrawImagePixels(toolbar, framebuffer.width, framebuffer.height, true);

	if (option == 1) {
		if (times == 4) {
			framebuffer.DrawLineDDA(pos[0], pos[1], pos[2], pos[3], colorpaint);
			times = 0;
		}
	}

	if (option == 2) {
		if (times == 4) {
			framebuffer.DrawLineBresenham(pos[0], pos[1], pos[2], pos[3], colorpaint);
			times = 0;
		}
	}

	if (option == 3) {
		if (times == 4) {
			try {
				float radius = sqrt((pos[2] - pos[0]) * (pos[2] - pos[0]) + (pos[3] - pos[1]) * (pos[3] - pos[1]));
				framebuffer.DrawCircle(pos[0], pos[1], radius, colorpaint, true);
				times = 0;
			}
			catch (std::exception& e) {
				for (int i = 0; i < 4; i++) {
					pos[i] = 0;
				}
			}
		}
	}
	if (option == 4) {
		framebuffer.DrawImagePixels(toolbar, framebuffer.width, framebuffer.height, true);
	}

	if (option == 5) {
		/*
		for (int i = 0; i < 100; i++) {
			framebuffer.DrawLineDDA(float(particles[i].x), float(particles[i].y), float(particles[i].x) + particles[i].velocityX), float(particles[i].y + particles[i].velocityY), Color::BLUE);
		}
		*/
	}
	if (option == 6) {
		framebuffer.Fill(Color::BLACK);
	}

	framebuffer.Render();

}


// Called after render
void Application::Update(float seconds_elapsed)
{
	for (int i = 0; i < 1000; i++) {
		particles[i].x = particles[i].x + particles[i].velocityX;
		particles[i].y = particles[i].y + particles[i].velocityY;
	}
}


//keyboard press event 
void Application::OnKeyPressed( SDL_KeyboardEvent event )
{
	// KEY CODES: https://wiki.libsdl.org/SDL2/SDL_Keycode
	switch(event.keysym.sym) {
		case SDLK_ESCAPE: exit(0); break; // ESC key, kill the app
		case SDLK_1: option = 1; break;
		case SDLK_2: option = 2; break;
		case SDLK_3: option = 3; break;
		case SDLK_4: option = 4; break;
		case SDLK_5: option = 5; break;
	}
}

void Application::OnMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) {
		int clicin = framebuffer.ToolbarButton(mouse_position.x, mouse_position.y, framebuffer.height, true);
		std::cout << mouse_position.x << std::endl;
		std::cout << mouse_position.y << std::endl;
		std::cout << clicin << std::endl;
		if (clicin == 0) {
			pos[times] = mouse_position.x;
			pos[times + 1] = mouse_position.y;
			times++;
			times++;
		}
		else {
			if (clicin == 4) {
				colorpaint = Color::BLACK;
			}
			if (clicin == 5) {
				colorpaint = Color::RED;
			}
			if (clicin == 6) {
				colorpaint = Color::GREEN;
			}
			if (clicin == 7) {
				colorpaint = Color::BLUE;
			}
			if (clicin == 8) {
				colorpaint = Color::YELLOW;
			}
			if (clicin == 9) {
				colorpaint = Color::PURPLE;
			}
			if (clicin == 10) {
				colorpaint = Color::CYAN;
			}
			if (clicin == 11) {
				colorpaint = Color::WHITE;
			}
		}
		
	}
}

void Application::OnMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) {

	}
}

void Application::OnMouseMove(SDL_MouseButtonEvent event)
{
	
}

void Application::OnFileChanged(const char* filename)
{ 
	Shader::ReloadSingleShader(filename);
}