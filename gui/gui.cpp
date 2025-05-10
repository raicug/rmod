#include "../public/d3d9_includes.h"
#include "wininet.h"

#include "gui.h"
#include "ext/imgui/imgui.h"
#include "ext/imgui/backends/imgui_impl_dx9.h"
#include "ext/imgui/backends/imgui_impl_win32.h"
#include "ext/imgui/imgui_internal.h"

#include "windows.h"
#include "stdexcept"

#include "logging/logger.h"
#include <globals/settings.h>

#include "themes.h"

#include "encoding/sha256.h"
#include "encoding/randstr.h"

#include <wininet.h>

#include <nlohmann/json.h>
#include <ext/imgui/custom/imgui_notify.h>

#include <vector>
#include <string>
#include <globals/configManager.h>
#include <ext/imgui/custom/TextEditor.h>
#include <mutex>
#include <cheats/Drawing/Drawing.h>
#include <cheats/Visuals.h>
#include <fonts/Inter.h>
#include <globals/offsets.h>
#include <cheats/lua/lua.h>

#include "cheats/aimbot/backtrack/history.h"
#include "helpers/hotkey.h"

#pragma comment(lib, "wininet.lib")

const float MAX_SIZE_FLOAT{50.f};
const float MIN_SIZE_FLOAT{1.f};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

inline std::wstring AnsiToWide(const char *str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstrTo[0], size_needed);
	return wstrTo;
}

static const ImGuiColorEditFlags colorPickerFlags =
		ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview;

LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParam, LPARAM longParam);

bool raicu::gui::WriteToRegistry(const std::wstring &valueName, const std::wstring &value) {
	const std::wstring keyPath = L"Environment";

	HKEY hKey;
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, AnsiToWide(reinterpret_cast<const char *>(keyPath.c_str())).c_str(),
	                           0, KEY_WRITE, &hKey);
	if (result != ERROR_SUCCESS) {
		logger::Log(3, "Failed to open registry key");
		return false;
	}

	result = RegSetValueEx(hKey, AnsiToWide(reinterpret_cast<const char *>(valueName.c_str())).c_str(), 0, REG_SZ,
	                       reinterpret_cast<const BYTE *>(value.c_str()), (value.size() + 1) * sizeof(wchar_t));
	if (result != ERROR_SUCCESS) {
		logger::Log(3, "Failed to write to registry key");
		RegCloseKey(hKey);
		return false;
	}

	RegCloseKey(hKey);
	return true;
}

bool raicu::gui::ReadFromRegistry(const std::wstring &valueName, std::wstring &value) {
	const std::wstring keyPath = L"Environment";

	HKEY hKey;
	LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		logger::Log(3, "Failed to open registry key");
		return false;
	}

	DWORD bufferSize = 0;
	result = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, nullptr, &bufferSize);
	if (result != ERROR_SUCCESS) {
		logger::Log(3, "Failed to query registry value size");
		RegCloseKey(hKey);
		return false;
	}

	std::vector<wchar_t> buffer(bufferSize / sizeof(wchar_t));
	result = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, reinterpret_cast<BYTE *>(buffer.data()),
	                          &bufferSize);
	if (result != ERROR_SUCCESS) {
		logger::Log(3, "Failed to read registry value");
		RegCloseKey(hKey);
		return false;
	}

	value.assign(buffer.data());
	RegCloseKey(hKey);
	return true;
}


size_t raicu::gui::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	((std::string *) userp)->append((char *) contents, size * nmemb);
	return size * nmemb;
}

bool raicu::gui::loginRequest(const std::string &email, const std::string &password, std::string &responseBody) {
	nlohmann::json data;
	data["email"] = email;
	data["password"] = password;
	std::string jsonString = data.dump();

	// Open a session handle
	HINTERNET hInternet = InternetOpen(LPCWSTR("WinINet HTTP Client"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		std::cerr << "Failed to initialize WinINet session." << std::endl;
		return false;
	}

	HINTERNET hConnect = InternetOpenUrlA(hInternet, "http://localhost:5000/api/login", NULL, 0, INTERNET_FLAG_RELOAD,
	                                      0);
	if (!hConnect) {
		std::cerr << "Failed to open connection to the server." << std::endl;
		InternetCloseHandle(hInternet);
		return false;
	}

	const char *contentType = "Content-Type: application/json\r\n";

	if (!HttpSendRequestA(hConnect, contentType, strlen(contentType), (LPVOID) jsonString.c_str(),
	                      jsonString.length())) {
		std::cerr << "Failed to send request." << std::endl;
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return false;
	}

	char buffer[1024];
	DWORD bytesRead = 0;
	responseBody.clear();

	while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
		buffer[bytesRead] = '\0';
		responseBody.append(buffer);
	}

	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return true;
}

struct ConsoleLogEntry {
	std::string text;
	int level;
};

struct ConsoleLog {
	std::vector<ConsoleLogEntry> entries;

	void Clear() {
		entries.clear();
	}

	void AddLog(int level, const std::string &message) {
		entries.push_back({message, level});
	}

	ImVec4 GetColorForLevel(int level) {
		using namespace raicu::globals::settings::consoleLogColours;
		switch (level) {
			case 1: return infoColor;
			case 2: return warningColor;
			case 3: return errorColor;
			case 4: return fatalColor;
			case 5: return successColor;
			default: return unknownColor;
		}
	}

	void Draw(const char *title) {
		if (!raicu::globals::settings::consoleOpen) return;

		if (!ImGui::Begin(title, nullptr)) {
			ImGui::End();
			return;
		}

		bool clear = ImGui::Button("Clear");
		ImGui::Separator();

		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
			if (clear) Clear();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGuiListClipper clipper;
			clipper.Begin(entries.size());

			while (clipper.Step()) {
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
					const auto &entry = entries[i];

					ImVec4 color = GetColorForLevel(entry.level);

					ImGui::TextColored(color, "%s", entry.text.c_str());
				}
			}
			clipper.End();
			ImGui::PopStyleVar();

			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}
};

ConsoleLog gConsoleLog;

