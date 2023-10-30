#include <SDL.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include "perlin.h"

const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;
double FOV = 1;

bool init();
void close();

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

class Vertex {
public:
	double x, y, z;
	int r, g, b;
	Vertex() { x = 0; y = 0; z = 0; r = 0; g = 0; b = 0; }
	Vertex(double a, double t, double c) { x = a; y = t; z = c; r = 0; g = 0; b = 0; }
	Vertex(double a, double t, double c, int d, int e, int f) { x = a; y = t; z = c; r = d; g = e; b = f; }
	Vertex operator + (Vertex const &obj) {
		Vertex out;
		out.x = x + obj.x;
		out.y = y + obj.y;
		out.z = z + obj.z;
		return out;
	}
	Vertex operator + (double v) {
		return Vertex(x + v, y + v, z + v);
	}
	Vertex operator - (Vertex const& obj) {
		Vertex out;
		out.x = x - obj.x;
		out.y = y - obj.y;
		out.z = z - obj.z;
		return out;
	}
	Vertex operator - (double v) {
		return Vertex(x - v, y - v, z - v);
	}
	Vertex operator * (double v) {
		return Vertex(x * v, y * v, z * v);
	}
	void operator = (Vertex const& obj) {
		x = obj.x;
		y = obj.y;
		z = obj.z;
	}
	void operator = (double v) {
		x = v; y = v; z = v;
	}
	void operator += (Vertex const& obj) {
		x += obj.x;
		y += obj.y;
		z += obj.z;
	}
	void operator += (double v) {
		x += v; y += v; z += v;
	}
	void operator -= (Vertex const& obj) {
		x -= obj.x;
		y -= obj.y;
		z -= obj.z;
	}
	void operator -= (double v) {
		x -= v; y -= v; z -= v;
	}
};

bool init() {
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL cound not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else {
		gWindow = SDL_CreateWindow("3d test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if (gRenderer == NULL) {
				printf("Renderer could not be created. SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else {
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
			}
		}
	}
	return success;
}

void close() {
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	SDL_Quit();
}

Vertex rotateX(Vertex origin, Vertex vertex, double theta) {
	double sin_theta = sin(theta * M_PI * 2);
	double cos_theta = cos(theta * M_PI * 2);
	Vertex d = vertex - origin;
	double y = origin.y + (d.y * cos_theta - d.z * sin_theta);
	double z = origin.z + (d.z * cos_theta + d.y * sin_theta);
	return Vertex(vertex.x, y, z);
}

Vertex rotateY(Vertex origin, Vertex vertex, double theta) {
	double sin_theta = sin(theta * M_PI * 2);
	double cos_theta = cos(theta * M_PI * 2);
	Vertex d = vertex - origin;
	double x = origin.x + (d.x * cos_theta + d.z * sin_theta);
	double z = origin.z + (d.z * cos_theta - d.x * sin_theta);
	return Vertex(x, vertex.y, z);
}

Vertex rotateZ(Vertex origin, Vertex vertex, double theta) {
	double sin_theta = sin(theta * M_PI * 2);
	double cos_theta = cos(theta * M_PI * 2);
	Vertex d = vertex - origin;
	double x = origin.x + (d.x * cos_theta - d.y * sin_theta);
	double y = origin.y + (d.y * cos_theta + d.x * sin_theta);
	return Vertex(x, y, vertex.z);
}

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<int> edges;
	Vertex origin;
	int r = 255;
	int g = 255;
	int b = 255;
	int a = 255;
	Mesh(std::vector<Vertex> a, std::vector<int> b, Vertex c) {
		vertices = a;
		edges = b;
		origin = c;
	}
	void translate(Vertex vertex) {
		for (int i = 0; i < vertices.size(); i++ ) {
			vertices[i] += vertex;
		}
		origin += vertex;
	}
	void absolute_translate(Vertex vertex) {
		for (int i = 0; i < vertices.size(); i++) {
			vertices[i] -= origin;
			vertices[i] += vertex;
		}
		origin = vertex;
	}
	void rotate(Vertex vertex, Vertex o) {
		for (int i = 0; i < vertices.size(); i++ ) {
			vertices[i] = rotateX(o, vertices[i], vertex.x);
			vertices[i] = rotateY(o, vertices[i], vertex.y);
			vertices[i] = rotateZ(o, vertices[i], vertex.z);
		}
	}
};

