#include "opentelemetry/sdk/metrics/exporter.h"
#include "opentelemetry/exporters/ostream/metrics_exporter.h"
#include "opentelemetry/sdk/metrics/record.h"
#include "opentelemetry/sdk/metrics/aggregator/counter_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/min_max_sum_count_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/gauge_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/exact_aggregator.h"


#include <iostream>
#include <gtest/gtest.h>

namespace sdkmetrics = opentelemetry::sdk::metrics;
namespace metrics_api = opentelemetry::metrics;
namespace nostd = opentelemetry::nostd;

TEST(OStreamMetricsExporter, PrintCounter)
{
  auto exporter = std::unique_ptr<sdkmetrics::MetricsExporter> (new
      opentelemetry::exporter::metrics::OStreamMetricsExporter);
  

  auto aggregator = nostd::shared_ptr<opentelemetry::sdk::metrics::Aggregator<double>> (new
      opentelemetry::sdk::metrics::CounterAggregator<double>(metrics_api::InstrumentKind::Counter));
  
  aggregator->update(5.5);
  aggregator->checkpoint();

  sdkmetrics::Record r("name", "description", "labels", aggregator);
  std::vector<sdkmetrics::Record> records;
  records.push_back(r);


  // Create stringstream to redirect to
  std::stringstream stdoutOutput;

  // Save cout's buffer here
  std::streambuf *sbuf = std::cout.rdbuf();

  // Redirect cout to our stringstream buffer
  std::cout.rdbuf(stdoutOutput.rdbuf());

  exporter->Export(records);

  std::cout.rdbuf(sbuf);

  std::string expectedOutput = 
  "{\n"
  "  name        : name\n"
  "  description : description\n"
  "  labels      : labels\n"
  "  sum         : 5.5\n"
  "}\n"; 

  ASSERT_EQ(stdoutOutput.str(),expectedOutput);
}


TEST(OStreamMetricsExporter, PrintMinMaxSumCount)
{
  auto exporter = std::unique_ptr<sdkmetrics::MetricsExporter> (new
      opentelemetry::exporter::metrics::OStreamMetricsExporter);
  
  auto aggregator = nostd::shared_ptr<opentelemetry::sdk::metrics::Aggregator<int>> (new
      opentelemetry::sdk::metrics::MinMaxSumCountAggregator<int>(metrics_api::InstrumentKind::Counter));
  
  aggregator->update(1);
  aggregator->update(2);
  aggregator->checkpoint();

  sdkmetrics::Record r("name", "description", "labels", aggregator);
  std::vector<sdkmetrics::Record> records;
  records.push_back(r);

  // Create stringstream to redirect to
  std::stringstream stdoutOutput;

  // Save cout's buffer here
  std::streambuf *sbuf = std::cout.rdbuf();

  // Redirect cout to our stringstream buffer
  std::cout.rdbuf(stdoutOutput.rdbuf());

  exporter->Export(records);

  std::cout.rdbuf(sbuf);

  std::string expectedOutput = 
  "{\n"
  "  name        : name\n"
  "  description : description\n"
  "  labels      : labels\n"
  "  min         : 1\n"
  "  max         : 2\n"
  "  sum         : 3\n"
  "  count       : 2\n"
  "}\n"; 

  ASSERT_EQ(stdoutOutput.str(),expectedOutput);
}

TEST(OStreamMetricsExporter, PrintGauge)
{
  auto exporter = std::unique_ptr<sdkmetrics::MetricsExporter> (new
      opentelemetry::exporter::metrics::OStreamMetricsExporter);
  
  auto aggregator = nostd::shared_ptr<opentelemetry::sdk::metrics::Aggregator<short>> (new
      opentelemetry::sdk::metrics::GaugeAggregator<short>(metrics_api::InstrumentKind::Counter));
  
  aggregator->update(1);
  aggregator->update(9);
  aggregator->checkpoint();

  sdkmetrics::Record r("name", "description", "labels", aggregator);
  std::vector<sdkmetrics::Record> records;
  records.push_back(r);

  // Create stringstream to redirect to
  std::stringstream stdoutOutput;

  // Save cout's buffer here
  std::streambuf *sbuf = std::cout.rdbuf();

  // Redirect cout to our stringstream buffer
  std::cout.rdbuf(stdoutOutput.rdbuf());

  exporter->Export(records);

  std::cout.rdbuf(sbuf);

  std::string expectedOutput = 
  "{\n"
  "  name        : name\n"
  "  description : description\n"
  "  labels      : labels\n"
  "  last value  : 9\n"
  "  timestamp   : " + std::to_string(aggregator->get_checkpoint_timestamp().time_since_epoch().count()) + "\n"
  "}\n"; 

  ASSERT_EQ(stdoutOutput.str(),expectedOutput);
}

