#pragma once

#include "prometheus/registry.h"
#include "util/config.hpp"

namespace spt::http::service
{
  struct RegistryManager
  {
    static RegistryManager& instance()
    {
      static RegistryManager instance;
      return instance;
    }

    static prometheus::Summary::Quantiles quantiles()
    {
      return {{0.5, 0.05}, {0.9, 0.01}, {0.99, 0.001}};
    }

    RegistryManager(const RegistryManager&) = delete;
    RegistryManager(RegistryManager&&) = delete;
    RegistryManager& operator=(const RegistryManager&) = delete;
    RegistryManager& operator=(RegistryManager&&) = delete;

    prometheus::Registry registry;
    prometheus::Family<prometheus::Summary>* summaries = nullptr;
    prometheus::Family<prometheus::Counter>* counters = nullptr;

  private:
    RegistryManager()
    {
      const auto env = environment();

      summaries = &prometheus::BuildSummary().
          Name("iot_service_sensor_data_time").
          Help("How long it took to process the request in milliseconds").
          Labels({{"environment", env}, {"system", "iot"}, {"subsystem", "ingress"}}).
          Register(registry);

      counters = &prometheus::BuildCounter().
          Name("iot_service_sensor_data_requests").
          Help("How many requests the service received").
          Labels({{"environment", env}, {"system", "iot"}, {"subsystem", "ingress"}}).
          Register(registry);
    }

    std::string environment()
    {
      auto& conf = util::Configuration::instance();
      auto opt = conf.get("/env");

      if (opt.has_value()) return opt.value();
      else
      {
        LOG_WARN << "No configuration for key: /env";
      }

      return std::string{"unknown"};
    }

    ~RegistryManager() = default;
  };
}