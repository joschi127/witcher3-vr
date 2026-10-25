#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "../src/cinema_aspect.h"

namespace w3vr {

enum class RenderMode {
    MonoNone,
    MonoTaau,
    MonoDlss,
    AerAfwTaau,
    AerAfwDlss,
    StereoNone,
    StereoTaau,
    StereoDlssSequential,
    Count,
};

enum class CameraFollowPolicy {
    AlwaysOn,
    HorseBoatOnly,
    AlwaysOff,
};

enum class FrameGenerationBackend {
    Off,
    FidelityFx,
    Nvidia,
    NvidiaNew,
    Fsr3,
    Count,
};

enum class OfxrNvidiaPreset {
    Fast,
    Medium,
    Slow,
    Count,
};

enum class OfxrNvidiaInputScale {
    Half,
    ThreeQuarter,
    Full,
    Count,
};

enum class OfxrOverlayPosition {
    Off,
    UpperLeft,
    UpperRight,
    LowerLeft,
    LowerRight,
    Count,
};

enum class IntegrationMode {
    Off,
    Optiscaler,
    Reshade,
    OptiscalerReshade,
    Count,
};

constexpr bool CameraFollowEnabled(
    CameraFollowPolicy policy, bool horse_or_boat, bool first_person) {
    if (policy == CameraFollowPolicy::AlwaysOff) return false;
    if (horse_or_boat) return true;
    return policy == CameraFollowPolicy::AlwaysOn && !first_person;
}

constexpr bool StaticHudModuleVisible(
    bool policy_enabled, bool in_combat, bool witcher_sense,
    bool horse_racing, bool navigation_module) {
    return !policy_enabled || in_combat || witcher_sense ||
        (navigation_module && horse_racing);
}

struct ModeSettings {
    int openxr_mode{};
    bool dual_render{};
    bool mode3_aer_presentation{};
    const char* temporal_backend{};
    int aa_mode{};
    bool allow_dlss{};
};

struct LauncherState {
    // This fork's default for both game versions: AER + AFW needs
    // PDAFWPlugin.dll (not shipped) and 5.x has no TAAU/DLSS route yet.
    RenderMode mode{RenderMode::StereoNone};
    bool resolution_auto{true};
    int width{2688};
    int height{2784};
    int dlss_quality{3};
    IntegrationMode integration_mode{IntegrationMode::Off};
    FrameGenerationBackend frame_generation_backend{
        FrameGenerationBackend::Off};
    OfxrNvidiaPreset ofxr_nvidia_preset{OfxrNvidiaPreset::Medium};
    OfxrNvidiaInputScale ofxr_nvidia_input_scale{
        OfxrNvidiaInputScale::Half};
    bool ofxr_nvidia_bidirectional{};
    OfxrOverlayPosition ofxr_overlay_position{
        OfxrOverlayPosition::UpperRight};
    bool ofxr_logging{};
    bool optiscaler_logging{};
    int hud_convergence_delta{-20};
    bool hud_controller_locked{};
    float presentation_scale{1.0f};
    float world_detail_range{1.0f};
    float menu_scale{0.85f};
    float cinema_scale{0.9f};
    CinemaAspect cinema_aspect{CinemaAspect::FiveFour};
    float cinema_hud_scale{1.30f};
    int cinema_hud_convergence_offset{};
    float full_vr_hud_scale{1.00f};
    int full_vr_hud_convergence_offset{};
    float near_view{0.75f};
    bool vertical_pitch_enabled{};
    bool cinema_full_vr{true};
    bool steady_icons{};
    // [openxr] recenter_key as written in the INI ("F9", "Space",
    // "NumpadDivide", or any other value the DLL accepts, which is kept).
    std::string recenter_key{"F9"};
    bool first_person_gamepad_head_follow{};
    int first_person_snap_turn_degrees{45};
    bool first_person_combat_exit{false};
    bool first_person_strafe{true};
    bool first_person_anchor_smoothing{true};
    CameraFollowPolicy camera_follow_policy{CameraFollowPolicy::HorseBoatOnly};
    bool hide_static_hud_outside_combat{};
    bool fast_movement_transitions{true};
    // Remastered 5.x only: the game's FSR (AAMode 5) with one FSR history per
    // eye ([engine] remaster_fsr_per_eye=1) instead of FXAA (AAMode 1).
    bool fsr_per_eye{};
    bool fullscreen_projection{};
    bool diagnostic_logging{};
    bool route_logging{};
    bool performance_logging{};
    bool renderdoc_enabled{};
    // Detected game edition (not stored in the INI). Remastered 5.x supports
    // Stereo/Mono with FXAA or per-eye FSR and uses different AAMode values.
    bool remastered{};
};

enum class GameEdition {
    Unknown,
    NextGen,     // witcher3.exe file version 4.x (Next-Gen 4.04)
    Remastered,  // witcher3.exe file version 5.x
};

// Reads the major file version of witcher3.exe.
GameEdition DetectGameEdition(const std::filesystem::path& game_executable);
// Installs the state bridge script variant for the detected game version
// (4.04 and 5.x need different state wrapper names). Succeeds without changes
// when the edition is unknown or the state bridge mod is not installed.
bool EnsureEditionScripts(const std::filesystem::path& launcher_directory,
    GameEdition edition, std::wstring& error);

// The closest render mode that can be used: Stereo/Mono with FXAA on
// Remastered 5.x, and the matching Stereo mode instead of AER + AFW when
// PDAFWPlugin.dll is missing.
RenderMode AvailableRenderMode(
    RenderMode mode, bool remastered, bool afw_plugin_present);

// Returns why the selected render mode cannot be used, or an empty string.
// AER + AFW needs PureDark's PDAFWPlugin.dll, which this fork does not ship.
std::wstring RenderModeUnavailableReason(
    const LauncherState& state, bool afw_plugin_present);

struct CompatibilityWarnings {
    bool ssr_high{};
};

struct ConfigPaths {
    std::filesystem::path launcher_directory;
    std::filesystem::path vr_ini;
    std::filesystem::path optiscaler_bridge_ini;
    std::filesystem::path ofxr_bridge_ini;
    std::filesystem::path game_settings;
    std::filesystem::path game_executable;
    std::filesystem::path optiscaler_ini;
};

struct LoadResult {
    LauncherState state;
    std::wstring warning;
};

class IniDocument {
public:
    static std::optional<IniDocument> Load(
        const std::filesystem::path& path, std::wstring& error);
    static IniDocument FromText(std::string text);

