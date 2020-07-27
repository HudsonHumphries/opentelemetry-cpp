#pragma once
 
#include "opentelemetry/sdk/metrics/record.h"
#include "opentelemetry/sdk/metrics/exporter.h"
#include "opentelemetry/sdk/metrics/aggregator/gauge_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/exact_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/histogram_aggregator.h"
#include <iostream>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdkmetrics = opentelemetry::sdk::metrics;

namespace exporter
{
namespace metrics
{

/**
 * The OStreamMetricsExporter exports record data through an ostream
 */ 
class OStreamMetricsExporter final : public sdkmetrics::MetricsExporter
{
public:

/**
 * Create an OStreamMetricsExporter. This constructor takes in a reference to an ostream that the
 * export() function will send span data into.
 * The default ostream is set to stdout
 */
explicit OStreamMetricsExporter(std::ostream &sout = std::cout) noexcept;

sdkmetrics::ExportResult Export(
    std::vector<sdkmetrics::Record> &records) noexcept override;

private:
std::ostream &sout_;


template <typename T>
void PrintAggregatorVariant(sdkmetrics::AggregatorVariant value)
{
  auto agg = nostd::get<nostd::shared_ptr<sdkmetrics::Aggregator<T>>>(value);
  auto aggKind = agg->get_aggregator_kind();

  if(aggKind == sdkmetrics::AggregatorKind::Counter)
  {
    sout_ << "\n  sum         : " << agg->get_checkpoint()[0];
  }
  else if(aggKind == sdkmetrics::AggregatorKind::MinMaxSumCount)
  {
    auto mmsc = agg->get_checkpoint();
    sout_ << "\n  min         : " << mmsc[0]
          << "\n  max         : " << mmsc[1]
          << "\n  sum         : " << mmsc[2]
          << "\n  count       : " << mmsc[3];
  }
  else if(aggKind == sdkmetrics::AggregatorKind::Gauge)
  {
    auto timestamp = agg->get_checkpoint_timestamp();

    sout_ << "\n  last value  : " << agg->get_checkpoint()[0]
          << "\n  timestamp   : " << std::to_string(timestamp.time_since_epoch().count());
  }
  else if(aggKind == sdkmetrics::AggregatorKind::Exact)
  {
    // TODO: Find better way to print quantiles
    if(agg->get_quant_estimation())
    {
      sout_ << "\n  quantiles   : "
            << "[0: "  << agg->get_quantiles(0) << ", "
            << ".25: " << agg->get_quantiles(.25) << ", "
            << ".50: " << agg->get_quantiles(.50) << ", "
            << ".75: " << agg->get_quantiles(.75) << ", "
            << "1: "   << agg->get_quantiles(1) << ']';
    }
    else
    {
      auto vec = agg->get_checkpoint();
      int size = vec.size();
      int i = 1;

      sout_ << "\n  values      : "
            <<'[';

      for(auto val : vec)
      {
        sout_ << val;
        if(i != size)
          sout_ << ", ";
        i++;
      }
      sout_ << ']';
    }


  }
  else if(aggKind == sdkmetrics::AggregatorKind::Histogram)
  {
    auto boundaries = agg->get_boundaries();
    auto counts = agg->get_counts();

    int boundaries_size = boundaries.size();
    int counts_size = counts.size();

    sout_ << "\n  buckets     : "
          << '[';

    for(int i = 0; i < boundaries_size; i++)
    {
      sout_ << boundaries[i];

      if(i != boundaries_size-1)
        sout_ << ", ";
    }
    sout_ << ']';

    sout_ << "\n  counts      : "
          << '[';
    for(int i = 0; i < counts_size; i++)
    {
      sout_ << counts[i];

      if(i != counts_size-1)
        sout_ << ", ";
    }
    sout_ << ']';
  }
  else if(aggKind == sdkmetrics::AggregatorKind::Sketch)
  {
    // TODO: Waiting for Sketch to be finished
    auto boundaries = agg->get_boundaries();
    auto counts = agg->get_counts();

    int boundaries_size = boundaries.size();
    int counts_size = counts.size();

    sout_ << "\n  buckets     : "
          << '[';

    for(int i = 0; i < boundaries_size; i++)
    {
      sout_ << boundaries[i];

      if(i != boundaries_size-1)
        sout_ << ", ";
    }
    sout_ << ']';

    sout_ << "\n  counts      : "
          << '[';
    for(int i = 0; i < counts_size; i++)
    {
      sout_ << counts[i];

      if(i != counts_size-1)
        sout_ << ", ";
    }
    sout_ << ']';
  }
}

};
}
}
OPENTELEMETRY_END_NAMESPACE