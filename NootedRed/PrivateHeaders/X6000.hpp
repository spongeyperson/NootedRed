// Copyright © 2022-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5.
// See LICENSE for details.

#pragma once
#include <Headers/kern_patcher.hpp>
#include <PrivateHeaders/ObjectField.hpp>

class X6000 {
    friend class X5000;

    bool initialised {false};
    ObjectField<UInt32> regBaseField {};
    mach_vm_address_t orgAllocateAMDHWDisplay {0};
    mach_vm_address_t orgInitDCNRegistersOffsets {0};

    public:
    static X6000 &singleton();

    void init();

    private:
    bool processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t slide, size_t size);

    static bool wrapAccelStartX6000();
    static void wrapInitDCNRegistersOffsets(void *that);
};

template<UInt32 N>
struct HWAlignVTableFix {
    const UInt32 offs[N];
    const UInt32 occurances[N];
    const UInt32 len {N};

    void apply(void *toFunction) const {
        for (UInt32 i = 0; i < this->len; i += 1) {
            const UInt32 off = this->offs[i];
            const UInt32 newOff = (off == 0x128) ? 0x230 : (off - 8);
            const UInt32 count = this->occurances[i];
            const UInt8 vtableCallPattern[] = {0xFF, 0x00, static_cast<UInt8>(off & 0xFF),
                static_cast<UInt8>((off >> 8) & 0xFF), static_cast<UInt8>((off >> 16) & 0xFF),
                static_cast<UInt8>((off >> 24) & 0xFF)};
            const UInt8 vtableCallMask[] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
            const UInt8 vtableCallReplacement[] = {0xFF, 0x00, static_cast<UInt8>(newOff & 0xFF),
                static_cast<UInt8>((newOff >> 8) & 0xFF), static_cast<UInt8>((newOff >> 16) & 0xFF),
                static_cast<UInt8>((newOff >> 24) & 0xFF)};
            PANIC_COND(!KernelPatcher::findAndReplaceWithMask(toFunction, PAGE_SIZE, vtableCallPattern, vtableCallMask,
                           vtableCallReplacement, vtableCallMask, count, 0),
                "X6000", "Failed to apply virtual call fix");
        }
    }
};

//------ Patches ------//

// Mismatched `getTtlInterface` virtual calls
static const UInt8 kGetTtlInterfaceCallOriginal[] = {0x40, 0x80, 0x00, 0xFF, 0x90, 0xC8, 0x02, 0x00, 0x00};
static const UInt8 kGetTtlInterfaceCallOriginalMask[] = {0xF0, 0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 kGetTtlInterfaceCallPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00};
static const UInt8 kGetTtlInterfaceCallPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00};

// Mismatched `getGpuDebugPolicy` virtual calls.
static const UInt8 kGetGpuDebugPolicyCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xC0, 0x03, 0x00, 0x00};
static const UInt8 kGetGpuDebugPolicyCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xC8, 0x03, 0x00, 0x00};

// Ditto
static const UInt8 kGetGpuDebugPolicyCallOriginal1015[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xC8, 0x03, 0x00, 0x00};
static const UInt8 kGetGpuDebugPolicyCallPatched1015[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xC0, 0x03, 0x00, 0x00};

// VTable Call to signalGPUWorkSubmitted.
// Doesn't exist on X5000, but looks like it isn't necessary, so we just NO-OP it.
static const UInt8 kHWChannelSubmitCommandBufferOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x30, 0x02, 0x00, 0x00,
    0x48, 0x8B, 0x43};
static const UInt8 kHWChannelSubmitCommandBufferPatched[] = {0x48, 0x8B, 0x07, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x48,
    0x8B, 0x43};

// Ditto
static const UInt8 kHWChannelSubmitCommandBufferOriginal1015[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x20, 0x02, 0x00, 0x00,
    0x49, 0x8B, 0x45};
static const UInt8 kHWChannelSubmitCommandBufferPatched1015[] = {0x48, 0x8B, 0x07, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90,
    0x49, 0x8B, 0x45};

// Mismatched `getScheduler` virtual calls.
static const UInt8 kGetSchedulerCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xB8, 0x03, 0x00, 0x00};
static const UInt8 kGetSchedulerCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xC0, 0x03, 0x00, 0x00};