    std::optional<std::string> Get(
        const std::string& section, const std::string& key) const;
    std::vector<std::pair<std::string, std::string>> Entries(
        const std::string& section) const;
    void Set(const std::string& section, const std::string& key,
        const std::string& value);
    void Remove(const std::string& section, const std::string& key);
    void FillMissingFrom(const IniDocument& defaults);
    std::string Serialize() const;

private:
    struct Line {
        std::string text;
        std::string newline;
    };
    std::vector<Line> lines_;
    std::string default_newline_{"\r\n"};
};

const ModeSettings& SettingsForMode(RenderMode mode);
const wchar_t* ModeDisplayName(RenderMode mode);
bool ModeUsesDlss(RenderMode mode);
bool ModeUsesStereo(RenderMode mode);
const wchar_t* IntegrationModeDisplayName(IntegrationMode mode);
const char* IntegrationModeIniValue(IntegrationMode mode) noexcept;
IntegrationMode ParseIntegrationMode(const std::string& value) noexcept;
bool IntegrationModeUsesOptiscaler(IntegrationMode mode) noexcept;
bool IntegrationModeUsesReshade(IntegrationMode mode) noexcept;
std::optional<int> DlssNearSquareCompatibleWidth(const LauncherState& state);
int CinemaHudConvergenceShift(float hud_scale, int offset);
int FullVrHudConvergenceShift(float hud_scale, int offset);

ConfigPaths DiscoverPaths();
bool EnsureVrConfiguration(const ConfigPaths& paths,
    const std::string& template_contents, bool& created, std::wstring& error);
bool EnsureHudEditorSetup(const ConfigPaths& paths, std::wstring& error);
std::wstring HudEditorManualSetupInstructions(const ConfigPaths& paths);
// On Remastered 5.x the game's AA keys use other values and are written by
// Save for every mode, so only the VR INI selects the displayed mode.
LoadResult LoadConfiguration(const ConfigPaths& paths, bool remastered = false);
CompatibilityWarnings InspectCompatibilitySettings(const ConfigPaths& paths);
bool BuildUpdatedDocuments(const ConfigPaths& paths, const LauncherState& state,
    IniDocument& vr_ini, IniDocument& game_settings, std::wstring& error);
bool BuildUpdatedOptiscalerDocument(const ConfigPaths& paths,
    const LauncherState& state, IniDocument& optiscaler,
    std::wstring& error);
bool BuildUpdatedOptiscalerIniDocument(const ConfigPaths& paths,
    const LauncherState& state, IniDocument& optiscaler,
    std::wstring& error);
bool BuildUpdatedOfxrDocument(const ConfigPaths& paths,
    const LauncherState& state, IniDocument& ofxr,
    std::wstring& error);
bool SaveConfiguration(const ConfigPaths& paths, const LauncherState& state,
    std::wstring& error);
bool AtomicWriteWithBackup(const std::filesystem::path& path,
    const std::string& contents, std::wstring& error);
std::filesystem::path OriginalSettingsBackupPath(const ConfigPaths& paths);
bool HasOriginalSettingsBackup(const ConfigPaths& paths);
bool ConfigureGameSettingsForVr(const ConfigPaths& paths,
    const std::string& template_contents, std::wstring& error);
// Remastered 5.x counterpart of ConfigureGameSettingsForVr: sets the VR
// settings that matter on 5.x in the existing dx12user.settings and keeps
// everything else (the 4.04 template must never be applied to 5.x).
bool ConfigureRemasteredGameSettingsForVr(
    const ConfigPaths& paths, std::wstring& error);
bool RestoreOriginalGameSettings(const ConfigPaths& paths, std::wstring& error);

} // namespace w3vr
