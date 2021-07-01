#pragma once

#include "sausage.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "Settings.h"

using namespace std;

class Gui {
public:
	static void InitGuiContext(SDL_Window* window, SDL_GLContext& context) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForOpenGL(window, context);
		const char* glsl_version = "#version 460";
		ImGui_ImplOpenGL3_Init(glsl_version);
	}

	static void RenderGui(SDL_Window* window, Camera* camera) {
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
		{
			ImGui::SetNextWindowPos(ImVec2(0, 20));
			ImGui::Begin("Physics");
			ImGui::InputFloat("physics step multiplier", &GameSettings::physics_step_multiplier, 0.0001f, 1.0f, "%.4f");
			ImGui::InputInt("ray_debug_draw_lifetime_milliseconds", &GameSettings::ray_debug_draw_lifetime_milliseconds, 500);

			ImGui::End();
		}
#ifdef SAUSAGE_PROFILE_ENABLE
		ImGui::Text("prepare draws ms: %i", std::chrono::duration_cast<std::chrono::milliseconds>(ProfTime::prepare_draws_ns));
		ImGui::Text("render ms: %i", std::chrono::duration_cast<std::chrono::milliseconds>(ProfTime::render_ns));
		ImGui::Text("simulate physics draws ms: %i", std::chrono::duration_cast<std::chrono::milliseconds>(ProfTime::physics_sym_ns));
		ImGui::Text("update physics transforms ms: %i", std::chrono::duration_cast<std::chrono::milliseconds>(ProfTime::physics_buf_trans_ns));
		ImGui::Text("total ms: %i", std::chrono::duration_cast<std::chrono::milliseconds>(ProfTime::total_frame_ns));

#endif

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	static void CleanupGui() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}
};