TEST(OStreamMetricsExporter, PrintExact)
{
  auto exporter = std::unique_ptr<sdkmetrics::MetricsExporter> (new
      opentelemetry::exporter::metrics::OStreamMetricsExporter);
  
  auto aggregator = nostd::shared_ptr<opentelemetry::sdk::metrics::Aggregator<int>> (new
      opentelemetry::sdk::metrics::ExactAggregator<int>(metrics_api::InstrumentKind::Counter,true));

  auto aggregator2 = nostd::shared_ptr<opentelemetry::sdk::metrics::Aggregator<int>> (new
      opentelemetry::sdk::metrics::ExactAggregator<int>(metrics_api::InstrumentKind::Counter,false));
  
  for(int i = 0; i < 10; i++)
  {
    aggregator->update(i);
    aggregator2->update(i);
  }
  aggregator->checkpoint();
  aggregator2->checkpoint();

  sdkmetrics::Record r("name", "description", "labels", aggregator);
  sdkmetrics::Record r2("name", "description", "labels", aggregator2);  
  std::vector<sdkmetrics::Record> records;
  records.push_back(r);
  records.push_back(r2);

  // Create stringstream to redirect to
  std::stringstream stdoutOutput;

  // Save cout's buffer here
  std::streambuf *sbuf = std::cout.rdbuf();

  // Redirect cout to our stringstream buffer
  std::cout.rdbuf(stdoutOutput.rdbuf());

  exporter->Export(records);

  std::cout.rdbuf(sbuf);

  std::string expectedOutput = 
  "{\n"
  "  name        : name\n"
  "  description : description\n"
  "  labels      : labels\n"
  "  quantiles   : [0: 0, .25: 3, .50: 5, .75: 7, 1: 9]\n"
  "}\n"
  "{\n"
  "  name        : name\n"
  "  description : description\n"
  "  labels      : labels\n"
  "  values      : [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]\n"
  "}\n"; 

  ASSERT_EQ(stdoutOutput.str(),expectedOutput);
}

TEST(OStreamMetricsExporter, PrintHistogram)
{
  auto exporter = std::unique_ptr<sdkmetrics::MetricsExporter> (new
      opentelemetry::exporter::metrics::OStreamMetricsExporter);
  
  std::vector<double> boundaries{10,20,30,40,50};
  auto aggregator = nostd::shared_ptr<opentelemetry::sdk::metrics::Aggregator<int>> (new
      opentelemetry::sdk::metrics::HistogramAggregator<int>(metrics_api::InstrumentKind::Counter,boundaries));
  
  for(int i = 0; i < 60; i++)
  {
    aggregator->update(i);
  }
  aggregator->checkpoint();

  sdkmetrics::Record r("name", "description", "labels", aggregator);
  std::vector<sdkmetrics::Record> records;
  records.push_back(r);

  // Create stringstream to redirect to
  std::stringstream stdoutOutput;

  // Save cout's buffer here
  std::streambuf *sbuf = std::cout.rdbuf();

  // Redirect cout to our stringstream buffer
  std::cout.rdbuf(stdoutOutput.rdbuf());

  exporter->Export(records);

  std::cout.rdbuf(sbuf);

  std::string expectedOutput = 
  "{\n"
  "  name        : name\n"
  "  description : description\n"
  "  labels      : labels\n"
  "  buckets     : [10, 20, 30, 40, 50]\n"
  "  counts      : [10, 10, 10, 10, 10, 10]\n"
  "}\n"; 

  ASSERT_EQ(stdoutOutput.str(),expectedOutput);
}