/*
 * Copyright (C) 2022-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include <string>
#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;

std::vector<std::string> ro_props_default_source_order = {
    "",
    "bootimage.",
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
    "vendor_dlkm.",
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

/*
 * Spoof build fingerprint and description
 * Workaround for passing snet
 */
void spoof_fp_desc() {
    std::string build_desc = "walleye-user 8.1.0 OPM1.171019.011 4448085 release-keys";
    std::string build_fingerprint = "google/walleye/walleye:8.1.0/OPM1.171019.011/4448085:user/release-keys";

    const auto set_ro_build_prop = [](const std::string &source,
                                      const std::string &prop, const std::string &value) {
        auto prop_name = "ro." + source + "build." + prop;
        OverrideProperty(prop_name.c_str(), value.c_str());
    };

    OverrideProperty("ro.build.description", build_desc.c_str());
    for (const auto &source : ro_props_default_source_order)
    {
        set_ro_build_prop(source, "fingerprint", build_fingerprint.c_str());
    }
}

/*
 * Only for read-only properties. Properties that can be wrote to more
 * than once should be set in a typical init script (e.g. init.oplus.hw.rc)
 * after the original property has been set.
 */
void vendor_load_properties() {
    auto prjname = std::stoi(GetProperty("ro.boot.prjname", "0"));

    switch (prjname) {
        // lunaa
        case 21603: // CN
            OverrideProperty("ro.product.product.model", "RMX3361");
            break;
        case 21675: // IN
            OverrideProperty("ro.product.product.model", "RMX3360");
            break;
        case 21676: // EU
            OverrideProperty("ro.product.product.model", "RMX3363");
            break;
        default:
            LOG(ERROR) << "Unexpected project name: " << prjname;
    }

    spoof_fp_desc();

    // SafetyNet workaround
    OverrideProperty("ro.boot.verifiedbootstate", "green");
}
