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
	/*for (int i = 0; i < 100; i++) {
		particles[i].x = window_width / 2;
		particles[i].y = window_height / 2;
		particles[i].size = (float)(rand() % 5) / 10.0f;
		particles[i].angle = (float)(rand() % 360);
		particles[i].velocityX = cos(particles[i].angle) * (float)(rand() % 5) / 5.0f;
		particles[i].velocityY = sin(particles[i].angle) * (float)(rand() % 5) / 5.0f;
		particles[i].color = Color(rand() % 255, rand() % 255, rand() % 255);
		//particles[i].acceleration = (float)(rand() % 5) / 5.0f;
	}*/
	

}


// Render one frame
void Application::Render(void)
{
	framebuffer.DrawImagePixels(toolbar, framebuffer.width, framebuffer.height, true);

	if (option == 1) {
		if (times == 4) {
			framebuffer.DrawLineDDA(pos[0], pos[1], pos[2], pos[3], Color::BLUE);
			times = 0;
		}
	}

	if (option == 2) {
		if (times == 4) {
			framebuffer.DrawLineBresenham(pos[0], pos[1], pos[2], pos[3], Color(255, 255, 255));
			times = 0;
		}
	}

	if (option == 3) {
		if (times == 4) {
			try {
				float radius = sqrt((pos[2] - pos[0]) * (pos[2] - pos[0]) + (pos[3] - pos[1]) * (pos[3] - pos[1]));
				framebuffer.DrawCircle(pos[0], pos[1], radius, Color::YELLOW, true);
				times = 0;
			}
			catch (std::exception& e) {
				for (int i = 0; i < 4; i++) {
					pos[i] = 0;
				}
			}
		}
	}

	if (option == 5) {
		
		for (int i = 0; i < 100; i++) {
			framebuffer.DrawCircle(particles[i].x, particles[i].y, particles[i].size, particles[i].color, true);
		}

	}


	framebuffer.Render();
}

// Called after render
void Application::Update(float seconds_elapsed)
{
	for (int i = 0; i < 100; i++) {
		//particles[i].x += particles[i].velocityX * seconds_elapsed;
		//particles[i].y += particles[i].velocityY * seconds_elapsed;
		//else {

		//particles[i].velocityX += particles[i].acceleration * cos(particles[i].angle) * seconds_elapsed;
		//particles[i].velocityY += particles[i].acceleration * sin(particles[i].angle) * seconds_elapsed;
		//particles[i].x += particles[i].velocityX * seconds_elapsed;
		//particles[i].y += particles[i].velocityY * seconds_elapsed;
		particles[i].velocityX += particles[i].acceleration * cos(particles[i].angle) * seconds_elapsed;
		particles[i].velocityY += particles[i].acceleration * sin(particles[i].angle) * seconds_elapsed;
		particles[i].x += particles[i].velocityX * seconds_elapsed;
		particles[i].y += particles[i].velocityY * seconds_elapsed;

		//if (sqrt(pow(particles[i].x - window_width, 2) + pow(particles[i].y - window_height, 2)) > 200) {
			//particles[i].x = window_width;
			//particles[i].y = window_height;
			//particles[i].velocityX = (float)(rand() % 10) / 10.0f - 0.5f;
			//particles[i].velocityY = (float)(rand() % 10) / 10.0f - 0.5f;
		//}


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
		pos[times] = mouse_position.x;
		pos[times + 1] = mouse_position.y;
		times++;
		times++;
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