#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>

const int width = 800;
const int height = 800;

float near_clip = 0.65;
float far_clip = 1000;

float range;

// fov isn't an angle, but the width of the frustum at distance 1
float fov = .5;

Uint32 *frame_buffer;

float *z_buffer;

float player_x = 0;
float player_y = 0;
float player_r = 0;

float move_speed = 10;

int wall_height = height;

SDL_Renderer *debug_renderer;

void draw_wall(int x0, int y0, int x1, int y1) {
    float new_x0 = (x0 - player_x) * cos(player_r) - (y0 - player_y) * sin(player_r);
    float new_y0 = (y0 - player_y) * cos(player_r) + (x0 - player_x) * sin(player_r);
    float new_x1 = (x1 - player_x) * cos(player_r) - (y1 - player_y) * sin(player_r);
    float new_y1 = (y1 - player_y) * cos(player_r) + (x1 - player_x) * sin(player_r);

//    SDL_RenderDrawLine(debug_renderer, new_x0 + 200, new_y0 + 200, new_x1 + 200, new_y1 + 200);

    int i = width * (new_x1 * (near_clip / new_y1)) + 400;
    int x_start = i;
    
    int w_height = abs(height * (wall_height / 2 * (near_clip / new_y1)) * 0.5);
    
    for (int j = -w_height; j < w_height; j++){
        int y = j + height / 2;
        if (y  >= height || y  < 0 || i >= width || i < 0)continue;
        
        frame_buffer[y * width + i] = 0xFFFFFFFF;
    }
        
        i = width * (new_x0 * (near_clip / new_y0)) + 400;
        int x_end = i;

        w_height = height * (wall_height / 2 * (near_clip / new_y0)) * 0.5;

        for (int j = -w_height; j < w_height; j++){
            int y = j + height / 2;

            if (y  >= height || y  < 0 || i >= width || i < 0)continue;
            frame_buffer[y * width + i] = 0xFFFFFFFF;
        }
   
        
        if (x_start > x_end){
            int temp = x_end;
            x_end = x_start;
            x_start = temp;
        }
        if(new_y0 <= 0 || new_y1 <= 0)
        return;

        printf("%d %d\n", x_start, x_end);

    for (int k = x_start; k < x_end; k++){
        if (k >= width || k < 0)continue;
        
        float w = (float)(k - x_start) / (x_end - x_start);

        w_height = (height * (wall_height / 2 * (near_clip / new_y0)) * 0.5) * (1.0 - w) + (height * (wall_height / 2 * (near_clip / new_y1)) * 0.5) * w;

        for (int j = -w_height; j < w_height; j++){
            int y = j + height / 2;

            if (y  >= height || y  < 0)continue;
            frame_buffer[y * width + k] = 0xFFFFFFFF;
        }
    }
}

int strafe_keys = 0;
int move_keys = 0;
int look_keys = 0;

int main(int arg_count, char* args[]){
	srand(time(NULL));

	int frame = 0;
 
    if(SDL_Init(SDL_INIT_VIDEO) != 0) printf("init failed\n");

	SDL_Window *window = SDL_CreateWindow("Doom Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
	if (!window) printf("window failed\n");
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) printf("renderer failed\n");
	
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture) printf("texture failed\n");

    // SDL_Window *debug_window = SDL_CreateWindow("Doom Renderer Debug", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 400, 400, SDL_WINDOW_OPENGL);
	// if (!debug_window) printf("window failed\n");
    
    // debug_renderer = SDL_CreateRenderer(debug_window, -1, SDL_RENDERER_ACCELERATED);
    // if (!debug_renderer) printf("renderer failed\n");
	
	frame_buffer = malloc(sizeof(Uint32) * width * height);
	
    z_buffer = malloc(sizeof(float) * width * height);

    int wall_count = 3;
    int wall_variable_size = 4;

    float *walls = malloc(sizeof(float) * wall_variable_size * wall_count);

    walls[0] = 0;
    walls[1] = 0;
    walls[2] = 200;
    walls[3] = 300;
    
    walls[4] = 200;
    walls[5] = 300;
    walls[6] = 600;
    walls[7] = 300;

    walls[8] = 600;
    walls[9] = 300;
    walls[10] = 800;
    walls[11] = 0;

	int running = 1;
	while (running){
		range = far_clip - near_clip;frame++;
		

    printf("%f\n", near_clip);		SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                    running = 0;
                    break;
            
                case SDL_KEYDOWN:
                    switch( event.key.keysym.sym ){
                        case SDLK_q:
                            near_clip -= 0.01;
                            break;
                        case SDLK_e:
                            near_clip += 0.01;
                            break;
              
                        case SDLK_a:
                            strafe_keys = -1;
                            break;
                        case SDLK_d:
                            strafe_keys = 1;
                            break;
                        case SDLK_w:
                            move_keys = -1;
                            break;
                        case SDLK_s:
                            move_keys = 1;
                            break;
                        case SDLK_LEFT:
                            look_keys = -1;
                            break;
                        case SDLK_RIGHT:
                            look_keys = 1;
                            break;
                        default:
                            break;
                    }break;
                
                case SDL_KEYUP:
                    switch( event.key.keysym.sym ){
                        case SDLK_a:
                            if(strafe_keys < 0)
                                strafe_keys = 0;
                            break;
                        case SDLK_d:
                            if(strafe_keys > 0)
                                strafe_keys = 0;
                            break;
                        case SDLK_w:
                            if(move_keys < 0)
                                move_keys = 0;
                            break;
                        case SDLK_s:
                            if(move_keys > 0)
                                move_keys = 0;
                            break;
                        case SDLK_LEFT:
                            if(look_keys < 0)
                                look_keys = 0;
                            break;
                        case SDLK_RIGHT:
                            if(look_keys > 0)
                                look_keys = 0;
                            break;
                        default:
                            break;
                                            }break;                        }
        }

        player_x -= move_keys * move_speed * sin(player_r);
        player_y -= move_keys * move_speed * cos(player_r);

        player_x += strafe_keys * move_speed * cos(player_r);
        player_y -= strafe_keys * move_speed * sin(player_r);

        player_r += look_keys * 0.1;

        for (int i = 0; i < width * height; i++) frame_buffer[i] = 0;

        SDL_SetRenderDrawColor(debug_renderer, 255, 255, 255, 255);
        for (int i = 0; i < wall_count * wall_variable_size; i += wall_variable_size){
            float x0 = walls[i + 0];
            float y0 = walls[i + 1];
            float x1 = walls[i + 2];
            float y1 = walls[i + 3];

            draw_wall(x0, y0, x1, y1);
                }
        
        SDL_RenderDrawPoint(debug_renderer, 200, 200);
            SDL_RenderDrawLine(debug_renderer, 200, 200, 200 - fov / 2 * 100, 200 + 100);
            SDL_RenderDrawLine(debug_renderer, 200, 200, 200 + fov / 2 * 100, 200 + 100);
        
        
        SDL_UpdateTexture(texture, NULL, frame_buffer, width * sizeof(Uint32));
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        
        SDL_RenderPresent(renderer);
        
        SDL_RenderPresent(debug_renderer);
        SDL_SetRenderDrawColor(debug_renderer, 0, 0, 0, 0);
        SDL_RenderClear(debug_renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(debug_renderer);
    // SDL_DestroyWindow(debug_window);
    SDL_Quit();
}
