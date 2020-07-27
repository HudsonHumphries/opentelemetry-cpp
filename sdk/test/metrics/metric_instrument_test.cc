#include <gtest/gtest.h>
#include "opentelemetry/sdk/metrics/sync_instruments.h"
#include "opentelemetry/sdk/metrics/async_instruments.h"

#include <cstring>
#include <string>
#include <map>
#include <thread>
#include <memory>
#include <iostream>

namespace metrics_api = opentelemetry::metrics;


OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk
{
namespace metrics
{

TEST(Counter, InstrumentFunctions)
{
  Counter<int> alpha("enabled", "no description", "unitless", true);
  Counter<double> beta("not enabled", "some description", "units", false);

  EXPECT_EQ(static_cast<std::string>(alpha.GetName()), "enabled");
  EXPECT_EQ(static_cast<std::string>(alpha.GetDescription()), "no description");
  EXPECT_EQ(static_cast<std::string>(alpha.GetUnits()), "unitless");
    EXPECT_EQ(alpha.IsEnabled(), true);

  EXPECT_EQ(static_cast<std::string>(beta.GetName()), "not enabled");
  EXPECT_EQ(static_cast<std::string>(beta.GetDescription()), "some description");
  EXPECT_EQ(static_cast<std::string>(beta.GetUnits()), "units");
    EXPECT_EQ(beta.IsEnabled(), false);
}

TEST(Counter, Binding)
{
  Counter<int> alpha("test", "none", "unitless", true);

  std::map<std::string, std::string> labels = {{"key", "value"}};
  std::map<std::string, std::string> labels1 = {{"key1", "value1"}};
  std::map<std::string, std::string> labels2 = {{"key2", "value2"}, {"key3", "value3"}};
  std::map<std::string, std::string> labels3 = {{"key3", "value3"}, {"key2", "value2"}};


  auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};
    auto labelkv2 = trace::KeyValueIterableView<decltype(labels2)>{labels2};
    auto labelkv3 = trace::KeyValueIterableView<decltype(labels3)>{labels3};

  auto beta = alpha.bindCounter(labelkv);
  auto gamma = alpha.bindCounter(labelkv1);
  auto delta = alpha.bindCounter(labelkv1);
  auto epsilon = alpha.bindCounter(labelkv1);
  auto zeta = alpha.bindCounter(labelkv2);
  auto eta = alpha.bindCounter(labelkv3);

  EXPECT_EQ (beta->get_ref(), 1);
  EXPECT_EQ(gamma->get_ref(),3);
  EXPECT_EQ(eta->get_ref(),2);

  delta->unbind();
  gamma->unbind();
  epsilon->unbind();

  EXPECT_EQ(alpha.boundInstruments_[KvToString(labelkv1)]->get_ref(), 0);
  EXPECT_EQ(alpha.boundInstruments_.size(), 3);
}

TEST(Counter, getAggsandnewupdate)
{
  Counter<int> alpha("test", "none", "unitless", true);

  std::map<std::string, std::string> labels = {{"key", "value"}};
  std::map<std::string, std::string> labels1 = {{"key1", "value1"}};

    //labels 2 and 3 are actually the same
  std::map<std::string, std::string> labels2 = {{"key2", "value2"}, {"key3", "value3"}};
  std::map<std::string, std::string> labels3 = {{"key3", "value3"}, {"key2", "value2"}};

    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};
    auto labelkv2 = trace::KeyValueIterableView<decltype(labels2)>{labels2};
    auto labelkv3 = trace::KeyValueIterableView<decltype(labels3)>{labels3};

  auto beta = alpha.bindCounter(labelkv);
  auto gamma = alpha.bindCounter(labelkv1);
  auto delta = alpha.bindCounter(labelkv1);
  auto epsilon = alpha.bindCounter(labelkv1);
  auto zeta = alpha.bindCounter(labelkv2);
  auto eta = alpha.bindCounter(labelkv3);

  EXPECT_EQ (beta->get_ref(), 1);
  EXPECT_EQ(gamma->get_ref(),3);
  EXPECT_EQ(eta->get_ref(),2);

  delta->unbind();
  gamma->unbind();
  epsilon->unbind();

  EXPECT_EQ(alpha.boundInstruments_[KvToString(labelkv1)]->get_ref(), 0);
  EXPECT_EQ(alpha.boundInstruments_.size(), 3);

    auto theta = alpha.GetRecords();
    EXPECT_EQ(theta.size(),3);
    EXPECT_EQ(theta[0].GetName(), "test");
    EXPECT_EQ(theta[0].GetDescription(), "none");
    EXPECT_EQ(theta[0].GetLabels(), "{\"key2\":\"value2\",\"key3\":\"value3\"}");
    EXPECT_EQ(theta[1].GetLabels(), "{\"key1\":\"value1\"}");
}

void CounterCallback(std::shared_ptr<Counter<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->add(1, labels);
  }
}