// Ditto
static const UInt8 kGetSchedulerCallOriginal13[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xB0, 0x03, 0x00, 0x00};
static const UInt8 kGetSchedulerCallPatched13[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xB8, 0x03, 0x00, 0x00};

// Ditto
static const UInt8 kGetSchedulerCallOriginal1015[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xC0, 0x03, 0x00, 0x00};
static const UInt8 kGetSchedulerCallPatched1015[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xB8, 0x03, 0x00, 0x00};

// Mismatched `isDeviceValid` virtual calls.
static const UInt8 kIsDeviceValidCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xA0, 0x02, 0x00, 0x00};
static const UInt8 kIsDeviceValidCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x98, 0x02, 0x00, 0x00};

// Mismatched `isDevicePCITunnelled` virtual calls.
static const UInt8 kIsDevicePCITunnelledCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xB0, 0x02, 0x00, 0x00};
static const UInt8 kIsDevicePCITunnelledCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xA8, 0x02, 0x00, 0x00};

// Mismatched `getSML` virtual calls.
static const UInt8 kGetSMLCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x98, 0x03, 0x00, 0x00};
static const UInt8 kGetSMLCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x90, 0x03, 0x00, 0x00};

// Mismatched `getUbmSwizzleMode` virtual call in `fillUBMSurface`.
static const UInt8 kGetUbmSwizzleModeCallOriginal[] = {0xFF, 0x91, 0x78, 0x04, 0x00, 0x00};
static const UInt8 kGetUbmSwizzleModeCallPatched[] = {0xFF, 0x91, 0xA0, 0x04, 0x00, 0x00};

// Mismatched `getUbmTileMode` virtual call in `fillUBMSurface`.
static const UInt8 kGetUbmTileModeCallOriginal[] = {0xFF, 0x90, 0x80, 0x04, 0x00, 0x00};
static const UInt8 kGetUbmTileModeCallPatched[] = {0xFF, 0x90, 0xA8, 0x04, 0x00, 0x00};

// Mismatched `writeWaitForRenderingPipe` virtual call in `writeUpdateFrameBufferOffsetCommands`.
static const UInt8 kWriteWaitForRenderingPipeCallOriginal[] = {0xFF, 0x90, 0xB8, 0x02, 0x00, 0x00, 0x89, 0x45, 0xAC};
static const UInt8 kWriteWaitForRenderingPipeCallPatched[] = {0xFF, 0x90, 0xB0, 0x02, 0x00, 0x00, 0x89, 0x45, 0xAC};

// Mismatched `dummyWPTRUpdateDiag` virtual call in `WPTRDiagnostic`.
static const UInt8 kDummyWPTRUpdateDiagCallOriginal[] = {0x48, 0x8B, 0x80, 0x50, 0x02, 0x00, 0x00};
static const UInt8 kDummyWPTRUpdateDiagCallPatched[] = {0x48, 0x8B, 0x80, 0x48, 0x02, 0x00, 0x00};

// Mismatched `getPM4CommandsUtility` virtual call in `init`.
static const UInt8 kGetPM4CommandUtilityCallOriginal[] = {0xFF, 0x90, 0xA0, 0x03, 0x00, 0x00};
static const UInt8 kGetPM4CommandUtilityCallPatched[] = {0xFF, 0x90, 0x98, 0x03, 0x00, 0x00};

// Mismatched `getChannelDoorbellOffset` virtual call in `allocateMemoryResources`.
static const UInt8 kGetChannelDoorbellOffsetCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x88, 0x03, 0x00, 0x00};
static const UInt8 kGetChannelDoorbellOffsetCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x80, 0x03, 0x00, 0x00};

// Mismatched `getDoorbellMemoryBaseAddress` virtual call in `allocateMemoryResources`.
static const UInt8 kGetDoorbellMemoryBaseAddressCallOriginal[] = {0xFF, 0x90, 0x80, 0x03, 0x00, 0x00};
static const UInt8 kGetDoorbellMemoryBaseAddressCallPatched[] = {0xFF, 0x90, 0x78, 0x03, 0x00, 0x00};

// Mismatched `updateUtilizationStatisticsCounter` virtual calls.
static const UInt8 kUpdateUtilizationStatisticsCounterCallOriginal[] = {0x41, 0xFF, 0x90, 0xE0, 0x03, 0x00, 0x00};
static const UInt8 kUpdateUtilizationStatisticsCounterCallPatched[] = {0x41, 0xFF, 0x90, 0xD8, 0x03, 0x00, 0x00};

