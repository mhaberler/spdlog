// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>

#include <array>
#include <esp_log.h>

namespace spdlog {
namespace sinks {

/**
 * Sink which utilizes esp-idf's log library (https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html).
 *
 */
template<typename Mutex>
class esp32_sink : public base_sink<Mutex>
{
public:
    //
    esp32_sink()
        : syslog_levels_{{/* spdlog::level::trace      */ esp_log_level_t::ESP_LOG_VERBOSE,
              /* spdlog::level::debug      */ esp_log_level_t::ESP_LOG_DEBUG,
              /* spdlog::level::info       */ esp_log_level_t::ESP_LOG_INFO,
              /* spdlog::level::warn       */ esp_log_level_t::ESP_LOG_WARN,
              /* spdlog::level::err        */ esp_log_level_t::ESP_LOG_ERROR,
              /* spdlog::level::critical   */ esp_log_level_t::ESP_LOG_ERROR,
              /* spdlog::level::off        */ esp_log_level_t::ESP_LOG_NONE}}
    {}

protected:
    using levels_array = std::array<esp_log_level_t, 7>;
    levels_array syslog_levels_;

    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        esp_log_write(syslog_level(msg.level), msg.logger_name.data(), "%s", formatted.data());
    }

    esp_log_level_t syslog_level(level::level_enum l)
    {
        return syslog_levels_.at(static_cast<levels_array::size_type>(l));
    }

    void flush_() override {}
};

using esp32_sink_mt = esp32_sink<std::mutex>;
using esp32_sink_st = esp32_sink<details::null_mutex>;
} // namespace sinks

// Create and register an esp32 system logger
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> esp32_logger_mt(const std::string &logger_name)
{
    return Factory::template create<sinks::esp32_sink_mt>(logger_name);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> esp32_logger_st(const std::string &logger_name)
{
    return Factory::template create<sinks::esp32_sink_st>(logger_name);
}
} // namespace spdlog
