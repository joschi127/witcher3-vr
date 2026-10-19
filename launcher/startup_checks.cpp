#include "startup_checks.h"

#include <Windows.h>
#include <TlHelp32.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cwctype>
#include <iomanip>
#include <sstream>

namespace w3vr {
namespace {

const auto kKnownDx12Dlls = std::to_array<std::wstring_view>({
    // Files shipped by The Witcher 3 DX12 runtime.
    L"APEX_ClothingGPU_x64.dll",
    L"cudart64_50_35.dll",
    L"d3dcompiler_47.dll",
    L"dxcompiler.dll",
    L"dxil.dll",
    L"Galaxy64.dll",
    L"GFSDK_Aftermath_Lib.x64.dll",
    L"GFSDK_HairWorks.win64.dll",
    L"GFSDK_SSAO.win64.dll",
    L"GFSDK_SSAO_D3D12.win32.dll",
    L"GFSDK_SSAO_D3D12.win64.dll",
    L"glew32.dll",
    L"GRB_1_1_api3_x64.dll",
    L"igxess.dll",
    L"libcurl.dll",
    L"libxess.dll",
    L"NvCameraSDK64.dll",
    L"NVHair_x64.dll",
    L"NVHairExt_x64.dll",
    L"nvngx_dlss.dll",
    L"nvngx_dlssd.dll",
    L"nvngx_dlssg.dll",
    L"nvngx_dlssnr.dll",
    L"PhysX3Common_x64.dll",
    L"PhysX3Gpu_x64.dll",
    L"PhysXDevice64.dll",
    L"REDGalaxy64.dll",
    L"RedTelemetryLib.dll",
    L"sl.common.dll",
    L"sl.dlss.dll",
    L"sl.dlss_g.dll",
    L"sl.interposer.dll",
    L"sl.reflex.dll",
    L"steam_api64.dll",
    L"XeFX.dll",
    L"XeFX_Loader.dll",
    // Additionally shipped by The Witcher 3 Remastered (5.x) DX12 runtime.
    L"amd_ags_x64.dll",
    L"amd_fidelityfx_loader_dx12.dll",
    L"libxell.dll",
    L"libxess_dx11.dll",
    L"libxess_fg.dll",
    L"msvcp110.dll",
    L"normaliz.dll",
    L"sl.dlss_d.dll",
    L"sl.pcl.dll",
    L"Wldap32.dll",
    L"xaudio2_9redist.dll",

    // Witcher 3 VR and its dedicated optional release packages.
    L"dxgi.dll",
    L"openxr_loader.dll",
    L"PDAFWPlugin.dll",
    L"OptiScaler.dll",
    // [FIX:DLSS5-RUNTIME-WHITELIST V23024 1/1] This root file belongs to
    // the managed modified-OptiScaler DLSSNR route.
    L"nvngx.dll_dlssnr.dll",
    L"amd_fidelityfx_dx12.dll",
    L"amd_fidelityfx_upscaler_dx12.dll",
    L"amd_fidelityfx_framegeneration_dx12.dll",
    L"ofxr_amd_fidelityfx_framegeneration_dx12.dll",
    L"renderdoc.dll",
    L"ReShade64.dll",
    // [FIX:OFXR-STARTUP-WHITELIST V1512 1/2] Root-owned XRFG-V041 layer.
    L"XR_APILAYER_XRFrameBridge_diagnostic.dll",
});

bool SameFilename(std::wstring_view left, std::wstring_view right) {
    return left.size() == right.size() &&
        _wcsnicmp(left.data(), right.data(), left.size()) == 0;
}

bool ProcessNameMatches(
    std::wstring_view candidate,
    const std::array<std::wstring_view, 3>& expected) {
    return std::any_of(expected.begin(), expected.end(),
        [&](std::wstring_view name) { return SameFilename(candidate, name); });
}

std::vector<std::wstring> NormalizeDllNames(
    const std::vector<std::wstring>& filenames) {
    std::vector<std::wstring> normalized = filenames;
    for (auto& filename : normalized) {
        std::transform(filename.begin(), filename.end(), filename.begin(),
            [](wchar_t character) {
                return static_cast<wchar_t>(std::towlower(character));
            });
    }
    std::sort(normalized.begin(), normalized.end());
    normalized.erase(std::unique(normalized.begin(), normalized.end()),
        normalized.end());
    return normalized;
}

} // namespace

bool IsWindowsHdrActive() {
    for (int attempt = 0; attempt < 3; ++attempt) {
        UINT32 path_count{};
        UINT32 mode_count{};
        if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS,
                &path_count, &mode_count) != ERROR_SUCCESS) {
            return false;
        }