struct Camera {
	Vertex position = Vertex();
	Vertex rotation = Vertex();
};

Mesh createSphere(Vertex position, double radius, int resolution) {
	double step = M_PI / resolution;
	double r,x,y,z;
	std::vector<Vertex> vertices = {};
	std::vector<int> edges = {};
	for (int latitude = 0; latitude < resolution; latitude++) {
		r = sin(step * latitude) * radius;
		y = cos(step * latitude) * radius;
		for (int longitude = 0; longitude < resolution; longitude++) {
			x = cos(2 * step * longitude) * r;
			z = sin(2 * step * longitude) * r;
			vertices.push_back(position + Vertex(x, y, z));
		}
	}
	vertices.push_back(Vertex(position.x, position.y - radius, position.z));
	for (int i = 1; i < resolution + 2; i++) {
		edges.push_back(vertices.size() - 1);
		edges.push_back(vertices.size() - i);
	}
	for (int latitude = 0; latitude < resolution; latitude++) {
		for (int longitude = 0; longitude < resolution; longitude++) {
			if ( latitude < resolution - 1 ) {
				edges.push_back(latitude * resolution + longitude);
				edges.push_back((1+latitude) * resolution + longitude);
			}
			if (longitude < resolution - 1) {
				edges.push_back(latitude * resolution + longitude);
				edges.push_back(latitude * resolution + longitude + 1);
			}
			else {
				edges.push_back(latitude * resolution + longitude);
				edges.push_back(latitude * resolution);
			}
		}
	}
	return Mesh(vertices, edges, position);
}

Mesh createCone(Vertex position, double radius, double height, int resolution) {
	std::vector<Vertex> vertices = { Vertex(position.x, position.y - height / 2, position.z) };
	std::vector<int> edges = {};
	double step = 2 * M_PI / resolution;
	for (int i = 0; i < resolution; i++) {
		vertices.push_back(Vertex(position.x + cos(step * i) * radius, position.y + height / 2, position.z + sin(step * i) * radius));
		edges.push_back(0);
		edges.push_back(i + 1);
		if (i > 1) {
			edges.push_back(i);
			edges.push_back(i - 1);
		}
	}
	edges.push_back(vertices.size()-1);
	edges.push_back(1);
	return Mesh(vertices, edges, position);
}

Mesh generateTerrain(Vertex position, Vertex noise_position, double size, double resolution, double amplitude, double smoothness) {
	double step = size/resolution;
	std::vector<Vertex> vertices = {};
	std::vector<int> edges = {};
	for (int z = 0; z < resolution; z++) {
		for (int x = 0; x < resolution; x++) {
			vertices.push_back(Vertex(step * x + position.x, amplitude * noise(smoothness*(noise_position.x+x),smoothness*(noise_position.z+z),noise_position.y) + position.y, step * z + position.z ));
			if (z < resolution - 1) {
				edges.push_back(z * resolution + x);
				edges.push_back((z + 1) * resolution + x);
			}
			if (x < resolution - 1) {
				edges.push_back(z * resolution + x);
				edges.push_back(z * resolution + x + 1);
			}
		}
	}
	return Mesh(vertices, edges, position);
}