bool raicu::gui::SetupWindowClass(const char *windowClassName) noexcept {
	logger::Log(logger::LOGGER_LEVEL_INFO, "Creating window class");

	static std::wstring wideClassName = AnsiToWide(windowClassName);

	windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_CLASSDC;
	windowClass.lpfnWndProc = DefWindowProcW;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleW(nullptr);
	windowClass.hIcon = nullptr;
	windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = wideClassName.c_str();
	windowClass.hIconSm = nullptr;

	WNDCLASSEXW wc;
	if (GetClassInfoExW(windowClass.hInstance, wideClassName.c_str(), &wc)) {
		UnregisterClassW(wideClassName.c_str(), windowClass.hInstance);
	}

	if (!RegisterClassExW(&windowClass)) {
		DWORD error = GetLastError();
		logger::Log(logger::LOGGER_LEVEL_FATAL, "Failed to create window class.");
		return false;
	}

	logger::Log(logger::LOGGER_LEVEL_SUCCESS, "Created window class");
	return true;
}

void raicu::gui::DestroyWindowClass() noexcept {
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool raicu::gui::SetupWindow(const char *windowName) noexcept {
	logger::Log(logger::LOGGER_LEVEL_INFO, "Creating window class");

	static wchar_t className[256];
	// Convert windowName to wide string once
	std::wstring wideWindowName = AnsiToWide(windowName);
	wcscpy_s(className, wideWindowName.c_str());

	// Zero initialize the structure
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_CLASSDC;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1); // More reliable than GetStockObject
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = className; // Now using wide string directly
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// Unregister any existing class
	UnregisterClassW(className, windowClass.hInstance);

	if (!RegisterClassExW(&windowClass)) {
		DWORD error = GetLastError();
		logger::Log(logger::LOGGER_LEVEL_FATAL,
		            ("Failed to register window class. Error: " + std::to_string(error)).c_str());
		return false;
	}

	logger::Log(logger::LOGGER_LEVEL_INFO, "Creating window");

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowWidth = 800;
	int windowHeight = 600;
	int posX = (screenWidth - windowWidth) / 2;
	int posY = (screenHeight - windowHeight) / 2;

	window = CreateWindowExW(
		0,
		className, // Using stored wide string
		wideWindowName.c_str(),
		WS_OVERLAPPEDWINDOW,
		posX, posY,
		windowWidth, windowHeight,
		NULL,
		NULL,
		windowClass.hInstance,
		NULL
	);

	if (!window) {
		DWORD error = GetLastError();
		logger::Log(logger::LOGGER_LEVEL_FATAL, ("Failed to create window. Error: " + std::to_string(error)).c_str());
		UnregisterClassW(className, windowClass.hInstance);
		return false;
	}

	ShowWindow(window, SW_SHOW);
	UpdateWindow(window);

	logger::Log(logger::LOGGER_LEVEL_SUCCESS, "Created window");
	return true;
}

void raicu::gui::DestroyWindow() noexcept {
	if (window) DestroyWindow(window);
}

bool raicu::gui::SetupDirectX() noexcept {
	logger::Log(1, "Creating DirectX");

	// Try Direct3D9Ex first
	using Direct3DCreate9ExFn = HRESULT(WINAPI*)(UINT, IDirect3D9Ex **);
	HMODULE d3d9Module = GetModuleHandleA("d3d9.dll");

	if (!d3d9Module) {
		logger::Log(3, "Failed to get d3d9.dll module");
		return false;
	}

	Direct3DCreate9ExFn createEx = reinterpret_cast<Direct3DCreate9ExFn>(
        GetProcAddress(d3d9Module, "Direct3DCreate9Ex"));

	IDirect3D9Ex *d3d9ex = nullptr;
	if (createEx && SUCCEEDED(createEx(D3D_SDK_VERSION, &d3d9ex))) {
		d3d9 = d3d9ex; // Store in base interface pointer
	} else {
		// Fallback to regular D3D9
		using Direct3DCreate9Fn = IDirect3D9*(WINAPI*)(UINT);
		auto create9 = reinterpret_cast<Direct3DCreate9Fn>(
			GetProcAddress(d3d9Module, "Direct3DCreate9"));

		if (!create9) {
			logger::Log(3, "Failed to get Direct3DCreate9 function");
			return false;
		}

		d3d9 = create9(D3D_SDK_VERSION);
	}

	if (!d3d9) {
		logger::Log(3, "Failed to create D3D9 interface");
		return false;
	}

	// Simplified presentation parameters
	D3DPRESENT_PARAMETERS params = {0};
	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = window;
	params.BackBufferFormat = D3DFMT_UNKNOWN; // Let D3D choose based on desktop
	params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	params.BackBufferCount = 1;

	// Try to create device with various settings
	const D3DDEVTYPE devTypes[] = {D3DDEVTYPE_HAL, D3DDEVTYPE_REF};
	const DWORD behaviors[] = {
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES,
		D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES
	};

	HRESULT hr = E_FAIL;
	for (auto devType: devTypes) {
		for (auto behavior: behaviors) {
			hr = d3d9->CreateDevice(
				D3DADAPTER_DEFAULT,
				devType,
				window,
				behavior,
				&params,
				&device
			);

			if (SUCCEEDED(hr)) {
				logger::Log(5, "Created DirectX device");
				return true;
			}
		}
	}

	// If we get here, all creation attempts failed
	logger::Log(3, "Failed to create DirectX device.");
	return false;
}

void raicu::gui::DestroyDirectX() noexcept {
	if (device) {
		device->Release();
		device = nullptr;
	}

	if (d3d9) {
		d3d9->Release();
		d3d9 = nullptr;
	}
}

void raicu::gui::Setup() {
	logger::Log(1, "Doing setup");

	if (!SetupWindowClass("raicuwindowclass001"))
		throw std::runtime_error("Failed to create window class");
	if (!SetupWindow("raicuwindow001"))
		throw std::runtime_error("Failed to create window");
	if (!SetupDirectX())
		throw std::runtime_error("Failed to create device");

	DestroyWindow();
	DestroyWindowClass();
}

