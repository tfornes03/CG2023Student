#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "utils.h" 


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
	this->painting = false;
	this->option = 0;
	this->particles_count_black = 0;

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
	int d = 0;
	int valorx = 0;
	int valory = 0;
	float random = 0;
	
	for (int i = 0; i<600; i++) {
		random = rand()%1280; 
		valorx = random- framebuffer.width/2;
		random = rand()%720;
		valory = random - framebuffer.height/2;
		d = fmax(abs(valorx), abs(valory));
		random = rand()%640 + 641;
		particles[i].velocityX = (random - framebuffer.width / 2)/d;
		random = rand()%360 + 361;
		particles[i].velocityY = (random - framebuffer.height / 2)/d;
		random = rand()%640 + 1;
		particles[i].x = framebuffer.width / 2 + particles[i].velocityX;
		random = rand()%360 + 1;
		particles[i].y = framebuffer.height / 2 + particles[i].velocityY;
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
				framebuffer.DrawCircle(pos[0], pos[1], radius, colorpaint, fill);
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
		
		for (int i = 0; i < 600; i++) {
			Color color = Color(rand() % 255, rand() % 255, rand() % 255);
			framebuffer.DrawCircle(floor(particles[i].x + particles[i].velocityX), floor(particles[i].y + particles[i].velocityY), 1, color, true);
		}
		framebuffer.DrawCircle(framebuffer.width / 2, framebuffer.height / 2, 10, Color::BLACK, true);

		//if (particles_count_black > 10) {
		//	framebuffer.Fill(Color::BLACK);
		//	particles_coun}
	}
		
	
	if (option == 6) {
		framebuffer.Fill(Color::BLACK);
	}

	
	framebuffer.Render();

}


// Called after render
void Application::Update(float seconds_elapsed)

{
	if (option == 5) {
	for (int i = 0; i < 600; i++) {
		particles[i].x = particles[i].x + particles[i].velocityX;
		particles[i].y = particles[i].y + particles[i].velocityY;

		if (particles[i].x > 1280 || particles[i].x < 0) {
			particles[i].x = framebuffer.width / 2;
			particles[i].y = framebuffer.height / 2;
		}
		if (particles[i].y > 720 || particles[i].y < 0) {
			particles[i].x = framebuffer.width / 2;
			particles[i].y = framebuffer.height / 2;
		}
		if (particles_count_black > 5000) {
			framebuffer.Fill(Color::BLACK);
			particles_count_black = 0;
		}
		particles_count_black++;
	}
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
		case SDLK_6: option = 6; break;
		case SDLK_f: if (fill) { fill = false; } else { fill = true; } break;
	}
}

void Application::OnMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) {
		if (option == 4) {
			painting = true;
		}
		int clicin = framebuffer.ToolbarButton(mouse_position.x, mouse_position.y, framebuffer.height, true);
		if (clicin == 0 && !(option==0) && !(option == 5)) {
			pos[times] = mouse_position.x;
			pos[times + 1] = mouse_position.y;
			times++;
			times++;
		}
		else {
			if (clicin == 1) {
				framebuffer.Fill(Color::WHITE);
			}
			if (clicin == 2) {
				framebuffer.Fill(Color::BLACK);
			}
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
			if (clicin == 12) {
				option = 1;
			}
			if (clicin == 13) {
				option = 2;
			}
			if (clicin == 14) {
				option = 3;
				fill = false;
			}
			if (clicin == 15) {
				option = 3;
				fill = true;
			}
			if (clicin == 16) {
				option = 4;
			}
			if (clicin == 17) {
				option = 5;
			} 

		}
		
	}
}

void Application::OnMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) {
		painting = false;
	}
}

void Application::OnMouseMove(SDL_MouseButtonEvent event)
{
	if (painting) {
		framebuffer.DrawCircle(mouse_position.x, mouse_position.y, 10, colorpaint, true);
	}
	
}

void Application::OnFileChanged(const char* filename)
{ 
	Shader::ReloadSingleShader(filename);
}