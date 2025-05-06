#pragma once
#include <ext/imgui/imgui.h>
#include <string>
#include "sdk/Interface.h"

struct BoneConnectionByName {
	const char* parent;
	const char* child;
};

namespace raicu::cheats {
	class Visuals {
	public:
		static void Render();

		static inline const char* boneArray[] = {
			    "ValveBiped.Bip01_Pelvis",
			    "ValveBiped.Bip01_Spine",
			    "ValveBiped.Bip01_Spine1",
			    "ValveBiped.Bip01_Spine2",
			    "ValveBiped.Bip01_Spine4",
			    "ValveBiped.Bip01_Neck1",
			    "ValveBiped.Bip01_Head1",
			    "ValveBiped.forward",
			    "ValveBiped.Bip01_R_Clavicle",
			    "ValveBiped.Bip01_R_UpperArm",
			    "ValveBiped.Bip01_R_Forearm",
			    "ValveBiped.Bip01_R_Hand",
			    "ValveBiped.Anim_Attachment_RH",
			    "ValveBiped.Bip01_L_Clavicle",
			    "ValveBiped.Bip01_L_UpperArm",
			    "ValveBiped.Bip01_L_Forearm",
			    "ValveBiped.Bip01_L_Hand",
			    "ValveBiped.Anim_Attachment_LH",
			    "ValveBiped.Bip01_R_Thigh",
			    "ValveBiped.Bip01_R_Calf",
			    "ValveBiped.Bip01_R_Foot",
			    "ValveBiped.Bip01_R_Toe0",
			    "ValveBiped.Bip01_L_Thigh",
			    "ValveBiped.Bip01_L_Calf",
			    "ValveBiped.Bip01_L_Foot",
			    "ValveBiped.Bip01_L_Toe0",
			    "ValveBiped.Bip01_L_Finger4",
			    "ValveBiped.Bip01_L_Finger41",
			    "ValveBiped.Bip01_L_Finger42",
			    "ValveBiped.Bip01_L_Finger3",
			    "ValveBiped.Bip01_L_Finger31",
			    "ValveBiped.Bip01_L_Finger32",
			    "ValveBiped.Bip01_L_Finger2",
			    "ValveBiped.Bip01_L_Finger21",
			    "ValveBiped.Bip01_L_Finger22",
			    "ValveBiped.Bip01_L_Finger1",
			    "ValveBiped.Bip01_L_Finger11",
			    "ValveBiped.Bip01_L_Finger12",
			    "ValveBiped.Bip01_L_Finger0",
			    "ValveBiped.Bip01_L_Finger01",
			    "ValveBiped.Bip01_L_Finger02",
			    "ValveBiped.Bip01_R_Finger4",
			    "ValveBiped.Bip01_R_Finger41",
			    "ValveBiped.Bip01_R_Finger42",
			    "ValveBiped.Bip01_R_Finger3",
			    "ValveBiped.Bip01_R_Finger31",
			    "ValveBiped.Bip01_R_Finger32",
			    "ValveBiped.Bip01_R_Finger2",
			    "ValveBiped.Bip01_R_Finger21",
			    "ValveBiped.Bip01_R_Finger22",
			    "ValveBiped.Bip01_R_Finger1",
			    "ValveBiped.Bip01_R_Finger11",
			    "ValveBiped.Bip01_R_Finger12",
			    "ValveBiped.Bip01_R_Finger0",
			    "ValveBiped.Bip01_R_Finger01",
			    "ValveBiped.Bip01_R_Finger02",
			    "ValveBiped.Bip01_L_Elbow",
			    "ValveBiped.Bip01_L_Ulna",
			    "ValveBiped.Bip01_R_Ulna",
			    "ValveBiped.Bip01_L_Pectoral",
			    "ValveBiped.Bip01_R_Shoulder",
			    "ValveBiped.Bip01_L_Shoulder",
			    "ValveBiped.Bip01_R_Trapezius",
			    "ValveBiped.Bip01_R_Wrist",
			    "ValveBiped.Bip01_R_Bicep",
			    "ValveBiped.Bip01_L_Bicep",
			    "ValveBiped.Bip01_L_Trapezius",
			    "ValveBiped.Bip01_L_Wrist",
			    "ValveBiped.Bip01_R_Elbow"
			};

