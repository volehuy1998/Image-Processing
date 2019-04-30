#include <SDL2\SDL.h>
#include <SDL2\SDL_image.h>
#include <string>
#include <math.h>

SDL_Window * window;
SDL_Renderer * renderer;
SDL_bool quit = SDL_FALSE;

typedef unsigned char byte;
int angle = 0;
struct Image {
	SDL_Surface * surface;
	SDL_Texture * texture;
	std::string path;
	byte * pixels;
	int pitch;
	int w, h;
public:
	void setBlendMod(SDL_BlendMode blend) {
		SDL_SetTextureBlendMode(texture, blend);
	}
	void setAlphaMod(int alpha) {
		SDL_SetTextureAlphaMod(texture, alpha);
	}
	Image(std::string path, bool access = false) 
		: surface(nullptr),
		texture(nullptr),
		path("")
	{
		SDL_Surface * loadSurface = IMG_Load(path.c_str());
		if (!loadSurface) {
			SDL_Log("Load Surface %s error: %s", path.c_str(), SDL_GetError());
		} else {
			surface = SDL_ConvertSurfaceFormat(loadSurface, SDL_PIXELFORMAT_ABGR8888, NULL);
			if (!surface) {
				SDL_Log("Surface %s error: %s", path.c_str(), SDL_GetError());
			} else {
				if (access) {
					w = surface->w;
					h = surface->h;
					texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);
					SDL_LockTexture(texture, nullptr, (void **)(&pixels), &pitch);
					memcpy(pixels, surface->pixels, pitch * h);
					/*for (int i = 0; i < h; i++) {
						for (int j = 0; j < w; j++) {
							int index = pitch * i + j * 4;
							const int r = ((unsigned char *)surface->pixels)[index + 0];
							const int g = ((unsigned char *)surface->pixels)[index + 1];
							const int b = ((unsigned char *)surface->pixels)[index + 2];
							const int a = ((unsigned char *)surface->pixels)[index + 3];
							pixels[index + 0] = r;
							pixels[index + 1] = g;
							pixels[index + 2] = b;
							pixels[index + 3] = a;
						}
					}*/
					SDL_UnlockTexture(texture);
				} else {
					texture = SDL_CreateTextureFromSurface(renderer, surface);
				}
				SDL_FreeSurface(loadSurface);
				if (!texture) {
					SDL_Log("Surface error: %s", SDL_GetError());
				}
			}
		}
	}
	void render(int x, int y, int w, int h, bool flip = false) {
		SDL_Rect rect = { x, y, w, h};
		SDL_RenderCopyEx(renderer, texture, nullptr, &rect, 0, nullptr, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	}
	~Image() {
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}
};

int main(int argv, char ** argc) {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG);
	window = SDL_CreateWindow("Image processing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 600, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
	Image ori_img("ori.jpg"), mod_img("ori.jpg", true);
	mod_img.setBlendMod(SDL_BLENDMODE_BLEND);

	SDL_Point mod_center = { 750, 250 };

	int w = 450;
	SDL_Point points[2];
	points[0].x = (1000 - w) / 2;
	points[0].y = 550;
	points[1].x = points[0].x + 450;
	points[1].y = points[0].y;

	SDL_Rect control = {points[1].x, points[0].y - 20, 20, 40};
	bool active = false;
	Uint8 a = 255;
	int t = 30;
	while (quit == SDL_FALSE) {
		SDL_Event e;
		while( SDL_PollEvent( &e ) != 0 ) {
			switch (e.type) {
			case SDL_QUIT:
				quit = SDL_TRUE; break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_UP) {
					SDL_LockTexture(mod_img.texture, nullptr, (void **)(&mod_img.pixels), &mod_img.pitch);
					for (int i = 0; i < mod_img.h; i++) {
						for (int j = 0; j < mod_img.w; j++) {
							int index = mod_img.pitch * i + j * 4;
							const int r = mod_img.pixels[index + 0];
							const int g = mod_img.pixels[index + 1];
							const int b = mod_img.pixels[index + 2];
							mod_img.pixels[index + 0] = r + t > 255 ? 255 : r + t;
							mod_img.pixels[index + 1] = g + t > 255 ? 255 : g + t;
							mod_img.pixels[index + 2] = b + t > 255 ? 255 : b + t;
						}
					}
					SDL_UnlockTexture(mod_img.texture);
				}
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT) {
						if      (e.motion.x < points[0].x) { control.x = points[0].x; } 
						else if (e.motion.x > points[1].x) { control.x = points[1].x; } 
						else								 control.x = e.motion.x;
						a = 1.0f * (control.x - points[0].x) * 255 / (points[1].x - points[0].x);
				}
				break;
			default: break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0x0);
		SDL_RenderClear(renderer);
		ori_img.render(0, 0, 500, 500);
		mod_img.render(500, 0, 500, 500, true);
		mod_img.setAlphaMod(a);

		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x0);
		SDL_RenderFillRect(renderer, &control);
		SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[1].x, points[1].y);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}

