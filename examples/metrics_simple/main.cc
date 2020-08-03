#include "opentelemetry/sdk/metrics/controller.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/ungrouped_processor.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/metrics/provider.h"

namespace sdkmetrics = opentelemetry::sdk::metrics;
namespace nostd      = opentelemetry::nostd;
namespace trace      = opentelemetry::trace;

int main() {
  // Initalize and set the global MeterProvider
  auto provider = nostd::shared_ptr<metrics_api::MeterProvider>(new sdkmetrics::MeterProvider);
  opentelemetry::metrics::Provider::SetMeterProvider(provider);

  // Get the Meter from the MeterProvider
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("Test");
  
  // Create the controller with Stateless Metrics Processor
  sdkmetrics::PushController ControllerStateless(meter,
                        std::unique_ptr<sdkmetrics::MetricsExporter>(new opentelemetry::exporter::metrics::OStreamMetricsExporter),
                        std::shared_ptr<sdkmetrics::MetricsProcessor>(new opentelemetry::sdk::metrics::UngroupedMetricsProcessor(false)),
                        .05);

  // Create and instrument with labelset
  auto instr = meter->NewIntCounter("test","none","none",true);
  std::map<std::string, std::string> labels = {{"key", "value"}};
  auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
  
  // Start exporting from the Controller with Stateless Processor
  std::cout << "-----" << " Stateless Processor " << "-----" << std::endl;
  ControllerStateless.start();
  for (int i = 0; i < 20; i++)
  {
    instr->add(i, labelkv);
    usleep(.01*1000000);
  }
  ControllerStateless.stop(); 

  // Do the same thing for stateful to see the difference
  sdkmetrics::PushController ControllerStateful(meter,
                      std::unique_ptr<sdkmetrics::MetricsExporter>(new opentelemetry::exporter::metrics::OStreamMetricsExporter),
                      std::shared_ptr<sdkmetrics::MetricsProcessor>(new opentelemetry::sdk::metrics::UngroupedMetricsProcessor(true)),
                      .05);

  // Start exporting from the Controller with Stateful Processor     
  std::cout << "-----" << " Stateful Processor " << "-----" << std::endl;
  ControllerStateful.start();
  for (int i = 0; i < 20; i++)
  {
    instr->add(i, labelkv);
    usleep(.01*1000000);
  }
  ControllerStateful.stop(); 
}