		static inline const BoneConnectionByName bone_connections_named[] = {
			// Spine
			{"ValveBiped.Bip01_Pelvis",   "ValveBiped.Bip01_Spine"},
			{"ValveBiped.Bip01_Spine",    "ValveBiped.Bip01_Spine1"},
			{"ValveBiped.Bip01_Spine1",   "ValveBiped.Bip01_Spine2"},
			{"ValveBiped.Bip01_Spine2",   "ValveBiped.Bip01_Neck1"},
			{"ValveBiped.Bip01_Neck1",    "ValveBiped.Bip01_Head1"},

			// Left Arm
			{"ValveBiped.Bip01_Neck1",   "ValveBiped.Bip01_L_Clavicle"},
			{"ValveBiped.Bip01_L_Clavicle","ValveBiped.Bip01_L_UpperArm"},
			{"ValveBiped.Bip01_L_UpperArm", "ValveBiped.Bip01_L_Forearm"},
			{"ValveBiped.Bip01_L_Forearm",  "ValveBiped.Bip01_L_Hand"},

			// Right Arm
			{"ValveBiped.Bip01_Neck1",   "ValveBiped.Bip01_R_Clavicle"},
			{"ValveBiped.Bip01_R_Clavicle","ValveBiped.Bip01_R_UpperArm"},
			{"ValveBiped.Bip01_R_UpperArm", "ValveBiped.Bip01_R_Forearm"},
			{"ValveBiped.Bip01_R_Forearm",  "ValveBiped.Bip01_R_Hand"},

			// Left Leg
			{"ValveBiped.Bip01_Pelvis",   "ValveBiped.Bip01_L_Thigh"},
			{"ValveBiped.Bip01_L_Thigh",  "ValveBiped.Bip01_L_Calf"},
			{"ValveBiped.Bip01_L_Calf",   "ValveBiped.Bip01_L_Foot"},
			{"ValveBiped.Bip01_L_Foot",   "ValveBiped.Bip01_L_Toe0"},

			// Right Leg
			{"ValveBiped.Bip01_Pelvis",   "ValveBiped.Bip01_R_Thigh"},
			{"ValveBiped.Bip01_R_Thigh",  "ValveBiped.Bip01_R_Calf"},
			{"ValveBiped.Bip01_R_Calf",   "ValveBiped.Bip01_R_Foot"},
			{"ValveBiped.Bip01_R_Foot",   "ValveBiped.Bip01_R_Toe0"},
		};

	private:
		static void DrawCrosshair(float width, float length, float offset, ImU32 color, float rounding, bool outline, ImU32 outlineColor, float outlineThickness);
		static void DrawAimbotFOV(float FOV);

		static void Drawbox(int CurrentEnt);

		static void DrawHealth(int CurrentEnt);

		static void DrawName(int CurrentEnt, float& offset);
		static void DrawDistance(int CurrentEnt, float& offset, float distance);

		static void DrawSkeleton(int CurrentEnt);
		static void DrawBacktrack(int CurrentEnt);

		static void DrawRPJob(int CurrentEnt, float& offset);
		static void DrawWeapon(int CurrentEnt, float& offset);

		static void DrawSnapline(int CurrentEnt, ImU32 color);
		static void DrawOrigin(int CurrentEnt, ImU32 color);

		static void DrawLineToTarget();
	};

	namespace Chams {
		i_material* create_material(const char* name, bool flat, bool wireframe, bool add_shine);
		void push_material_override(ImU32 color, int material_type);
		void pop_material_override();

		void push_ignore_z(bool ignore_z);
		void pop_ignore_z();
	};

}