        std::vector<DISPLAYCONFIG_PATH_INFO> paths(path_count);
        std::vector<DISPLAYCONFIG_MODE_INFO> modes(mode_count);
        const LONG query = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,
            &path_count, paths.data(), &mode_count, modes.data(), nullptr);
        if (query == ERROR_INSUFFICIENT_BUFFER) continue;
        if (query != ERROR_SUCCESS) return false;
        paths.resize(path_count);

        for (const auto& path : paths) {
            DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO color{};
            color.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
            color.header.size = sizeof(color);
            color.header.adapterId = path.targetInfo.adapterId;
            color.header.id = path.targetInfo.id;
            if (DisplayConfigGetDeviceInfo(&color.header) == ERROR_SUCCESS &&
                color.advancedColorEnabled) {
                return true;
            }
        }
        return false;
    }
    return false;
}

bool IsRivaTunerStatisticsServerRunning() {
    constexpr std::array<std::wstring_view, 3> process_names{
        L"RTSS.exe", L"RTSSHooksLoader.exe", L"RTSSHooksLoader64.exe"};
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W entry{sizeof(entry)};
    bool found{};
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (ProcessNameMatches(entry.szExeFile, process_names)) {
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return found;
}

bool IsKnownDx12Dll(std::wstring_view filename) {
    return std::any_of(kKnownDx12Dlls.begin(), kKnownDx12Dlls.end(),
        [&](std::wstring_view known) { return SameFilename(filename, known); });
}

std::vector<std::wstring> FindForeignDx12Dlls(
    const std::filesystem::path& dx12_directory) {
    std::vector<std::wstring> result;
    std::error_code error;
    std::filesystem::directory_iterator iterator(
        dx12_directory,
        std::filesystem::directory_options::skip_permission_denied,
        error);
    const std::filesystem::directory_iterator end;
    while (!error && iterator != end) {
        const auto& entry = *iterator;
        std::error_code type_error;
        if (entry.is_regular_file(type_error)) {
            const auto filename = entry.path().filename().wstring();
            if (_wcsicmp(entry.path().extension().c_str(), L".dll") == 0 &&
                !IsKnownDx12Dll(filename)) {
                result.push_back(filename);
            }
        }
        iterator.increment(error);
    }
    std::sort(result.begin(), result.end(),
        [](const std::wstring& left, const std::wstring& right) {
            return _wcsicmp(left.c_str(), right.c_str()) < 0;
        });
    return result;
}

std::string ForeignDllWarningSignature(
    const std::vector<std::wstring>& filenames) {
    const auto normalized = NormalizeDllNames(filenames);
    if (normalized.empty()) return {};

    constexpr uint64_t kOffset = 14695981039346656037ull;
    constexpr uint64_t kPrime = 1099511628211ull;
    uint64_t hash = kOffset;
    for (const auto& filename : normalized) {
        for (wchar_t character : filename) {
            const auto value = static_cast<uint16_t>(character);
            hash ^= static_cast<uint8_t>(value & 0xffu);
            hash *= kPrime;
            hash ^= static_cast<uint8_t>((value >> 8u) & 0xffu);
            hash *= kPrime;
        }
        hash ^= 0xffu;
        hash *= kPrime;
    }

    std::ostringstream output;
    output << std::hex << std::setfill('0') << std::setw(16) << hash;
    return output.str();
}

bool IsForeignDllWarningSuppressed(
    const ConfigPaths& paths, const std::string& signature) {
    if (signature.empty()) return false;
    std::wstring error;
    const auto ini = IniDocument::Load(paths.vr_ini, error);
    if (!ini) return false;
    return ini->Get("launcher", "ignored_foreign_dll_signature") == signature;
}

bool SuppressForeignDllWarning(
    const ConfigPaths& paths, const std::string& signature,
    std::wstring& error) {
    if (signature.empty()) return true;
    const auto loaded = IniDocument::Load(paths.vr_ini, error);
    if (!loaded) return false;
    IniDocument updated = *loaded;
    updated.Set("launcher", "ignored_foreign_dll_signature", signature);
    return AtomicWriteWithBackup(paths.vr_ini, updated.Serialize(), error);
}

} // namespace w3vr
