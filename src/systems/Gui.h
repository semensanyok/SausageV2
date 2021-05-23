#pragma once

#include "sausage.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"


void InitGuiContext(SDL_Window* window, SDL_GLContext& context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	const char* glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void RenderGui(SDL_Window* window, Camera* camera) {
	static bool show_demo_window = true;
	static bool show_another_window = true;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	{
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Camera");
		ImGui::Text("POSITION: %.1f %.1f %.1f", camera->pos.x, camera->pos.y, camera->pos.z);
		ImGui::Text("Yaw: %.1f Pitch: %.1f", camera->yaw_angle, camera->pitch_angle);
		ImGui::Text("Right: %.1f %.1f %.1f", camera->right.x, camera->right.y, camera->right.z);
		ImGui::Text("Up: %.1f %.1f %.1f", camera->up.x, camera->up.y, camera->up.z);
		ImGui::Text("direction: %.1f %.1f %.1f", camera->direction.x, camera->direction.y, camera->direction.z);
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CleanupGui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}