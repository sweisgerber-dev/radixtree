/*
 *  (c) Copyright 2016-2017 Hewlett Packard Enterprise Development Company LP.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As an exception, the copyright holders of this Library grant you permission
 *  to (i) compile an Application with the Library, and (ii) distribute the
 *  Application containing code generated by the Library and added to the
 *  Application during this compilation process under terms of your choice,
 *  provided you also meet the terms and conditions of the Application license.
 *
 */

#include "kvs_metrics_config.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <sstream>

namespace famradixtree {

bool env_bool(const char *env_name, bool default_val = false) {
    const char *env_p = getenv(env_name);
    if (!env_p) {
        return default_val;
    }

    std::string env_val(env_p);
    std::transform(env_val.begin(), env_val.end(), env_val.begin(), ::tolower);
    if (env_val == "true" || env_val == "set" || env_val == "enabled") {
        return true;
    }

    return false;
}

std::string env_val(const char *env_name, std::string default_val) {
    const char *env_p = getenv(env_name);
    if (!env_p) {
        return default_val;
    }

    return std::string(env_p);
}

std::string env_val(const char *env_name, const char *default_val) {
    return env_val(env_name, std::string(default_val));
}

template <typename T> T env_val(const char *env_name, T default_val) {
    const char *env_p = getenv(env_name);
    if (!env_p) {
        return default_val;
    }

    std::stringstream ss(std::string(env_p));
    T val;
    ss >> val;
    return val;
}

KVSMetricsConfig::KVSMetricsConfig() {
    enabled_ = env_bool("KVS_METRICS_ENABLED");
    output_path_ = env_val("KVS_METRICS_OUTPUT_PATH", "kvs_metrics.json");
}

} // namespace famradixtree