void raicu::gui::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept {
	if (imguiInitialized) {
		return;
	}

	D3DDEVICE_CREATION_PARAMETERS parms;
	if (FAILED(device->GetCreationParameters(&parms))) {
		return;
	}
	window = parms.hFocusWindow;

	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtrA(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
	);

	if (!originalWindowProcess) {
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO &io = ImGui::GetIO();

	logger::Log(logger::LOGGER_LEVEL_INFO, "Setting up fonts");

	io.Fonts->AddFontDefault();

	ImFontConfig cfg;
	cfg.FontDataOwnedByAtlas = false;
	if (ImFont *newDefault = io.Fonts->AddFontFromMemoryTTF(
		(void *) inter,
		inter_len,
		16.0f,
		&cfg
	)) {
		io.FontDefault = newDefault;
		logger::Log(logger::LOGGER_LEVEL_INFO, "Custom font loaded successfully");
	} else {
		logger::Log(logger::LOGGER_LEVEL_WARNING, "Failed to load custom font, using default");
	}

	if (!ImGui_ImplWin32_Init(window)) {
		logger::Log(logger::LOGGER_LEVEL_ERROR, "Failed to initialize ImGui Win32 backend");
		return;
	}

	if (!ImGui_ImplDX9_Init(device)) {
		logger::Log(logger::LOGGER_LEVEL_ERROR, "Failed to initialize ImGui D3D9 backend");
		ImGui_ImplWin32_Shutdown();
		return;
	}

	imguiInitialized = true;
	setup = true;
	logger::Log(logger::LOGGER_LEVEL_SUCCESS, "Menu setup completed successfully");
}

void raicu::gui::Destroy() noexcept {
	if (imguiInitialized) {
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		imguiInitialized = false;
	}

	if (window && originalWindowProcess)
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWindowProcess));


	DestroyDirectX();
}

void ShowLog() {
	ImGui::SetNextWindowSize(ImVec2(425, 300), ImGuiCond_Once);

	if (raicu::globals::settings::consoleOpen) {
		ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoResize);
		ImGui::End();
	}

	gConsoleLog.Draw("Console");
}

bool g_needsLogin = true;

void ShowLogin() {
	std::wstring regKey;
	const char *n = "9dz0jeheCBp7E3WWnI0h";
	std::wstring n2(n, n + strlen(n));

	if (raicu::gui::ReadFromRegistry(n2, regKey)) {
		logger::Log(1, "This PC is authed already");
		g_needsLogin = false;
	}

	static char email[128] = "";
	static char password[128] = "";

	ImGui::SetNextWindowSize(ImVec2(300, 200));

	ImGui::Begin("Raicu GMOD - Login", nullptr, ImGuiWindowFlags_NoResize);

	ImGui::InputText("Email", email, IM_ARRAYSIZE(email));
	ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

	if (ImGui::Button("Login")) {
		std::string responseBody;

		try {
			// USES LOCALHOST FOR NOW
			bool loginSuccess = raicu::gui::loginRequest(email, password, responseBody);

			if (loginSuccess) {
				logger::Log(5, "successfully logged in");
				g_needsLogin = false;

				std::string s0 = randstr::generate(20);

				std::string s = std::string(email) + password + s0;

				SHA256 sha;

				sha.update(s);

				std::array<uint8_t, 32> dig = sha.digest();

				std::string s2 = SHA256::toString(dig);

				std::wstring a(s2.c_str(), s2.c_str() + strlen(s2.c_str()));

				logger::Log(1, s2.c_str());

				raicu::gui::WriteToRegistry(n2, a);
			}
		} catch (const std::exception &error) {
			logger::Log(4, "Failed to connect to API!");
		}
	}

	ImGui::End();
}

std::vector<std::string> configs;
static char selectedConfig[256] = "";
static char newConfigName[256] = "";

const char *executorLuaState[]{
	"Client",
	"Server",
	"Menu",
};

const char *materialList[]{
	"Normal",
	"Metal",
	"Wireframe",
	"Flag"
};

const char *snaplinePosition[]{
	"Bottom",
	"Middle",
	"Top"
};

const char *aimbot_hitboxes[]{
	"Head",
	"Chest",
	"Stomach",
	"Hitscan"
};

const char *aimbot_priorities[]{
	"Fov",
	"Distance",
	"Health"
};

static bool editorInited = false;
static TextEditor editor;
static bool configLoading = false;

const char *sidebarTabs[] = {"Visuals", "Appearance", "Lua", "Config", "World", "Game"};
const char *topTabsVisuals[] = {"FOV", "Crosshair", "ESP", "Chams"};
const char *topTabsAppearance[] = {"Main", "Console Colours"};
const char *topTabsLua[] = {"Main"};
const char *topTabsWorld[] = {"Aimbot", "Movement", "Players"};
const char *topTabsConfig[] = {"Loading", "Saving"};
const char *topTabsGame[] = {"Loading Screen"};

const char **topTabsArray[] = {topTabsVisuals, topTabsAppearance, topTabsLua, topTabsConfig, topTabsWorld, topTabsGame};
int topTabSizes[] = {
	IM_ARRAYSIZE(topTabsVisuals),
	IM_ARRAYSIZE(topTabsAppearance),
	IM_ARRAYSIZE(topTabsLua),
	IM_ARRAYSIZE(topTabsConfig),
	IM_ARRAYSIZE(topTabsWorld),
	IM_ARRAYSIZE(topTabsGame)
};

