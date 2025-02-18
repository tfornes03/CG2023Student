#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "GL/glew.h"
#include "../extra/picopng.h"
#include "image.h"
#include "utils.h"
#include "camera.h"
#include "mesh.h"

Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

// Copy constructor
Image::Image(const Image& c)
{
	pixels = NULL;
	width = c.width;
	height = c.height;
	bytes_per_pixel = c.bytes_per_pixel;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*bytes_per_pixel);
	}
}

// Assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	bytes_per_pixel = c.bytes_per_pixel;

	if(c.pixels)
	{
		pixels = new Color[width*height*bytes_per_pixel];
		memcpy(pixels, c.pixels, width*height*bytes_per_pixel);
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete pixels;
}

void Image::Render()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(width, height, bytes_per_pixel == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pixels);

}

// Change image size (the old one will remain in the top-left corner)
void Image::Resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = GetPixel(x,y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

// Change image size and scale the content
void Image::Scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = GetPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::GetArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++x)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.SetPixel( x, y, GetPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::FlipY()
{
	int row_size = bytes_per_pixel * width;
	Uint8* temp_row = new Uint8[row_size];
#pragma omp simd
	for (int y = 0; y < height * 0.5; y += 1)
	{
		Uint8* pos = (Uint8*)pixels + y * row_size;
		memcpy(temp_row, pos, row_size);
		Uint8* pos2 = (Uint8*)pixels + (height - y - 1) * row_size;
		memcpy(pos, pos2, row_size);
		memcpy(pos2, temp_row, row_size);
	}
	delete[] temp_row;
}

bool Image::LoadPNG(const char* filename, bool flip_y)
{
	std::string sfullPath = absResPath(filename);
	std::ifstream file(sfullPath, std::ios::in | std::ios::binary | std::ios::ate);

	// Get filesize
	std::streamsize size = 0;
	if (file.seekg(0, std::ios::end).good()) size = file.tellg();
	if (file.seekg(0, std::ios::beg).good()) size -= file.tellg();

	if (!size)
		return false;

	std::vector<unsigned char> buffer;

	// Read contents of the file into the vector
	if (size > 0)
	{
		buffer.resize((size_t)size);
		file.read((char*)(&buffer[0]), size);
	}
	else
		buffer.clear();

	std::vector<unsigned char> out_image;

	if (decodePNG(out_image, width, height, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size(), true) != 0)
		return false;

	size_t bufferSize = out_image.size();
	unsigned int originalBytesPerPixel = (unsigned int)bufferSize / (width * height);
	
	// Force 3 channels
	bytes_per_pixel = 3;

	if (originalBytesPerPixel == 3) {
		pixels = new Color[bufferSize];
		memcpy(pixels, &out_image[0], bufferSize);
	}
	else if (originalBytesPerPixel == 4) {

		unsigned int newBufferSize = width * height * bytes_per_pixel;
		pixels = new Color[newBufferSize];

		unsigned int k = 0;
		for (unsigned int i = 0; i < bufferSize; i += originalBytesPerPixel) {
			pixels[k] = Color(out_image[i], out_image[i + 1], out_image[i + 2]);
			k++;
		}
	}

	// Flip pixels in Y
	if (flip_y)
		FlipY();

	return true;
}

// Loads an image from a TGA file
bool Image::LoadTGA(const char* filename, bool flip_y)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int imageSize;
	unsigned int bytesPerPixel;

    std::string sfullPath = absResPath( filename );

	FILE * file = fopen( sfullPath.c_str(), "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << sfullPath.c_str() << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;
    
	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];
    
	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		fclose(file);
		delete tgainfo;
		return NULL;
	}
    
	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;
    
	tgainfo->data = new unsigned char[imageSize];
    
	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;
            
		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	// Save info in image
	if(pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	// Convert to float all pixels
	for (unsigned int y = 0; y < height; ++y) {
		for (unsigned int x = 0; x < width; ++x) {
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			// Make sure we don't access out of memory
			if( (pos < imageSize) && (pos + 1 < imageSize) && (pos + 2 < imageSize))
				SetPixel(x, height - y - 1, Color(tgainfo->data[pos + 2], tgainfo->data[pos + 1], tgainfo->data[pos]));
		}
	}

	// Flip pixels in Y
	if (flip_y)
		FlipY();

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::SaveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	std::string fullPath = absResPath(filename);
	FILE *file = fopen(fullPath.c_str(), "wb");
	if ( file == NULL )
	{
		perror("Failed to open file: ");
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	// Convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[y*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);

	return true;
}

void Image::DrawRect(int x, int y, int w, int h, const Color& c)
{
	for (int i = 0; i < w; ++i) {
		SetPixel(x + i, y, c);
		SetPixel(x + i, y + h, c);
	}

	for (int j = 0; j < h; ++j) {
		SetPixel(x, y + j, c);
		SetPixel(x + w, y + j, c);
	}
}

#ifndef IGNORE_LAMBDAS

// You can apply and algorithm for two images and store the result in the first one
// ForEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void ForEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

#endif


//*******OUR FUNCTIONS*********
void Image::DrawLineDDA(int x0, int y0, int x1, int y1, const Color& c) {
	float dx = x1 - x0;
	float dy = y1 - y0;
	float d = fmax(abs(dx), abs(dy));
	Vector2 v;
	v.x = dx / d;
	v.y = dy / d;
	Vector2 start;
	int x = x0;
	int y = y0;
	start.x = (float) x;
	start.y = (float) y;
	for (int i = 0; i < d; i++) {
		start.x += v.x;
		start.y += v.y;
		SetPixelSafe(floor(start.x), floor(start.y), c);
	}
}
void Image::DrawLineBresenham(int x0, int y0, int x1, int y1, const Color& c) {
	int x = abs(x1 - x0);
	int y = abs(y1 - y0);
	int dirx, diry;
	if (x0 < x1) { dirx = 1; }
	else { dirx = -1; }
	if (y0 < y1) { diry = 1; }
	else { diry = -1; }
	int d = x - y;
		
	while (!(x0 == x1) && !(y0 == y1)) {
		SetPixel(x0, y0, c);
		int d2 = 2 * d;
		if (d2 < x) {
			d = d + x;
			y0 = y0 + diry;
		}
		if (d2 > -y) {
			d = d - y;
			x0 = x0 + dirx;
		}
	}
}

void Image::DrawCircle(int x0, int y0, int r, const Color& c, bool fill) {
	int x = r;
	int y = 0;
	int decision = 1 - x;
	while (y <= x) {
		if (fill) {
			for (int i = x0 - x; i <= x0 + x; i++) {
				SetPixelSafe(i, y0 + y, c);
				SetPixelSafe(i, y0 - y, c);
			}
			for (int i = x0 - y; i <= x0 + y; i++) {
				SetPixelSafe(i, y0 + x, c);
				SetPixelSafe(i, y0 - x, c);
			}
		}
		else {
			SetPixelSafe(x0 + x, y0 + y, c);
			SetPixelSafe(x0 - x, y0 + y, c);
			SetPixelSafe(x0 + x, y0 - y, c);
			SetPixelSafe(x0 - x, y0 - y, c);
			SetPixelSafe(x0 + y, y0 + x, c);
			SetPixelSafe(x0 - y, y0 + x, c);
			SetPixelSafe(x0 + y, y0 - x, c);
			SetPixelSafe(x0 - y, y0 - x, c);
		}
		y++;

		if (decision <= 0) {
			decision += 2 * y + 1;
		}
		else {
			x--;
			decision += 2 * (y - x) + 1;
		}
	}
}
void Image::DrawImagePixels(const Image& image, int x, int y, bool top) {
	int w = image.width;
	int h = image.height;
	if (top) {
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				Color c = image.GetPixel(i, j);
				SetPixel(i, y - h + j, c);
			}
		}
	}
	else
	{
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				Color c = image.GetPixel(i, j);
				SetPixel(i, j, c);
			}
		}
	}

}