TEST(Counter, StressAdd){
  std::shared_ptr<Counter<int>> alpha(new Counter<int>("test", "none", "unitless", true));

  std::map<std::string, std::string> labels = {{"key", "value"}};
  std::map<std::string, std::string> labels1 = {{"key1", "value1"}};

    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};

  std::thread first (CounterCallback, alpha, 100000, labelkv);
  std::thread second (CounterCallback, alpha, 100000, labelkv);
  std::thread third (CounterCallback, alpha, 300000, labelkv1);

  first.join();
  second.join();
  third.join();

    EXPECT_EQ(dynamic_cast<BoundCounter<int>*>(alpha->boundInstruments_[KvToString(labelkv)].get())->GetAggregator()->get_values()[0], 200000);
    EXPECT_EQ(dynamic_cast<BoundCounter<int>*>(alpha->boundInstruments_[KvToString(labelkv1)].get())->GetAggregator()->get_values()[0], 300000);
}

void UpDownCounterCallback(std::shared_ptr<UpDownCounter<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->add(1, labels);
  }
}

void NegUpDownCounterCallback(std::shared_ptr<UpDownCounter<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->add(-1, labels);
  }
}

TEST(IntUpDownCounter, StressAdd){
  std::shared_ptr<UpDownCounter<int>> alpha(new UpDownCounter<int>("test", "none", "unitless", true));

  std::map<std::string, std::string> labels = {{"key", "value"}};
  std::map<std::string, std::string> labels1 = {{"key1", "value1"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};

  std::thread first (UpDownCounterCallback, alpha, 123400, labelkv);     // spawn new threads that call the callback
  std::thread second (UpDownCounterCallback, alpha, 123400, labelkv);
  std::thread third (UpDownCounterCallback, alpha, 567800, labelkv1);
  std::thread fourth (NegUpDownCounterCallback, alpha, 123400, labelkv1); // negative values

  first.join();
  second.join();
  third.join();
  fourth.join();

    EXPECT_EQ(dynamic_cast<BoundUpDownCounter<int>*>(alpha->boundInstruments_[KvToString(labelkv)].get())->GetAggregator()->get_values()[0], 123400*2);
    EXPECT_EQ(dynamic_cast<BoundUpDownCounter<int>*>(alpha->boundInstruments_[KvToString(labelkv1)].get())->GetAggregator()->get_values()[0], 567800-123400);
}

void RecorderCallback(std::shared_ptr<ValueRecorder<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->record(i, labels);
  }
}

void NegRecorderCallback(std::shared_ptr<ValueRecorder<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->record(-i, labels);
  }
}

TEST(IntValueRecorder, StressRecord){
  std::shared_ptr<ValueRecorder<int>> alpha(new ValueRecorder<int>("test", "none", "unitless", true));

  std::map<std::string, std::string> labels = {{"key", "value"}};
  std::map<std::string, std::string> labels1 = {{"key1", "value1"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};

  std::thread first (RecorderCallback, alpha, 25, labelkv);     // spawn new threads that call the callback
  std::thread second (RecorderCallback, alpha, 50, labelkv);
  std::thread third (RecorderCallback, alpha, 25, labelkv1);
  std::thread fourth (NegRecorderCallback, alpha, 100, labelkv1); // negative values

  first.join();
  second.join();
  third.join();
  fourth.join();

    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv)].get())->GetAggregator()->get_values()[0], 0); // min
    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv)].get())->GetAggregator()->get_values()[1], 49); // max
    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv)].get())->GetAggregator()->get_values()[2], 1525); // sum
    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv)].get())->GetAggregator()->get_values()[3], 75); // count

    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv1)].get())->GetAggregator()->get_values()[0], -99); // min
    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv1)].get())->GetAggregator()->get_values()[1], 24); // max
    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv1)].get())->GetAggregator()->get_values()[2], -4650); // sum
    EXPECT_EQ(dynamic_cast<BoundValueRecorder<int>*>(alpha->boundInstruments_[KvToString(labelkv1)].get())->GetAggregator()->get_values()[3], 125); // count
}