// Mismatched `dumpASICHangState` virtual calls in `submitCommandBuffer`.
static const UInt8 kDumpASICHangStateCallOriginal[] = {0xFF, 0x90, 0xA8, 0x03, 0x00, 0x00};
static const UInt8 kDumpASICHangStateCallPatched[] = {0xFF, 0x90, 0xA0, 0x03, 0x00, 0x00};

// Mismatched `registerChannel` virtual call in `init`.
static const UInt8 kRegisterChannelCallOriginal[] = {0x4C, 0x89, 0xEE, 0xFF, 0x90, 0x28, 0x03, 0x00, 0x00};
static const UInt8 kRegisterChannelCallPatched[] = {0x4C, 0x89, 0xEE, 0xFF, 0x90, 0x20, 0x03, 0x00, 0x00};

// Mismatched `disableGfxOff` virtual calls.
static const UInt8 kDisableGfxOffCallOriginal[] = {0xFF, 0x90, 0x00, 0x04, 0x00, 0x00};
static const UInt8 kDisableGfxOffCallPatched[] = {0xFF, 0x90, 0xF8, 0x03, 0x00, 0x00};

// Mismatched `enableGfxOff` virtual calls.
static const UInt8 kEnableGfxOffCallOriginal[] = {0xFF, 0x90, 0x08, 0x04, 0x00, 0x00};
static const UInt8 kEnableGfxOffCallPatched[] = {0xFF, 0x90, 0x00, 0x04, 0x00, 0x00};

// Mismatched `getHWMemory` virtual calls.
static const UInt8 kGetHWMemoryCallOriginal[] = {0x18, 0x48, 0x8B, 0x07, 0xFF, 0x90, 0xE0, 0x02, 0x00, 0x00, 0x48};
static const UInt8 kGetHWMemoryCallPatched[] = {0x18, 0x48, 0x8B, 0x07, 0xFF, 0x90, 0xD8, 0x02, 0x00, 0x00, 0x48};

// Mismatched `getHWGart` virtual calls.
static const UInt8 kGetHWGartCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xE8, 0x02, 0x00, 0x00};
static const UInt8 kGetHWGartCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0xE0, 0x02, 0x00, 0x00};

// Mismatched `getChannelCount` virtual calls.
static const UInt8 kGetChannelCountCallOriginal[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x38, 0x03, 0x00, 0x00};
static const UInt8 kGetChannelCountCallPatched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x30, 0x03, 0x00, 0x00};

// Mismatched `flushSystemCaches` virtual calls.
static const UInt8 kFlushSystemCachesCallOriginal[] = {0xFF, 0x90, 0xA8, 0x04, 0x00, 0x00};
static const UInt8 kFlushSystemCachesCallPatched[] = {0xFF, 0x90, 0xD0, 0x04, 0x00, 0x00};

// Mismatched `getIOPCIDevice` virtual calls.
static const UInt8 kGetIOPCIDeviceCallOriginal[] = {0xFF, 0x90, 0x90, 0x03, 0x00, 0x00};
static const UInt8 kGetIOPCIDeviceCallPatched[] = {0xFF, 0x90, 0x88, 0x03, 0x00, 0x00};

// Mismatched `getHWRegisters` virtual calls.
static const UInt8 kGetHWRegistersCallOriginal[] = {0x40, 0x80, 0x00, 0xFF, 0x90, 0xD8, 0x02, 0x00, 0x00};
static const UInt8 kGetHWRegistersCallOriginalMask[] = {0xF0, 0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 kGetHWRegistersCallPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x00, 0x00, 0x00};
static const UInt8 kGetHWRegistersCallPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00};

// Mismatched `getChannelWriteBackFrameAddr` virtual calls.
static const UInt8 kGetChannelWriteBackFrameAddrCallOriginal[] = {0xFF, 0x90, 0x50, 0x03, 0x00, 0x00, 0x40};
static const UInt8 kGetChannelWriteBackFrameAddrCallOriginalMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0};
static const UInt8 kGetChannelWriteBackFrameAddrCallPatched[] = {0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kGetChannelWriteBackFrameAddrCallPatchedMask[] = {0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};

// Mismatched `getChannelWriteBackFrameOffset` virtual calls.
static const UInt8 kGetChannelWriteBackFrameOffsetCall1Original[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x48, 0x03, 0x00,
    0x00};
static const UInt8 kGetChannelWriteBackFrameOffsetCall1Patched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x40, 0x03, 0x00,
    0x00};

