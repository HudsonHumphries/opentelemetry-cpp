#pragma once
// Please refer to provider.h for documentation on how to obtain a Tracer object.
//
// This file is part of the internal implementation of OpenTelemetry. Nothing in this file should be
// used directly. Please refer to span.h and tracer.h for documentation on these interfaces.

#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/nostd/unique_ptr.h"
#include "opentelemetry/metrics/meter.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/version.h"

#include <memory>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace meter
{
/**
 * No-op implementation of Span. This class should not be used directly.
 */
class NoopInstrument
{
public:
  NoopInstrument(const std::shared_ptr<Meter> &meter) noexcept : meter_{meter} {}


  Meter &meter() const noexcept override { return *meter_; }

private:
  std::shared_ptr<Meter> meter_;
};

/**
 * No-op implementation of Tracer.
 */
class NoopMeter final : public Meter, public std::enable_shared_from_this<NoopMeter>
{
public:
    NoopMeter() = default;

};

/**
 * No-op implementation of a TracerProvider.
 */
class NoopMeterProvider final : public opentelemetry::meter::MeterProvider
{
public:
  NoopMeterProvider()
      : meter_{nostd::shared_ptr<opentelemetry::meter::MeterProvider>(
            new opentelemetry::meter::MeterProvider)}
  {}

  nostd::shared_ptr<opentelemetry::meter::Meter> GetMeter(
      nostd::string_view library_name,
      nostd::string_view library_version) override
  {
    return meter_;
  }

private:
  nostd::shared_ptr<opentelemetry::meter::Meter> meter_;
};
}  // namespace meter
OPENTELEMETRY_END_NAMESPACE