#pragma once
#include "opentelemetry/version.h"
#include "opentelemetry/sdk/metrics/instrument.h"
#include "opentelemetry/sdk/metrics/record.h"
#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/nostd/type_traits.h"

#include <unordered_map>
#include <string>
#include <tuple>
#include <iostream>

namespace sdkmetrics = opentelemetry::sdk::metrics;

OPENTELEMETRY_BEGIN_NAMESPACE

namespace sdk
{
namespace metrics
{
typedef std::tuple<std::string, std::string, std::string,
          opentelemetry::metrics::InstrumentKind> instrument_label_tuple;

class MetricsProcessor
{
  MetricsProcessor(bool stateful)
  {
    stateful_ = stateful;
    batch_map_ =  {};
  } 

  std::vector<sdkmetrics::Record> CheckpointSelf() noexcept
  {
    std::vector<sdkmetrics::Record> metric_records;
    
    for(auto iter : batch_map_)
    {
      std::string name = std::get<0>(iter.first);
      std::string description = std::get<1>(iter.first);
      std::string labels = std::get<2>(iter.first);

      sdkmetrics::Record r(nostd::string_view(name), nostd::string_view(description), labels, iter.second);
      metric_records.push_back(r);
    }

    return metric_records;
  }

  void FinishedCollection() noexcept
  {
    if(!stateful_)
    {
      batch_map_ = {};
    }
  }

  virtual void process(sdkmetrics::Record record) noexcept = 0;

private:
  bool stateful_;
  std::unordered_map<instrument_label_tuple, sdkmetrics::AggregatorVariant> batch_map_;
};

}
}

OPENTELEMETRY_END_NAMESPACE