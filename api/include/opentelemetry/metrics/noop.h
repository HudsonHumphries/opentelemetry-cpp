#pragma once
// Please refer to provider.h for documentation on how to obtain a Meter object

// This file is part of the internal implementation of OpenTelemetry. Nothing in this file should be
// used directly. Please refer to meter.h for documentation on these interfaces.

#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/nostd/unique_ptr.h"
#include "opentelemetry/metrics/meter_provider.h"

#include <memory>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace meter
{

class NoopMeter 
{
public:
  // Meter

}

class NoopMeterProvider final : public opentelemetry::meter::MeterProvider
{
    
}
}