void ObserverConstructorCallback(metrics_api::ObserverResult<int> result){
    std::map<std::string, std::string> labels = {{"key", "value"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    result.observe(1,labelkv);
}

TEST(ApiSdkConversion, async){
    nostd::shared_ptr<metrics_api::AsynchronousInstrument<int>> alpha = nostd::shared_ptr<metrics_api::AsynchronousInstrument<int>>(new ValueObserver<int>("ankit","none","unitles",true, &ObserverConstructorCallback));
    
    std::map<std::string, std::string> labels = {{"key587", "value264"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    
    alpha->observe(123456,labelkv);
    EXPECT_EQ(dynamic_cast<AsynchronousInstrument<int>*>(alpha.get())->GetRecords()[0].GetLabels(),"{\"key587\":\"value264\"}");
    
    alpha->observe(123456,labelkv);
    AggregatorVariant canCollect = dynamic_cast<AsynchronousInstrument<int>*>(alpha.get())->GetRecords()[0].GetAggregator();
    EXPECT_EQ(nostd::holds_alternative<nostd::shared_ptr<Aggregator<short>>>(canCollect), false);
    EXPECT_EQ(nostd::holds_alternative<nostd::shared_ptr<Aggregator<int>>>(canCollect), true);
    EXPECT_EQ(nostd::get<nostd::shared_ptr<Aggregator<int>>>(canCollect)->get_checkpoint()[0], 123456);
}

TEST(IntValueObserver, InstrumentFunctions)
{
    ValueObserver<int> alpha("enabled", "no description", "unitless", true, &ObserverConstructorCallback);
    std::map<std::string, std::string> labels = {{"key", "value"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};

    EXPECT_EQ(alpha.GetName(), "enabled");
    EXPECT_EQ(alpha.GetDescription(), "no description");
    EXPECT_EQ(alpha.GetUnits(), "unitless");
    EXPECT_EQ(alpha.IsEnabled(), true);
    EXPECT_EQ(alpha.GetKind(), metrics_api::InstrumentKind::ValueObserver);

    alpha.run();
    EXPECT_EQ(alpha.boundAggregators_[KvToString(labelkv)]->get_values()[0], 1); // min
}

void ObserverCallback(std::shared_ptr<ValueObserver<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->observe(i, labels);
  }
}

void NegObserverCallback(std::shared_ptr<ValueObserver<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->observe(-i, labels);
  }
}

TEST(IntValueObserver, StressObserve)
{
    std::shared_ptr<ValueObserver<int>> alpha(new ValueObserver<int>("enabled", "no description", "unitless", true, &ObserverConstructorCallback));

    std::map<std::string, std::string> labels = {{"key", "value"}};
    std::map<std::string, std::string> labels1 = {{"key1", "value1"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};

    std::thread first (ObserverCallback, alpha, 25, labelkv);     // spawn new threads that call the callback
    std::thread second (ObserverCallback, alpha, 50, labelkv);
    std::thread third (ObserverCallback, alpha, 25, labelkv1);
    std::thread fourth (NegObserverCallback, alpha, 100, labelkv1); // negative values

    first.join();
    second.join();
    third.join();
    fourth.join();

    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv)]->get_values()[0], 0); // min
    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv)]->get_values()[1], 49); // max
    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv)]->get_values()[2], 1525); // sum
    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv)]->get_values()[3], 75); // count

    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv1)]->get_values()[0], -99); // min
    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv1)]->get_values()[1], 24); // max
    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv1)]->get_values()[2], -4650); // sum
    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv1)]->get_values()[3], 125); // count
}

void SumObserverCallback(std::shared_ptr<SumObserver<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->observe(1, labels);
  }
}

TEST(IntSumObserver, StressObserve)
{
  std::shared_ptr<SumObserver<int>> alpha(new SumObserver<int>("test", "none", "unitless", true, &ObserverConstructorCallback));

  std::map<std::string, std::string> labels = {{"key", "value"}};
  std::map<std::string, std::string> labels1 = {{"key1", "value1"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};

  std::thread first (SumObserverCallback, alpha, 100000, labelkv);
  std::thread second (SumObserverCallback, alpha, 100000, labelkv);
  std::thread third (SumObserverCallback, alpha, 300000, labelkv1);

  first.join();
  second.join();
  third.join();

  EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv)]->get_values()[0], 200000);
  EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv1)]->get_values()[0], 300000);
}


void UpDownSumObserverCallback(std::shared_ptr<UpDownSumObserver<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->observe(1, labels);
  }
}

void NegUpDownSumObserverCallback(std::shared_ptr<UpDownSumObserver<int>> in, int freq, const trace::KeyValueIterable &labels){
  for (int i=0; i<freq; i++){
    in->observe(-1, labels);
  }
}

TEST(IntUpDownObserver, StressAdd){
  std::shared_ptr<UpDownSumObserver<int>> alpha(new UpDownSumObserver<int>("test", "none", "unitless", true, &ObserverConstructorCallback));

  std::map<std::string, std::string> labels = {{"key", "value"}};
  std::map<std::string, std::string> labels1 = {{"key1", "value1"}};
    auto labelkv = trace::KeyValueIterableView<decltype(labels)>{labels};
    auto labelkv1 = trace::KeyValueIterableView<decltype(labels1)>{labels1};

  std::thread first (UpDownSumObserverCallback, alpha, 123400, labelkv);     // spawn new threads that call the callback
  std::thread second (UpDownSumObserverCallback, alpha, 123400, labelkv);
  std::thread third (UpDownSumObserverCallback, alpha, 567800, labelkv1);
  std::thread fourth (NegUpDownSumObserverCallback, alpha, 123400, labelkv1); // negative values

  first.join();
  second.join();
  third.join();
  fourth.join();

    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv)]->get_values()[0], 123400*2);
    EXPECT_EQ(alpha->boundAggregators_[KvToString(labelkv1)]->get_values()[0],  567800-123400);
}



} //namespace sdk
}  // namespace metrics
OPENTELEMETRY_END_NAMESPACE