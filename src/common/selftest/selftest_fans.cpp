#include <selftest_fans.hpp>

#include <logging/log.hpp>

LOG_COMPONENT_REF(Selftest);

using namespace fan_selftest;

void FanHandler::evaluate() {
    calculate_avg_rpm();
    bool passed = is_rpm_within_bounds(avg_rpm);
    log_info(Selftest, "%s fan %c: RPM %u %s range (%u - %u)",
        fan_type_names[fan_type],
        '0' + desc_num,
        avg_rpm,
        passed ? "in" : "out of",
        fan_range.rpm_min,
        fan_range.rpm_max);
    if (!passed) {
        failed = true;
    }
}

uint16_t FanHandler::calculate_avg_rpm() {
    avg_rpm = sample_count ? (sample_sum / sample_count) : 0;
    return avg_rpm;
}

void FanHandler::reset_samples() {
    sample_count = 0;
    sample_sum = 0;
    avg_rpm = 0;
}

TestResult FanHandler::test_result() const {
    return is_failed() ? TestResult_Failed : TestResult_Passed;
}

CommonFanHandler::CommonFanHandler(const FanType type, uint8_t tool_nr, FanRPMRange fan_range, CFanCtlCommon *fan_control)
    : FanHandler(type, fan_range, tool_nr)
    , fan(fan_control) {
    fan->enterSelftestMode();
}

CommonFanHandler::~CommonFanHandler() {
    fan->exitSelftestMode();
}

void CommonFanHandler::set_pwm(uint8_t pwm) {
    fan->selftestSetPWM(pwm);
}

void CommonFanHandler::record_sample() {
    sample_count++;
    sample_sum += fan->getActualRPM();
}
