#include "Chams.h"
#include "globals/settings.h"
#include "logging/logger.h"
#include <SDK/Interface.h>

void ImU32ToFloat4(ImU32 color, float out[4]) {
	out[0] = ((color >> 0) & 0xFF) / 255.0f; // R
	out[1] = ((color >> 8) & 0xFF) / 255.0f; // G
	out[2] = ((color >> 16) & 0xFF) / 255.0f; // B
	out[3] = ((color >> 24) & 0xFF) / 255.0f; // A
}

namespace raicu::cheats::Chams {
	i_material *create_material(const char *name, bool flat, bool wireframe, bool add_shine) {
		// Create material directly with shader type
		const char *shader_type = (flat || wireframe) ? "UnlitGeneric" : "VertexLitGeneric";
		i_material *material = interfaces::material_system->create_material(name, nullptr);

		if (!material)
			return nullptr;

		// Set basic flags
		material->set_material_var_flag(material_var_ignorez, false);
		material->set_material_var_flag(material_var_wireframe, wireframe);
		material->set_material_var_flag(material_var_flat, false);
		material->set_material_var_flag(material_var_model, true);

		if (flat) {
			material->set_material_var_flag(material_var_flat, true);
		}

		if (add_shine) {
			material->set_material_var_flag(material_var_envmapsphere, true);
			material->set_material_var_flag(material_var_envmapmode, true);
		}

		material->increment_reference_count();
		return material;
	}

	void push_material_override(ImU32 color, int material_type) {
		static auto material_system = interfaces::material_system;
		if (!material_system)
			return;

		// Cache but allow reloading if material is invalid
		static i_material *flat = nullptr;
		static i_material *wireframe = nullptr;
		static i_material *metal = nullptr;

		if (!flat) {
			flat = material_system->find_material("debug/debugdrawflat", "Other textures");
			flat->increment_reference_count();
		}
		if (!wireframe) {
			wireframe = material_system->find_material("models/wireframe", "Model textures");
			wireframe->increment_reference_count();
		}
		if (!metal) {
			metal = material_system->find_material("models/props_pipes/GutterMetal01a", "Model textures");
			metal->increment_reference_count();
		}

		i_material *material = nullptr;
		switch (material_type) {
			case 1: material = metal;
				break;
			case 2: material = wireframe;
				break;
			case 3: material = flat;
				break;
			default: return;
		}

		if (!material)
			return;

		// Lighting and color setup
		interfaces::model_render->suppress_engine_lighting(true);
		interfaces::model_render->setup_lighting(c_vector());

		float col[4];
		ImU32ToFloat4(color, col);

		if (material_type > 0) {
			interfaces::render_view->set_color_modulation(col);
			interfaces::render_view->set_blend(col[3]);
		}

		// Reset all flags
		material->set_material_var_flag(material_var_ignorez, false);
		material->set_material_var_flag(material_var_wireframe, false);
		material->set_material_var_flag(material_var_flat, false);
		material->set_material_var_flag(material_var_selfillum, false);

		// Set specific flags
		if (material_type == 1) {
			material->set_material_var_flag(material_var_selfillum, true); // Metal
		} else if (material_type == 2) {
			material->set_material_var_flag(material_var_wireframe, true); // Wireframe
		} else if (material_type == 3) {
			material->set_material_var_flag(material_var_flat, true); // Flat
		}

		interfaces::model_render->forced_material_override(material);
	}

	void pop_material_override() {
		static float clear_color[3] = {
			1.f, 1.f, 1.f
		};

		interfaces::model_render->suppress_engine_lighting(false);

		interfaces::render_view->set_color_modulation(clear_color);
		interfaces::render_view->set_blend(1.f);

		interfaces::model_render->forced_material_override(nullptr);
	}

	void push_ignore_z(bool ignore_z) {
		c_mat_render_context *render_context = interfaces::material_system->get_render_context();
		if (!render_context) return;

		if (ignore_z)
			render_context->depth_range(0.f, 0.f);
	}

	void pop_ignore_z() {
		c_mat_render_context *render_context = interfaces::material_system->get_render_context();
		if (!render_context)
			return;

		render_context->depth_range(0.f, 1.f);
	}
}