//#include <SDL2/SDL.h>
//#include <SDL2/SDL_image.h>
//#include <string>
//
//SDL_Window* gWindow = NULL;
//SDL_Renderer* gRenderer = NULL;
//
//struct Image {
//	Image(std::string path, bool access = false) {
//		SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
//		if( loadedSurface == NULL ) {
//			printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
//		}
//		else {
//			SDL_Surface* surface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_ABGR8888, NULL);
//			if (access) {
//				unsigned char * pixels;
//				int pitch;
//				const int w = surface->w;
//				const int h = surface->h;
//				mTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);
//				SDL_LockTexture(mTexture, nullptr, (void **)(&pixels), &pitch);
//				memcpy(pixels, surface->pixels, pitch * h);
//				SDL_UnlockTexture(mTexture);
//			} else {
//				mTexture = SDL_CreateTextureFromSurface(gRenderer, surface);
//			}
//			if( mTexture == NULL ) {
//				printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
//			}
//			SDL_FreeSurface( loadedSurface );
//			SDL_FreeSurface(surface);
//		}
//	}
//	~Image() {
//		SDL_DestroyTexture( mTexture );
//	}
//	void setBlendMode( SDL_BlendMode blending ) {
//		SDL_SetTextureBlendMode( mTexture, blending );
//	}
//	void setAlpha( Uint8 alpha ) {
//		SDL_SetTextureAlphaMod( mTexture, alpha );
//	}
//	void render( int x, int y) {
//		SDL_Rect renderQuad = { x, y, 500, 500 };
//		SDL_RenderCopy( gRenderer, mTexture, nullptr, &renderQuad );
//	}
//	SDL_Texture* mTexture;
//};
//
//int main( int argc, char* args[] )
//{
//	SDL_Init( SDL_INIT_EVERYTHING);
//	gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000, 500, SDL_WINDOW_SHOWN );
//	gRenderer = SDL_CreateRenderer( gWindow, -1, 0 );
//	IMG_Init( IMG_INIT_JPG );
//
//	Image gModulatedTexture("ori.jpg", true);
//	gModulatedTexture.setBlendMode(SDL_BLENDMODE_BLEND);
//	bool quit = false;
//	Uint8 a = 255;
//	
//	while( !quit ) {
//		SDL_Event e;
//		while( SDL_PollEvent( &e ) != 0 ) {
//			switch (e.type) {
//			case SDL_QUIT:
//					quit = true; break;
//			case SDL_KEYDOWN:
//				switch (e.key.keysym.sym) {
//				case SDLK_w:
//					a = a + 32 > 255 ? 255 : a + 32;
//					break;
//				case SDLK_s:
//					a = a - 32 < 0 ? 0 : a - 32;
//					break;
//				}
//			}
//		}
//
//		SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
//		SDL_RenderClear( gRenderer );
//
//		gModulatedTexture.setAlpha( a );
//		gModulatedTexture.render( 500, 0 );
//
//		SDL_RenderPresent( gRenderer );
//	}
//
//	SDL_DestroyRenderer( gRenderer );
//	SDL_DestroyWindow( gWindow );
//
//	IMG_Quit();
//	SDL_Quit();
//
//	return 0;
//}