double get_distance(Vertex a, Vertex b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

void render(std::vector<Mesh> meshes, SDL_Renderer* renderer, Camera camera ) {
	double w = SCREEN_WIDTH/2;
	double h = SCREEN_HEIGHT / 2;
	Vertex a = Vertex();
	Vertex b = Vertex();
	std::vector<Vertex> edgea = {};
	std::vector<Vertex> edgeb = {};
	for (int mesh = 0; mesh < meshes.size(); mesh++) {
		for (int i = 0; i < meshes[mesh].edges.size(); i += 2) {
			a = meshes[mesh].vertices[meshes[mesh].edges[i]];
			b = meshes[mesh].vertices[meshes[mesh].edges[i + 1]];
			a -= camera.position; b -= camera.position;
			a = rotateY(Vertex(), a, camera.rotation.x); a = rotateX(Vertex(), a, camera.rotation.y);
			if (a.z <= 0) continue;
			b = rotateY(Vertex(), b, camera.rotation.x); b = rotateX(Vertex(), b, camera.rotation.y);
			if (b.z <= 0) continue;
			a.z = w / a.z; b.z = w / b.z;
			a.x *= a.z * FOV; a.y *= a.z * FOV;
			b.x *= b.z * FOV; b.y *= b.z * FOV;
			a.x += w; a.y += h; b.x += w; b.y += h;
			if ((a.x < 0 || a.y < 0) && (b.x < 0 || b.y < 0)) continue;
			if ((a.x > SCREEN_WIDTH || a.y > SCREEN_HEIGHT) && (b.x > SCREEN_WIDTH || b.y > SCREEN_HEIGHT)) continue;
			a.z = (a.z + b.z) / 2;
			a.r = meshes[mesh].vertices[meshes[mesh].edges[i]].r;
			a.g = meshes[mesh].vertices[meshes[mesh].edges[i]].g;
			a.b = meshes[mesh].vertices[meshes[mesh].edges[i]].b;
			edgea.push_back(a);
			edgeb.push_back(b);
		}
	}
	std::vector<int> indices(edgea.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::sort(indices.begin(), indices.end(), [&](int A, int B) -> bool {
		return edgea[A].z < edgea[B].z;
		});
	for (int i = 0; i < indices.size(); i += 1) {
		SDL_SetRenderDrawColor(renderer, edgea[indices[i]].r, edgea[indices[i]].g, edgea[indices[i]].b, 255);
		SDL_RenderDrawLineF(renderer, edgea[indices[i]].x, edgea[indices[i]].y, edgeb[indices[i]].x, edgeb[indices[i]].y);
	}
}

int main(int argc, char* args[]) {
	if (!init()) {
		printf("Failed to initialize\n");
	}
	else {
		bool quit = false;
		SDL_Event e;

		Camera camera = Camera();
		camera.position.z = 1028;
		camera.position.x = 1028;
		Vertex speed = Vertex(0, 0, 0);
		Vertex gravity = Vertex(0, 0.001, 0);
		double camera_move_speed = 0.03;
		double camera_rot_speed = 0.01;
		int d = 64;
		Vertex intpos;
		std::vector<Mesh> meshes = { 
			generateTerrain(Vertex(-4, 1, -4), Vertex(), 18, d, 0,0)
			};
		for (int y = 0; y < d; y++) {
			for (int x = 0; x < d; x++) {
				int i = y * d + x;
				meshes[0].vertices[i].y = 2 * -cos((double)x/5);
				meshes[0].vertices[i].r = (int)(127 * (1+-cos((double)x / 5)));
				meshes[0].vertices[i].g = (int)(127 * (1+cos((double)x / 5)));
				meshes[0].vertices[i].b = 255;//(int)(127 * (1+-cos((double)x / 5)));
			}
		}

		double lowest = 0;
		double tick = 0;
		int prev_mouse_x, prev_mouse_y;
		double mouse_sensitivity = 0.0004;
		bool mouse_is_pressed = false;
		SDL_GetMouseState(&prev_mouse_x, &prev_mouse_y);
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
		while (!quit) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
				if (e.type == SDL_MOUSEBUTTONDOWN) { mouse_is_pressed = true; SDL_GetMouseState(&prev_mouse_x, &prev_mouse_y); }
				if (e.type == SDL_MOUSEBUTTONUP) { mouse_is_pressed = false; }
				if (e.type == SDL_MOUSEMOTION && mouse_is_pressed ) {
					int mouse_x, mouse_y;
					SDL_GetMouseState(&mouse_x, &mouse_y);
					camera.rotation += Vertex((mouse_x - prev_mouse_x) * mouse_sensitivity, (prev_mouse_y - mouse_y) * mouse_sensitivity, 0);
					prev_mouse_x = mouse_x; prev_mouse_y = mouse_y;
				}
				if (e.type == SDL_MOUSEWHEEL) {
					FOV += e.wheel.y * .1;
				}
			}
			const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
			if (currentKeyStates[SDL_SCANCODE_UP]) camera.rotation.y -= camera_rot_speed;
			if (currentKeyStates[SDL_SCANCODE_DOWN]) camera.rotation.y += camera_rot_speed;
			if (currentKeyStates[SDL_SCANCODE_LEFT]) camera.rotation.x += camera_rot_speed;
			if (currentKeyStates[SDL_SCANCODE_RIGHT]) camera.rotation.x -= camera_rot_speed;
			if (currentKeyStates[SDL_SCANCODE_W]) camera.position.x += cos((.25+camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_W]) camera.position.z += sin((.25+camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_S]) camera.position.x -= cos((.25+camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_S]) camera.position.z -= sin((.25+camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_D]) camera.position.x += cos((camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_D]) camera.position.z += sin((camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_A]) camera.position.x -= cos((camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_A]) camera.position.z -= sin((camera.rotation.x)*M_PI*2)*camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_LSHIFT]) camera_move_speed = 0.06; else camera_move_speed = 0.03;
			if (currentKeyStates[SDL_SCANCODE_LCTRL]) camera.position.y += camera_move_speed;
			if (currentKeyStates[SDL_SCANCODE_SPACE]) if (speed.y == 0) speed.y = -0.05;

			SDL_PushEvent(SDL_Event test_event);
			tick++;
			intpos.x = round(camera.position.x);
			intpos.y = round(camera.position.y);
			intpos.z = round(camera.position.z);
			for (int y = 0; y < d; y++) {
				for (int x = 0; x < d; x++) {
					int i = y * d + x;
					meshes[0].vertices[i].y = 2 * (noise((-(d/2) + intpos.x + x) * 0.1, (-(d/2) + intpos.z + y) * 0.1, 0) * 0.0001);
					meshes[0].vertices[i].x = -(d/2) + x + intpos.x;
					meshes[0].vertices[i].z = -(d/2) + y + intpos.z;
					if (meshes[0].vertices[i].y > 3) {
						meshes[0].vertices[i].r = 64;
						meshes[0].vertices[i].g = 128;
						meshes[0].vertices[i].y = 3;
					}
					else {
						meshes[0].vertices[i].r = (5 + meshes[0].vertices[i].y) * 24;
						meshes[0].vertices[i].g = -(5 + meshes[0].vertices[i].y) * 24;
					}
				}
			}
			camera.position += speed;
			//meshes[0].absolute_translate(Vertex(camera.position.x - 8, meshes[0].origin.y, camera.position.z - 8));
			if (camera.position.y < 2 * (noise(camera.position.x * 0.1, camera.position.z * 0.1, 0) * 0.0001) - .3) {
				speed += gravity;
			}
			else {
				speed.y = 0;
				camera.position.y = 2 * (noise(camera.position.x * 0.1, camera.position.z * 0.1, 0) * 0.0001) - .3;
			}
			if (camera.position.y > 2.7) camera.position.y = 2.7;

			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xAA);
			SDL_RenderFillRect(gRenderer, NULL);
			render(meshes, gRenderer, camera);
			SDL_RenderPresent(gRenderer);
		}
	}
	close();
	return 0;
}