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

#ifndef KVS_METRICS_H
#define KVS_METRICS_H

#include <fstream>
#include <iostream>

#ifdef METRICS
#include "medida/medida.h"
#endif

#include "kvs_metrics_config.h"

namespace famradixtree {

#ifdef METRICS
class KVSMetrics {
  public:
    KVSMetrics(KVSMetricsConfig &config) : config_(config) {}

    void Report() {
        medida::reporting::JsonReporter reporter(registry_);
        std::string report = reporter.Report();
        std::ofstream out(config_.output_path_);
        out << report;
        out.close();
    }

  protected:
    medida::MetricsRegistry registry_;
    KVSMetricsConfig config_;
};

#define METRIC_COUNTER_INC(metrics, counter)                                   \
    if (metrics) {                                                             \
        metrics->counter->inc();                                               \
    }

#define METRIC_HISTOGRAM_UPDATE(metrics, histogram, val)                       \
    if (metrics) {                                                             \
        metrics->histogram->Update(val);                                       \
    }

#else // METRICS

class KVSMetrics {
  public:
    KVSMetrics(KVSMetricsConfig &config) : config_(config) {}

    void Report() { /* do nothing */
    }

  protected:
    KVSMetricsConfig config_;
};

#define METRIC_COUNTER_INC(metrics, counter) (void)0;

#define METRIC_HISTOGRAM_UPDATE(metrics, histogram, val) (void)0;

#endif

} // namespace famradixtree

#endif