// TODO: Provide an explanation here.
static const UInt8 kGetChannelWriteBackFrameOffsetCall2Original[] = {0x89, 0xDE, 0xFF, 0x90, 0x48, 0x03, 0x00, 0x00};
static const UInt8 kGetChannelWriteBackFrameOffsetCall2Patched[] = {0x89, 0xDE, 0xFF, 0x90, 0x40, 0x03, 0x00, 0x00};

// Mismatched `getHWChannel` virtual calls.
static const UInt8 kGetHWChannelCall1Original[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x20, 0x03, 0x00, 0x00, 0x48, 0x85,
    0xC0};
static const UInt8 kGetHWChannelCall1Patched[] = {0x48, 0x8B, 0x07, 0xFF, 0x90, 0x18, 0x03, 0x00, 0x00, 0x48, 0x85,
    0xC0};

// TODO: Provide an explanation here.
static const UInt8 kGetHWChannelCall2Original[] = {0x31, 0xD2, 0xFF, 0x90, 0x18, 0x03, 0x00, 0x00};
static const UInt8 kGetHWChannelCall2Patched[] = {0x31, 0xD2, 0xFF, 0x90, 0x10, 0x03, 0x00, 0x00};

// TODO: Provide an explanation here.
static const UInt8 kGetHWChannelCall3Original[] = {0x00, 0x00, 0x00, 0xFF, 0x90, 0x18, 0x03, 0x00, 0x00};
static const UInt8 kGetHWChannelCall3Patched[] = {0x00, 0x00, 0x00, 0xFF, 0x90, 0x10, 0x03, 0x00, 0x00};

// Mismatched `getHWAlignManager` virtual calls.
static const UInt8 kGetHWAlignManagerCall1Original[] = {0x48, 0x80, 0x00, 0xFF, 0x90, 0x00, 0x03, 0x00, 0x00};
static const UInt8 kGetHWAlignManagerCall1OriginalMask[] = {0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 kGetHWAlignManagerCall1Patched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x02, 0x00, 0x00};
static const UInt8 kGetHWAlignManagerCall1PatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};

// TODO: Provide an explanation here.
static const UInt8 kGetHWAlignManagerCall2Original[] = {0x49, 0x89, 0xD4, 0xFF, 0x90, 0x00, 0x03, 0x00, 0x00};
static const UInt8 kGetHWAlignManagerCall2Patched[] = {0x49, 0x89, 0xD4, 0xFF, 0x90, 0xF8, 0x02, 0x00, 0x00};

// Mismatched `getHWEngine` virtual calls.
static const UInt8 kGetHWEngineCallOriginal[] = {0x00, 0x00, 0x00, 0xFF, 0x90, 0x10, 0x03, 0x00, 0x00};
static const UInt8 kGetHWEngineCallPatched[] = {0x00, 0x00, 0x00, 0xFF, 0x90, 0x08, 0x03, 0x00, 0x00};

// Mismatched `getAMDHWHandler` virtual calls.
static const UInt8 kGetAMDHWHandlerCallOriginal[] = {0xFF, 0x90, 0xD0, 0x02, 0x00, 0x00};
static const UInt8 kGetAMDHWHandlerCallPatched[] = {0xFF, 0x90, 0xC8, 0x02, 0x00, 0x00};

static const HWAlignVTableFix<2> FillUBMSurfaceVTFix {
    {0x1B8, 0x218},
    {1, 1},
};

static const HWAlignVTableFix<3> ConfigureDisplayVTFix {
    {0x1B8, 0x200, 0x218},
    {2, 2, 2},
};

static const HWAlignVTableFix<4> GetDisplayInfoVTFix {
    {0x128, 0x130, 0x138, 0x1D0},
    {1, 2, 2, 4},
};

static const HWAlignVTableFix<5> AllocateScanoutFBVTFix {
    {0x130, 0x138, 0x190, 0x1B0, 0x218},
    {1, 1, 1, 1, 1},
};
