// Copyright © 2022-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5.
// See LICENSE for details.

#pragma once
#include <Headers/kern_patcher.hpp>
#include <Headers/kern_util.hpp>

class AppleGFXHDA {
    static AppleGFXHDA *callback;

    public:
    void init();
    bool processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t slide, size_t size);

    private:
    OSMetaClass *orgFunctionGroupTahiti {nullptr};
    OSMetaClass *orgWidget1002AAA0 {nullptr};
    mach_vm_address_t orgCreateAppleHDAFunctionGroup {0};
    mach_vm_address_t orgCreateAppleHDAWidget {0};

    static void *wrapCreateAppleHDAFunctionGroup(void *devId);
    static void *wrapCreateAppleHDAWidget(void *devId);
};