int Image::ToolbarButton(int mousex, int mousey, int h, bool top) {
	if(top){
		if ((mousex > 7 && mousex < 794) && (mousey < (h-15) && mousey > (h-47))) {
			if ((mousex > 7 && mousex < 32) && (mousey < (h - 15) && mousey > (h - 47))) {
				//Fill white
				return 1;
			}
			else if ((mousex > 36 && mousex < 60) && (mousey < (h - 15) && mousey > (h - 47))) {
				//Fill Black
				return 2;
			}
			else if ((mousex > 64 && mousex < 96) && (mousey < (h - 15) && mousey > (h - 47))) {
				//Save
				return 3;
			}
			else if ((mousex > 116 && mousex < 144) && (mousey < (h - 15) && mousey > (h - 47))) {
				//Black
				return 4;
			}
			else if ((mousex > 167 && mousex < 193) && (mousey < (h - 15) && mousey > (h - 47))) {
				//Red
				//std::cout << "in" << std::endl;
				return 5;
			}
			else if ((mousex > 216 && mousex < 244) && (mousey < (h - 15) && mousey > (h - 47))) {
				//Green
				return 6;
			}
			else if ((mousex > 266 && mousex < 294) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Blue
				return 7;
			}
			else if ((mousex > 316 && mousex < 344) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Yellow
				return 8;
			}
			else if ((mousex > 366 && mousex < 394) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Pink
				return 9;
			}
			else if ((mousex > 416 && mousex < 444) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Cian
				return 10;
			}
			else if ((mousex > 466 && mousex < 494) && (mousey < (h - 15) && mousey >(h - 47))) {
				//White
				return 11;
			}
			else if ((mousex > 516 && mousex < 544) && (mousey < (h - 15) && mousey >(h - 47))) {
				//DDA
				return 12;
			}
			else if ((mousex > 566 && mousex < 594) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Bresenham
				return 13;
			}
			else if ((mousex > 616 && mousex < 644) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Circles
				return 14;
			}
			else if ((mousex > 666 && mousex < 694) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Circles Fill
				return 15;
			}
			else if ((mousex > 716 && mousex < 744) && (mousey < (h - 15) && mousey >(h - 47))) {
				//Painting
				return 16;
			}
			else if ((mousex > 766 && mousex < 794) && (mousey < (h - 15) && mousey >(h - 47))) {
				//AnimationParticles
				return 17;
			}
		}
		else { return 0; }
	}
	else {
		if ((mousex > 7 && mousex < 794) && (mousey < (63-15) && mousey >(63 - 47))) {
			if ((mousex > 7 && mousex < 32) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Fill white
				return 1;
			}
			else if ((mousex > 36 && mousex < 60) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Fill Black
				return 2;
			}
			else if ((mousex > 64 && mousex < 96) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Save
				return 3;
			}
			else if ((mousex > 116 && mousex < 144) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Black
				return 4;
			}
			else if ((mousex > 167 && mousex < 193) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Red
				//std::cout << "in" << std::endl;
				return 5;
			}
			else if ((mousex > 216 && mousex < 244) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Green
				return 6;
			}
			else if ((mousex > 266 && mousex < 294) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Blue
				return 7;
			}
			else if ((mousex > 316 && mousex < 344) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Yellow
				return 8;
			}
			else if ((mousex > 366 && mousex < 394) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Pink
				return 9;
			}
			else if ((mousex > 416 && mousex < 444) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Cian
				return 10;
			}
			else if ((mousex > 466 && mousex < 494) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//White
				return 11;
			}
			else if ((mousex > 516 && mousex < 544) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//DDA
				return 12;
			}
			else if ((mousex > 566 && mousex < 594) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Bresemham
				return 13;
			}
			else if ((mousex > 616 && mousex < 644) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Circles
				return 14;
			}
			else if ((mousex > 666 && mousex < 694) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Circle Fill
				return 15;
			}
			else if ((mousex > 716 && mousex < 744) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Painting
				return 16;
			}
			else if ((mousex > 766 && mousex < 794) && (mousey < (63 - 15) && mousey >(63 - 47))) {
				//Painting
				return 16;
			}
		}
		else { return 0; }

	}
	
}



FloatImage::FloatImage(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new float[width * height];
	memset(pixels, 0, width * height * sizeof(float));
}

// Copy constructor
FloatImage::FloatImage(const FloatImage& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width * height];
		memcpy(pixels, c.pixels, width * height * sizeof(float));
	}
}

// Assign operator
FloatImage& FloatImage::operator = (const FloatImage& c)
{
	if (pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width * height * sizeof(float)];
		memcpy(pixels, c.pixels, width * height * sizeof(float));
	}
	return *this;
}

FloatImage::~FloatImage()
{
	if (pixels)
		delete pixels;
}

// Change image size (the old one will remain in the top-left corner)
void FloatImage::Resize(unsigned int width, unsigned int height)
{
	float* new_pixels = new float[width * height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for (unsigned int x = 0; x < min_width; ++x)
		for (unsigned int y = 0; y < min_height; ++y)
			new_pixels[y * width + x] = GetPixel(x, y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}