void ShowPlayers() {
    if (raicu::globals::settings::other::playerList) {
        if (!interfaces::engine->is_in_game() || !interfaces::engine->is_connected()) {
            return;
        }
    	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing |
                                        ImGuiWindowFlags_NoNav |
                                      ImGuiWindowFlags_NoMove;

        if (ImGui::Begin("Players", nullptr, window_flags)) {
            // Get the available content region width to match the window width
            float windowWidth = ImGui::GetContentRegionAvail().x;

            const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
            const float ROW_HEIGHT = TEXT_BASE_HEIGHT + 4;
            const float TABLE_HEIGHT = ROW_HEIGHT * 6; // 5 rows + header

            if (ImGui::BeginTable("tPlayers", 3,
                                  ImGuiTableFlags_Borders |
                                  ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_ScrollY,
                                  ImVec2(windowWidth, TABLE_HEIGHT))) {
	            ImGui::TableSetupScrollFreeze(0, 1);

	            // Adjust column widths to fill the window
	            float nameWidth = windowWidth * 0.4f; // 40% of width
	            float healthWidth = windowWidth * 0.2f; // 20% of width
	            float posWidth = windowWidth * 0.4f; // 40% of width

	            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, nameWidth);
	            ImGui::TableSetupColumn("Health", ImGuiTableColumnFlags_WidthFixed, healthWidth);
	            ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, posWidth);
	            ImGui::TableHeadersRow();

	            for (int i = 0; i <= interfaces::engine->get_max_clients(); i++) {
		            c_base_entity *currentEntity = interfaces::entity_list->get_entity(i);

		            if (!currentEntity) {
			            continue;
		            }

		            if (!currentEntity->is_player()) {
			            continue;
		            }

		            player_info_t pinfo;
		            if (!interfaces::engine->get_player_info(i, &pinfo)) {
			            continue;
		            }

		            c_vector pos = currentEntity->get_abs_origin();

		            ImGui::TableNextRow();
		            ImGui::TableNextColumn();
		            ImGui::Text("%s", pinfo.name);
		            ImGui::TableNextColumn();
		            ImGui::Text("%d", currentEntity->get_health());
		            ImGui::TableNextColumn();
		            ImGui::Text("%.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
	            }
	            ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}

void ShowSpectators() {
	// retarded way of doing this, idc
	if (!raicu::globals::settings::other::spectatorList) return;

	ImGui::SetNextWindowSize(ImVec2(200.f, 200.f));
	ImGui::Begin("Spectators window", nullptr,
	             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
	             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar); {
		std::string names = "";
		for (int i = 0; i < interfaces::engine->get_max_clients(); i++) {
			c_base_entity *currentEntity = interfaces::entity_list->get_entity(i);
			if (currentEntity == nullptr || !currentEntity->is_player() || currentEntity == interfaces::entity_list->
			    get_entity(interfaces::engine->get_local_player()))
				continue;

			if (currentEntity->GetObserverTarget() == interfaces::entity_list->get_entity(
				    interfaces::engine->get_local_player()))
				continue;

			player_info_s pinfo;
			interfaces::engine->get_player_info(i, &pinfo);

			names += std::to_string(*pinfo.name) + "\n";
		}
		ImGui::GetStyle().ItemSpacing = ImVec2(4, 2);
		ImGui::GetStyle().WindowPadding = ImVec2(4, 4);
		ImGui::SameLine(15.f);
		ImGui::Text(names.c_str());
	}

	ImGui::GetStyle().ItemSpacing = ImVec2(8, 4);
	ImGui::GetStyle().WindowPadding = ImVec2(8, 8);

	ImGui::End();
}

void raicu::gui::Render() noexcept {
	IDirect3DStateBlock9 *stateBlock = nullptr;

	if (SUCCEEDED(device->CreateStateBlock(D3DSBT_ALL, &stateBlock)))
		stateBlock->Capture();

	c_mat_render_context *ctx = interfaces::material_system->get_render_context(); // bypass screengrabs
	if (ctx) {
		ctx->begin_render();
		rt = ctx->get_render_target();
		ctx->set_render_target(nullptr);
		ctx->end_render();
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	raicu::gui::themes::setFluentUITheme();

	if (g_needsLogin) {
		ShowLogin();
	} else {
		ShowLog();

		raicu::cheats::Visuals::Render();
		ShowPlayers();
		ShowSpectators();

		if (!editorInited) {
			editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
			editor.SetText(raicu::globals::settings::lua::ScriptInput);
			editorInited = true;
		}

		if (configLoading) {
			editor.SetText(raicu::globals::settings::lua::ScriptInput);
			configLoading = false;
		}

		if (raicu::globals::settings::open) {
			ImGui::SetNextWindowSize(ImVec2(650, 450));

			ImGui::Begin("RMOD", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse); {
				static int selectedSidebar = 0;
				static int selectedTopTabs[IM_ARRAYSIZE(sidebarTabs)] = {0};

				ImGui::BeginChild("Sidebar##mainui", ImVec2(150, 0), true); {
					for (int i = 0; i < IM_ARRAYSIZE(sidebarTabs); i++) {
						if (ImGui::Selectable(sidebarTabs[i], selectedSidebar == i)) {
							selectedSidebar = i;
						}
					}
					ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetTextLineHeightWithSpacing() - 5);
					ImGui::Text(raicu::globals::settings::version.c_str());
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginGroup(); {
					const char **currentTopTabs = topTabsArray[selectedSidebar];
					int currentTopTabCount = topTabSizes[selectedSidebar];
					int &currentTopTab = selectedTopTabs[selectedSidebar];

					ImGui::BeginChild("Topbar", ImVec2(0, 40), true); {
						for (int i = 0; i < currentTopTabCount; i++) {
							if (i > 0) ImGui::SameLine(); // Add space between buttons
							if (ImGui::Button(currentTopTabs[i])) {
								currentTopTab = i;
							}
						}
					}
					ImGui::EndChild();

					const char *currentTab = sidebarTabs[selectedSidebar];
					const char *current2Tab = currentTopTabs[currentTopTab];

					ImGui::Separator();

					ImGui::BeginChild("VisualsSection", ImVec2(0, 360), true); {
						// ImGui::Text("Welcome to %s | %s", currentTab, current2Tab); // DEBUGGING
						ImGui::PushItemWidth(350);

						/* --- VISUALS --- */
						if (strcmp(currentTab, "Visuals") == 0 && strcmp(current2Tab, "FOV") == 0) {
							ImGui::PushID("FOV");

							ImGui::Checkbox("Enabled", &raicu::globals::settings::other::enableFov);
							ImGui::SliderFloat("Size", &raicu::globals::settings::other::fovSize, 0.f, 180.f, "%.1f");
							ImGui::ColorEdit4(
								"Color", reinterpret_cast<float *>(&raicu::globals::settings::other::fovColor));

							ImGui::PopID();
						}
						if (strcmp(currentTab, "Visuals") == 0 && strcmp(current2Tab, "Crosshair") == 0) {
							ImGui::PushID("Crosshair");

							ImGui::Checkbox("Enabled", &raicu::globals::settings::crosshairValues::enabled);
							ImGui::Checkbox("Outline Enabled",
							                &raicu::globals::settings::crosshairValues::outlineEnabled);

							ImGui::SliderFloat("Outline Thickness",
							                   &raicu::globals::settings::crosshairValues::outlineThickness, 0.0f,
							                   10.0f,
							                   "%.2f");
							ImGui::SliderFloat("Rounding", &raicu::globals::settings::crosshairValues::rounding, 0.0f,
							                   10.0f, "%.2f");

							ImGui::SliderFloat("Width", &raicu::globals::settings::crosshairValues::width, 0.1f, 100.0f,
							                   "%.1f");
							ImGui::SliderFloat("Height", &raicu::globals::settings::crosshairValues::height, 0.1f,
							                   100.0f,
							                   "%.1f");
							ImGui::SliderFloat("Offset", &raicu::globals::settings::crosshairValues::offset, -100.0f,
							                   100.0f, "%.1f");

							ImGui::ColorEdit4(
								"Color", reinterpret_cast<float *>(&raicu::globals::settings::crosshairValues::color),
								ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4("Outline Color",
							                  reinterpret_cast<float *>(&
								                  raicu::globals::settings::crosshairValues::outlineColor),
							                  ImGuiColorEditFlags_NoAlpha);

							ImGui::PopID();
						}
						if (strcmp(currentTab, "Visuals") == 0 && strcmp(current2Tab, "ESP") == 0) {
							ImGui::PushID("ESP");

							ImGui::Checkbox("Enabled", &raicu::globals::settings::espValues::enabled);
							ImGui::Columns(2, "GeneralESPGrid", false);

							ImGui::Checkbox("Health", &raicu::globals::settings::espValues::health);
							ImGui::Checkbox("Distance", &raicu::globals::settings::espValues::distance);
							ImGui::Checkbox("Box", &raicu::globals::settings::espValues::box);
							ImGui::Checkbox("Weapon", &raicu::globals::settings::espValues::weapon);
							ImGui::NextColumn();
							ImGui::Checkbox("Origin", &raicu::globals::settings::espValues::origin);
							ImGui::Checkbox("Name", &raicu::globals::settings::espValues::name);
							ImGui::Checkbox("Snapline", &raicu::globals::settings::espValues::snapline);
							ImGui::Checkbox("Skeleton", &globals::settings::espValues::skeleton);
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("PRONE TO CRASHING ON LARGE SERVERS");
							}

							ImGui::Columns(1);

							ImGui::Combo("Snapline Position", &raicu::globals::settings::espValues::snaplinePosition,
							             snaplinePosition, IM_ARRAYSIZE(snaplinePosition));
							ImGui::SliderInt("Render Distance", &raicu::globals::settings::espValues::render_distance,
							                 1000,
							                 30000);

							ImGui::ColorEdit4(
								"Snapline",
								reinterpret_cast<float *>(&raicu::globals::settings::espValues::snapLineColor),
								ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4(
								"Origin", reinterpret_cast<float *>(&raicu::globals::settings::espValues::originColor),
								ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4(
								"Name", reinterpret_cast<float *>(&raicu::globals::settings::espValues::nameColor),
								ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4(
								"Box", reinterpret_cast<float *>(&raicu::globals::settings::espValues::boxColor),
								ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4(
								"Skeleton",
								reinterpret_cast<float *>(&raicu::globals::settings::espValues::skeletonColor),
								ImGuiColorEditFlags_NoAlpha);

							ImGui::PopID();
						}

						/* --- APPEARANCE --- */
						if (strcmp(currentTab, "Appearance") == 0 && strcmp(current2Tab, "Main") == 0) {
							ImGui::Checkbox("Enable console", &raicu::globals::settings::consoleOpen);
							ImGui::Checkbox("Enable logging notifications",
							                &raicu::globals::settings::loggerNotifications);
						}
						if (strcmp(currentTab, "Appearance") == 0 && strcmp(current2Tab, "Console Colours") == 0) {
							ImGui::ColorEdit4("INFO Color",
							                  reinterpret_cast<float *>(&
								                  raicu::globals::settings::consoleLogColours::infoColor),
							                  ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4("WARNING Color",
							                  reinterpret_cast<float *>(&
								                  raicu::globals::settings::consoleLogColours::warningColor),
							                  ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4("ERROR Color",
							                  reinterpret_cast<float *>(&
								                  raicu::globals::settings::consoleLogColours::errorColor),
							                  ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4("FATAL Color",
							                  reinterpret_cast<float *>(&
								                  raicu::globals::settings::consoleLogColours::fatalColor),
							                  ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4("SUCCESS Color",
							                  reinterpret_cast<float *>(&
								                  raicu::globals::settings::consoleLogColours::successColor),
							                  ImGuiColorEditFlags_NoAlpha);
							ImGui::ColorEdit4("UNKNOWN Color",
							                  reinterpret_cast<float *>(&
								                  raicu::globals::settings::consoleLogColours::unknownColor),
							                  ImGuiColorEditFlags_NoAlpha);
						}

						/* --- LUA --- */
						if (strcmp(currentTab, "Lua") == 0 && strcmp(current2Tab, "Main") == 0) {
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

							editor.Render("##Source", ImVec2(460, 310)); {
								raicu::globals::settings::lua::ScriptInput = editor.GetText();
							}

							bool executePressed = false;

							float inputWidth = 460.0f;
							float buttonWidth = inputWidth * 0.45f;
							float comboWidth = inputWidth * 0.85f;

							ImGui::PushItemWidth(buttonWidth);
							if (ImGui::Button("Execute")) executePressed = true;
							ImGui::SameLine();

							ImGui::PushItemWidth(comboWidth);
							ImGui::Combo("##State", &raicu::globals::settings::lua::executeState, executorLuaState,
							             IM_ARRAYSIZE(executorLuaState));

							if (executePressed) {
								std::lock_guard<std::mutex> lock(lua::executionData.mutex);
								lua::executionData.script = globals::settings::lua::ScriptInput;
								lua::executionData.waiting.store(true);
								ImGui::InsertNotification({
									ImGuiToastType::Success, 3000, "Executed script successfully!"
								});
							}

							ImGui::PopItemWidth();
							ImGui::PopItemWidth();

							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);

							ImGui::PopStyleVar();
						}

						/* --- CONFIG --- */
						if (strcmp(currentTab, "Config") == 0 && strcmp(current2Tab, "Loading") == 0) {
							ImGui::PushID("Config - Loading");

							configs = ConfigManager::GetAllConfigs();

							if (!configs.empty()) {
								if (ImGui::BeginCombo("Select Config",
								                      selectedConfig[0] ? selectedConfig : "Select...")) {
									for (const auto &config: configs) {
										bool isSelected = (strcmp(selectedConfig, config.c_str()) == 0);
										if (ImGui::Selectable(config.c_str(), isSelected)) {
											strcpy_s(selectedConfig, config.c_str());
										}
										if (isSelected) {
											ImGui::SetItemDefaultFocus();
										}
									}
									ImGui::EndCombo();
								}

								if (ImGui::Button("Load Config")) {
									if (selectedConfig[0] != '\0') {
										configLoading = true;
										ConfigManager::Load(selectedConfig);
									}
								}
							} else {
								ImGui::TextDisabled("You have no configs! Go onto the \"Saving\" tab first!");
							}

							ImGui::PopID();
						}
						if (strcmp(currentTab, "Config") == 0 && strcmp(current2Tab, "Saving") == 0) {
							ImGui::PushID("Config - Saving");
							ImGui::InputText("Config name", newConfigName, IM_ARRAYSIZE(newConfigName));

							if (ImGui::Button("Save Config")) {
								if (newConfigName[0] != '\0') {
									ConfigManager::Save(std::string(newConfigName) + ".json");
									configs = ConfigManager::GetAllConfigs();
									strcpy_s(selectedConfig, newConfigName);
								}
							}
							ImGui::PopID();
						}

						/* --- WORLD --- */
						if (strcmp(currentTab, "World") == 0 && strcmp(current2Tab, "Aimbot") == 0) {
							ImGui::PushID("Aimbot");

							ImGui::Checkbox("Enabled", &raicu::globals::settings::aimbot::enabled);
							raicu::gui::other::hotkey("Aimbot key", &raicu::globals::settings::aimbot::hotkey);

							ImGui::Columns(2, "AimbotGrid", false);

							ImGui::Checkbox("Silent", &raicu::globals::settings::aimbot::silent);
							ImGui::Checkbox("Line to target", &raicu::globals::settings::aimbot::visualise_target_line);
							ImGui::NextColumn();
							ImGui::Checkbox("Trigger Fire", &raicu::globals::settings::aimbot::automatic_fire);
							ImGui::Checkbox("Penetrate walls", &raicu::globals::settings::aimbot::penetrate_walls);

							ImGui::Columns(1);

							ImGui::Combo("Hitbox", &raicu::globals::settings::aimbot::hitbox, aimbot_hitboxes,
							             IM_ARRAYSIZE(aimbot_hitboxes));
							ImGui::Combo("Priority", &raicu::globals::settings::aimbot::priority, aimbot_priorities,
							             IM_ARRAYSIZE(aimbot_priorities));

							ImGui::SeparatorText("Accuracy");
							ImGui::Checkbox("Predict spread", &raicu::globals::settings::aimbot::predict_spread);
							ImGui::SliderFloat("Smooth", &raicu::globals::settings::aimbot::smooth, 0.f, 20.f, "%.1f",
							                   ImGuiSliderFlags_NoInput);

							ImGui::SeparatorText("Backtrack");
							ImGui::Checkbox("Enable", &globals::settings::aimbot::backtrackEnabled);
							ImGui::SliderFloat("Backtrack", &raicu::globals::settings::aimbot::backtrack, 0.f, 1.f,
							                   "%.3f ms", ImGuiSliderFlags_NoInput);
							//ImGui::Combo("Material", &raicu::globals::settings::aimbot::backtrackMaterial, materialList, IM_ARRAYSIZE(materialList));
							ImGui::ColorEdit4("Colour", reinterpret_cast<float *>(&
								                  raicu::globals::settings::aimbot::backtrackColor),
							                  ImGuiColorEditFlags_NoAlpha);

							ImGui::PopID();
						}
						if (strcmp(currentTab, "World") == 0 && strcmp(current2Tab, "Movement") == 0) {
							ImGui::PushID("Movement");

							ImGui::Checkbox("Bunny Hop", &raicu::globals::settings::movement::bhop);
							ImGui::Checkbox("Air Strafe", &raicu::globals::settings::movement::air_strafe);

							ImGui::PopID();
						}
						if (strcmp(currentTab, "World") == 0 && strcmp(current2Tab, "Players") == 0) {
							ImGui::PushID("Player List");
							static int selected_player = -1;
							static char search_buffer[128] = "";

							int player_count = 0;
							for (size_t i = 0; i <= interfaces::engine->get_max_clients(); i++) {
								c_base_entity *entity = interfaces::entity_list->get_entity(static_cast<int>(i));
								if (entity && entity->is_player()) {
									player_info_t temp_info;
									if (interfaces::engine->get_player_info(i, &temp_info) && strlen(temp_info.name) >
									    0) {
										player_count++;
									}
								}
							}

							ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Players Online (%d)", player_count - 1);
							static bool show_friends_only = false;
							static bool show_whitelisted_only = false;
							static bool show_targets_only = false;

							ImGui::Checkbox("Show Friends Only", &show_friends_only);
							ImGui::SameLine();
							ImGui::Checkbox("Show Whitelisted Only", &show_whitelisted_only);
							ImGui::SameLine();
							ImGui::Checkbox("Show Targets Only", &show_targets_only);

							if (ImGui::Button("Clear friends")) {
							}
							ImGui::SameLine();
							if (ImGui::Button("Clear whitelist")) {
							}
							ImGui::SameLine();
							if (ImGui::Button("Clear targets")) {
							}

							ImGui::Checkbox("Spectator list", &raicu::globals::settings::other::spectatorList);

							ImGui::InputTextWithHint("##SearchPlayer", "Search by name...", search_buffer,
							                         IM_ARRAYSIZE(search_buffer));
							ImGui::Separator();

							player_info_t pinfo;

							if (ImGui::BeginChild("##PlayerList", ImVec2(0, 200), true)) {
								for (size_t i = 0; i <= interfaces::engine->get_max_clients(); i++) {
									c_base_entity *entity = interfaces::entity_list->get_entity(static_cast<int>(i));
									if (!entity) {
										continue;
									}

									if (entity->is_player()) {
										if (entity == interfaces::entity_list->get_entity(
											    interfaces::engine->get_local_player()))
											continue;

										if (interfaces::engine->get_player_info(i, &pinfo)) {
											if (strlen(pinfo.name) > 0) {
												std::string name_lower = pinfo.name;
												std::string search_lower = search_buffer;
												std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
												               ::tolower);
												std::transform(search_lower.begin(), search_lower.end(),
												               search_lower.begin(), ::tolower);
												if (!search_lower.empty() && name_lower.find(search_lower) ==
												    std::string::npos) {
													continue;
												}

												bool is_whitelisted = false;
												if (raicu::globals::settings::whitelist.contains("players")) {
													const auto &players = raicu::globals::settings::whitelist[
														"players"];
													is_whitelisted = std::any_of(players.begin(), players.end(),
														[&pinfo](const nlohmann::json &player) {
															return player["name"] == pinfo.name;
														});
												}

												bool is_friend = false;
												if (raicu::globals::settings::friend_list.contains("players")) {
													const auto &players = raicu::globals::settings::friend_list[
														"players"];
													is_friend = std::any_of(players.begin(), players.end(),
													                        [&pinfo](const nlohmann::json &player) {
														                        return player["name"] == pinfo.name;
													                        });
												}
												bool is_target = false;
												if (raicu::globals::settings::target_list.contains("players")) {
													const auto &players = raicu::globals::settings::target_list[
														"players"];
													is_target = std::any_of(players.begin(), players.end(),
													                        [&pinfo](const nlohmann::json &player) {
														                        return player["name"] == pinfo.name;
													                        });
												}

												bool should_display = true;
												if (show_friends_only && show_whitelisted_only) {
													should_display =
															is_friend || is_whitelisted || (
																is_friend && is_whitelisted);
												} else if (show_friends_only) {
													should_display = is_friend;
												} else if (show_whitelisted_only) {
													should_display = is_whitelisted;
												} else if (show_targets_only) {
													should_display = is_target;
												}

												if (!should_display) {
													continue;
												}

												ImGui::PushID(static_cast<int>(i));

												std::string display_name = pinfo.name;

												if (is_target) {
													display_name += " [TARGET]";
												}
												if (is_whitelisted) {
													display_name += " [WHITELISTED]";
												}
												if (is_friend) {
													display_name += " [FRIEND]";
												}

												if (is_whitelisted && is_friend) {
													ImGui::PushStyleColor(
														ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
												} else if (is_whitelisted) {
													ImGui::PushStyleColor(
														ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
												} else if (is_friend) {
													ImGui::PushStyleColor(
														ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
												} else if (is_target) {
													ImGui::PushStyleColor(
														ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
												}

												if (player_count % 2 == 0) {
													ImGui::PushStyleColor(
														ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 0.55f));
													ImGui::PushStyleColor(
														ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.55f));
												} else {
													ImGui::PushStyleColor(
														ImGuiCol_Header, ImVec4(0.15f, 0.15f, 0.15f, 0.55f));
													ImGui::PushStyleColor(
														ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.55f));
												}

												if (ImGui::Selectable(display_name.c_str(), selected_player == i)) {
													selected_player = i;
													ImGui::OpenPopup("PlayerOptions");
												}

												if (ImGui::BeginPopup("PlayerOptions")) {
													ImGui::Text("Player: %s", pinfo.name);
													ImGui::Separator();

													if (ImGui::MenuItem("Whitelist player", nullptr, is_whitelisted)) {
														if (is_whitelisted) {
															if (raicu::globals::settings::whitelist.
																contains("players")) {
																auto &players = raicu::globals::settings::whitelist[
																	"players"];
																players.erase(
																	std::remove_if(players.begin(), players.end(),
																		[&pinfo](const nlohmann::json &player) {
																			return player["name"] == pinfo.name;
																		}
																	),
																	players.end()
																);
															}
														} else {
															if (!raicu::globals::settings::whitelist.
																contains("players")) {
																raicu::globals::settings::whitelist["players"] =
																		nlohmann::json::array();
															}

															nlohmann::json player_entry;
															player_entry["name"] = pinfo.name;
															player_entry["index"] = i;

															raicu::globals::settings::whitelist["players"].push_back(
																player_entry);
														}
													}

													if (ImGui::MenuItem("Mark player as friend", nullptr, is_friend)) {
														if (is_friend) {
															if (raicu::globals::settings::friend_list.contains(
																"players")) {
																auto &friends = raicu::globals::settings::friend_list[
																	"players"];
																friends.erase(
																	std::remove_if(friends.begin(), friends.end(),
																		[&pinfo](const nlohmann::json &player) {
																			return player["name"] == pinfo.name;
																		}
																	),
																	friends.end()
																);
															}
														} else {
															if (!raicu::globals::settings::friend_list.contains(
																"players")) {
																raicu::globals::settings::friend_list["players"] =
																		nlohmann::json::array();
															}

															nlohmann::json player_entry;
															player_entry["name"] = pinfo.name;
															player_entry["index"] = i;

															raicu::globals::settings::friend_list["players"].push_back(
																player_entry);
														}
													}

													if (ImGui::MenuItem("Mark player as target", nullptr, is_target)) {
														if (is_target) {
															if (raicu::globals::settings::target_list.contains(
																"players")) {
																auto &friends = raicu::globals::settings::target_list[
																	"players"];
																friends.erase(
																	std::remove_if(friends.begin(), friends.end(),
																		[&pinfo](const nlohmann::json &player) {
																			return player["name"] == pinfo.name;
																		}
																	),
																	friends.end()
																);
															}
														} else {
															if (!raicu::globals::settings::target_list.contains(
																"players")) {
																raicu::globals::settings::target_list["players"] =
																		nlohmann::json::array();
															}

															nlohmann::json player_entry;
															player_entry["name"] = pinfo.name;
															player_entry["index"] = i;

															raicu::globals::settings::target_list["players"].push_back(
																player_entry);
														}
													}

													ImGui::EndPopup();
												}

												if (ImGui::IsItemHovered()) {
													ImGui::BeginTooltip();
													ImGui::Text("Player Index: %d", i);
													ImGui::EndTooltip();
												}

												ImGui::PopStyleColor(2);
												if (is_whitelisted || is_friend || is_target) {
													ImGui::PopStyleColor();
												}
												ImGui::PopID();
											}
										}
									}
								}
							}

							ImGui::EndChild();
							ImGui::PopID();
						}

						/* --- GAME --- */
						if (strcmp(currentTab, "Game") == 0 && strcmp(current2Tab, "Loading Screen") == 0) {
							ImGui::PushID("Loading Screen");
							ImGui::TextDisabled("NOT TESTED, MAY OR MAY NOT WORK");
							ImGui::Checkbox("Enable", &globals::settings::loading_screen::enabled);
							ImGui::InputTextWithHint("Loading Screen", "asset://garrysmod/html/menu.html",
							                         reinterpret_cast<char *>(&globals::settings::loading_screen::url),
							                         IM_ARRAYSIZE(globals::settings::loading_screen::url));

							ImGui::PopID();
						}


						ImGui::PopItemWidth();
					}
					ImGui::EndChild();
				}
				ImGui::EndGroup();
			}
			ImGui::End();
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));

	ImGui::RenderNotifications();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(1);

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	if (rt) {
		c_mat_render_context *ctx = interfaces::material_system->get_render_context();
		if (ctx) {
			ctx->begin_render();
			ctx->set_render_target(rt);
			ctx->end_render();
		}
	}

	rt = nullptr;

	if (stateBlock) {
		stateBlock->Apply();
		stateBlock->Release();
	}
}

bool containsBlacklistedWords(const std::string &message) {
	// TO HIDE CERTAIN LOGGING FROM IN-GAME LOG CONSOLE
	std::vector<std::string> blacklistedWords = {

	};

	for (const auto &word: blacklistedWords) {
		if (message.find(word) != std::string::npos) {
			return true;
		}
	}
	return false;
}

void raicu::gui::AddToLog(const int level, const std::string &message) {
	const char *levelStr = "";
	switch (level) {
		case 1: levelStr = "[INFO]    ";
			break;
		case 2: levelStr = "[WARNING] ";
			break;
		case 3: levelStr = "[ERROR]   ";
			break;
		case 4: levelStr = "[FATAL]   ";
			break;
		case 5: levelStr = "[SUCCESS] ";
			break;
		default: levelStr = "[UNKNOWN] ";
			break;
	}

	std::string fullMessage = std::string(levelStr) + message;
	if (!containsBlacklistedWords(fullMessage)) {
		gConsoleLog.AddLog(level, fullMessage);
	}
}

void raicu::gui::other::hotkey(const char *label, hotkey_t *hotkey) {
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	/*if (window->SkipItems)
		return;*/

	ImGui::SameLine();

	ImGuiContext &g = *GImGui;
	const ImGuiStyle &style = g.Style;
	const ImGuiID id = window->GetID(label);

	const float width = ImGui::GetColumnWidth();
	const ImVec2 pos = window->DC.CursorPos;

	char context_name[64] = {};
	ImFormatString(context_name, sizeof(context_name), "HotKeyContext%s", label);

	char text[64] = {};
	ImFormatString(text, sizeof(text), "[  %s  ]", (hotkey->key != 0 && g.ActiveId != id)
		                                               ? key_names[hotkey->key]
		                                               : (g.ActiveId == id)
			                                                 ? "WAIT KEY"
			                                                 : "NONE");

	const ImVec2 text_size = ImGui::CalcTextSize(text, NULL, true);

	const ImRect total_bb(ImVec2(pos.x + 120.f - (text_size.x + 10.f), pos.y - style.FramePadding.y + 8.f),
	                      ImVec2(pos.x + width, pos.y + text_size.y));

	ImGui::ItemSize(total_bb);
	if (!ImGui::ItemAdd(total_bb, id))
		return;

	const bool hovered = ImGui::ItemHoverable(total_bb, id, 0);
	if (hovered)
		ImGui::SetHoveredID(id);

	if (hovered && (g.IO.MouseClicked[0] || g.IO.MouseDoubleClicked[0])) {
		if (g.ActiveId != id) {
			memset(g.IO.MouseDown, 0, sizeof(g.IO.MouseDown));
			memset(g.IO.KeysDown, 0, sizeof(g.IO.KeysDown));
			hotkey->key = 0;
		}

		ImGui::SetActiveID(id, window);
	}

	bool changed = false;
	if (int key = hotkey->key; g.ActiveId == id) {
		for (int n = 0; n < IM_ARRAYSIZE(g.IO.MouseDown); n++) {
			if (ImGui::IsMouseDown(n)) {
				switch (n) {
					case 0:
						key = VK_LBUTTON;
						break;
					case 1:
						key = VK_RBUTTON;
						break;
					case 2:
						key = VK_MBUTTON;
						break;
					case 3:
						key = VK_XBUTTON1;
						break;
					case 4:
						key = VK_XBUTTON2;
						break;
				}

				changed = true;
				ImGui::ClearActiveID();
			}
		}

		if (!changed) {
			for (int n = VK_BACK; n <= VK_RMENU; n++) {
				if (ImGui::IsKeyDown((ImGuiKey) n)) {
					key = n;
					changed = true;
					ImGui::InsertNotification({ImGuiToastType::Success, 3000, "Keybind %s has changed", label});
					ImGui::ClearActiveID();
				}
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			hotkey->key = 0;
			ImGui::ClearActiveID();
		} else
			hotkey->key = key;
	}

	ImGui::RenderText(total_bb.Min, text);

	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(80, 65));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));

	if (ImGui::BeginPopupContextItem(context_name)) {
		ImGui::SetWindowPos(ImVec2(total_bb.Min.x, total_bb.Max.y));

		if (ImGui::Selectable("Always on", hotkey->type == hotkey_t::always_on))
			hotkey->type = hotkey_t::always_on;

		if (ImGui::Selectable("Hold", hotkey->type == hotkey_t::hold))
			hotkey->type = hotkey_t::hold;

		if (ImGui::Selectable("Toggle", hotkey->type == hotkey_t::toggle))
			hotkey->type = hotkey_t::toggle;

		if (ImGui::Selectable("Force disable", hotkey->type == hotkey_t::force_disable))
			hotkey->type = hotkey_t::force_disable;

		ImGui::End();
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
}

LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParam, LPARAM longParam) {
	if (GetAsyncKeyState(VK_HOME) & 1)
		raicu::globals::settings::open = !raicu::globals::settings::open;

	if (raicu::globals::settings::open)
		if (ImGui_ImplWin32_WndProcHandler(window, message, wideParam, longParam))
			return 1L;


	return CallWindowProc(raicu::gui::originalWindowProcess, window, message, wideParam, longParam);
}
