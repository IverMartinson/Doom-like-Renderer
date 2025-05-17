#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>

const int width = 800;
const int height = 800;

const float near_clip = 0.1;
const float far_clip = 1000;

const float range = far_clip - near_clip;

// fov isn't an angle, but the width of the frustum at distance 1
const float fov = 100;

Uint32 *frame_buffer;

float *z_buffer;

float player_x = 0;
float player_y = 0;
float player_r = 0;

void draw_wall(int x0, int y0, int x1, int y1) {
    float new_y0 = y0 * cos(player_r) + x0 * sin(player_r) + player_x;
    float new_x0 = x0 * cos(player_r) - y0 * sin(player_r) + player_y;
    float new_y1 = y1 * cos(player_r) + x1 * sin(player_r) + player_x;
    float new_x1 = x1 * cos(player_r) - y1 * sin(player_r) + player_y;
    
    float h_scale_0 = new_y0 / fov;
    float h_scale_1 = new_y1 / fov;

    int x_range = new_x1 / h_scale_1 - new_x0 / h_scale_0;
    
    if (x_range <= 0){
        return;
    }

    for (int i = 0; i < x_range; i++){
        float w = (float)i / x_range;

        float distance = new_y0 * (1.0 - w) + new_y1 * w - near_clip;

        if (distance < 0 || distance > far_clip - near_clip){
            return;
        }

        int vertical_scale = height * ((float)(range - distance) / (float)range);

        for (int j = -vertical_scale / 2; j < vertical_scale / 2; j++){
            int x = i + (int)(new_x0 / h_scale_0) + width / 2;
            int y = j + height / 2;

            if (x < 0 || x > width || y < 0 || y > height)
                continue;
            
            frame_buffer[y * width + x] = 0xFFFFFFFF;
        }
    }
}

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

	frame_buffer = malloc(sizeof(Uint32) * width * height);
	
    z_buffer = malloc(sizeof(float) * width * height);

    int wall_count = 1;
    int wall_variable_size = 4;

    float *walls = malloc(sizeof(float) * wall_variable_size * wall_count);

    walls[0] = -300;
    walls[1] = 100;
    walls[2] = 300;
    walls[3] = 200;

	int running = 1;
	while (running){
		frame++;
		
		SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                    running = 0;
            }
        }

        player_x = sin(frame * 0.1) * 50;
        player_y = cos(frame * 0.1) * 50;
        // walls[1] += cos(frame * 0.1);
        // walls[3] += cos(frame * 0.1);

        for (int i = 0; i < width * height; i++) frame_buffer[i] = 0;

        for (int i = 0; i < wall_count * wall_variable_size; i += wall_variable_size){
            float x0 = walls[i + 0];
            float y0 = walls[i + 1];
            float x1 = walls[i + 2];
            float y1 = walls[i + 3];

            draw_wall(x0, y0, x1, y1);
        }

        SDL_UpdateTexture(texture, NULL, frame_buffer, width * sizeof(Uint32));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_RenderPresent(renderer);
	}

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
