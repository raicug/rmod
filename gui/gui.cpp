#include "gui.h"
#include "ext/imgui/imgui.h"
#include "ext/imgui/backends/imgui_impl_dx9.h"
#include "ext/imgui/backends/imgui_impl_win32.h"

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

#pragma comment(lib, "wininet.lib")

const float MAX_SIZE_FLOAT{50.f};
const float MIN_SIZE_FLOAT{1.f};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static const ImGuiColorEditFlags colorPickerFlags =
		ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview;

LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParam, LPARAM longParam);

bool raicu::gui::WriteToRegistry(const std::wstring &valueName, const std::wstring &value) {
	const std::wstring keyPath = L"Environment";

	HKEY hKey;
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, reinterpret_cast<LPCSTR>(keyPath.c_str()), 0, KEY_WRITE, &hKey);
	if (result != ERROR_SUCCESS) {
		logger::Log(3, "Failed to open registry key");
		return false;
	}

	result = RegSetValueEx(hKey, reinterpret_cast<LPCSTR>(valueName.c_str()), 0, REG_SZ,
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
	HINTERNET hInternet = InternetOpen("WinINet HTTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
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
	ImGuiTextFilter filter;

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
		ImGui::SameLine();
		filter.Draw("Filter", -100.0f);
		ImGui::Separator();

		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
			if (clear) Clear();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGuiListClipper clipper;
			clipper.Begin(entries.size());

			while (clipper.Step()) {
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
					const auto &entry = entries[i];

					if (filter.IsActive() && !filter.PassFilter(entry.text.c_str()))
						continue;

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
	logger::Log(1, "Creating window class");

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.hIcon = nullptr;
	windowClass.hCursor = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = nullptr;

	if (!RegisterClassEx(&windowClass)) {
		logger::Log(3, "Failed to create window class");
		return false;
	}

	logger::Log(5, "Created window class");

	return true;
}

void raicu::gui::DestroyWindowClass() noexcept {
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool raicu::gui::SetupWindow(const char *windowName) noexcept {
	logger::Log(1, "Creating window");

	window = CreateWindow(windowClass.lpszClassName, windowName, WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr,
	                      windowClass.hInstance, nullptr);

	if (!window) {
		logger::Log(3, "Failed to create window");
		return false;
	}

	logger::Log(5, "Created window");

	return true;
}

void raicu::gui::DestroyWindow() noexcept {
	if (window) DestroyWindow(window);
}

bool raicu::gui::SetupDirectX() noexcept {
	logger::Log(1, "Creating DirectX");

	const auto handle = GetModuleHandleA("d3d9.dll");

	if (!handle) {
		logger::Log(3, "Failed to load d3d9.dll");
		return false;
	}

	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	const auto create = reinterpret_cast<CreateFn>(GetProcAddress(handle, "Direct3DCreate9"));

	if (!create) {
		logger::Log(3, "Failed to get Direct3DCreate9 address");
		return false;
	}

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9) {
		logger::Log(3, "Failed to get D3D9 SDK version from address");
		return false;
	}

	D3DPRESENT_PARAMETERS params = {};
	params.BackBufferWidth = 100;
	params.BackBufferHeight = 100;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;
	params.BackBufferCount = 1;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = window;
	params.Windowed = TRUE;
	params.EnableAutoDepthStencil = TRUE;
	params.AutoDepthStencilFormat = D3DFMT_D16;
	params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
	                                &params, &device);

	if (FAILED(hr)) {
		logger::Log(3, "Failed to create DirectX device");
		return false;
	}

	logger::Log(5, "Created DirectX device");
	return true;
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

const char *sidebarTabs[] = {"Visuals", "Appearance", "Lua", "Config", "World"};
const char *topTabsVisuals[] = {"FOV", "Crosshair", "ESP", "Chams"};
const char *topTabsAppearance[] = {"Main", "Console Colours"};
const char *topTabsLua[] = {"Main"};
const char *topTabsWorld[] = {"Aimbot" /*"Movement"*/};
const char *topTabsConfig[] = {"Loading", "Saving"};

const char **topTabsArray[] = {topTabsVisuals, topTabsAppearance, topTabsLua, topTabsConfig, topTabsWorld};
int topTabSizes[] = {
	IM_ARRAYSIZE(topTabsVisuals),
	IM_ARRAYSIZE(topTabsAppearance),
	IM_ARRAYSIZE(topTabsLua),
	IM_ARRAYSIZE(topTabsConfig),
	IM_ARRAYSIZE(topTabsWorld)
};

void raicu::gui::Render() noexcept {
	IDirect3DStateBlock9 *stateBlock = nullptr;

	if (SUCCEEDED(device->CreateStateBlock(D3DSBT_ALL, &stateBlock)))
		stateBlock->Capture();

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	raicu::gui::themes::setFluentUITheme();

	if (g_needsLogin) {
		ShowLogin();
	} else {
		ShowLog();

		raicu::cheats::Visuals::Render();

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

			std::string title = std::string("R-MOD | ") + raicu::globals::settings::version;

			ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse); {
				static int selectedSidebar = 3;
				static int selectedTopTabs[IM_ARRAYSIZE(sidebarTabs)] = {0};

				ImGui::BeginChild("Sidebar##mainui", ImVec2(150, 0), true); {
					for (int i = 0; i < IM_ARRAYSIZE(sidebarTabs); i++) {
						if (ImGui::Selectable(sidebarTabs[i], selectedSidebar == i)) {
							selectedSidebar = i;
						}
					}
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

					// ImGui::Text("Welcome to %s | %s", currentTab, current2Tab);

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
						ImGui::Checkbox("Outline Enabled", &raicu::globals::settings::crosshairValues::outlineEnabled);

						ImGui::SliderFloat("Outline Thickness",
						                   &raicu::globals::settings::crosshairValues::outlineThickness, 0.0f, 10.0f,
						                   "%.2f");
						ImGui::SliderFloat("Rounding", &raicu::globals::settings::crosshairValues::rounding, 0.0f,
						                   10.0f, "%.2f");

						ImGui::SliderFloat("Width", &raicu::globals::settings::crosshairValues::width, 0.1f, 100.0f,
						                   "%.1f");
						ImGui::SliderFloat("Height", &raicu::globals::settings::crosshairValues::height, 0.1f, 100.0f,
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
						ImGui::NextColumn();
						ImGui::Checkbox("Origin", &raicu::globals::settings::espValues::origin);
						ImGui::Checkbox("Name", &raicu::globals::settings::espValues::name);
						ImGui::Checkbox("Snapline", &raicu::globals::settings::espValues::snapline);

						ImGui::Columns(1);

						ImGui::Combo("Snapline Position", &raicu::globals::settings::espValues::snaplinePosition,
						             snaplinePosition, IM_ARRAYSIZE(snaplinePosition));
						ImGui::SliderInt("Render Distance", &raicu::globals::settings::espValues::render_distance, 1000,
						                 30000);

						ImGui::ColorEdit4(
							"Snapline", reinterpret_cast<float *>(&raicu::globals::settings::espValues::snapLineColor),
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

						ImGui::PopID();
					}

					/* --- APPEARANCE --- */
					if (strcmp(currentTab, "Appearance") == 0 && strcmp(current2Tab, "Main") == 0) {
						ImGui::Checkbox("Enable console", &raicu::globals::settings::consoleOpen);
						ImGui::Checkbox("Enable logging notifications", &raicu::globals::settings::loggerNotifications);
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

						editor.Render("##Source", ImVec2(400, 300)); {
							raicu::globals::settings::lua::ScriptInput = editor.GetText();
						}

						bool executePressed = false;

						if (ImGui::Button("Execute")) executePressed = true;
						ImGui::SameLine();
						ImGui::Combo("##State", &raicu::globals::settings::lua::executeState, executorLuaState,
						             IM_ARRAYSIZE(executorLuaState));

						if (executePressed) {
							std::lock_guard<std::mutex> lock(lua::executionData.mutex);
							lua::executionData.script = globals::settings::lua::ScriptInput;
							lua::executionData.waiting.store(true);
						}

						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);

						ImGui::PopStyleVar();
					}

					/* --- CONFIG --- */
					if (strcmp(currentTab, "Config") == 0 && strcmp(current2Tab, "Loading") == 0) {
						ImGui::PushID("Config - Loading");

						configs = ConfigManager::GetAllConfigs();

						if (!configs.empty()) {
							if (ImGui::BeginCombo("Select Config", selectedConfig[0] ? selectedConfig : "Select...")) {
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

						ImGui::Columns(2, "AimbotGrid", false);

						ImGui::Checkbox("Enabled", &raicu::globals::settings::aimbot::enabled);
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
						ImGui::SliderFloat("Backtrack", &raicu::globals::settings::aimbot::backtrack, 0.f, 20.f, "%.1f",
						                   ImGuiSliderFlags_NoInput);

						ImGui::PopID();
					}
					if (strcmp(currentTab, "World") == 0 && strcmp(current2Tab, "Movement") == 0) {
						ImGui::PushID("Movement");

						ImGui::Checkbox("Bunny Hop", &raicu::globals::settings::movement::bhop);

						ImGui::PopID();
					}

					ImGui::PopItemWidth();
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

	if (stateBlock) {
		stateBlock->Apply();
		stateBlock->Release();
	}
}

bool containsBlacklistedWords(const std::string &message) { // TO HIDE CERTAIN LOGGING FROM IN-GAME LOG CONSOLE
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

LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParam, LPARAM longParam) {
	if (GetAsyncKeyState(VK_HOME) & 1)
		raicu::globals::settings::open = !raicu::globals::settings::open;

	if (raicu::globals::settings::open)
		if (ImGui_ImplWin32_WndProcHandler(window, message, wideParam, longParam))
			return 1L;


	return CallWindowProc(raicu::gui::originalWindowProcess, window, message, wideParam, longParam);
}
