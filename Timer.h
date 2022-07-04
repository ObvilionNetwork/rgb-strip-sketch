#pragma once
struct Timer {
  uint32_t started_at, _period;
  bool _started;

  // Указать период, опционально статус (запущен/не запущен)
  Timer (uint32_t period, bool started = true) {
    _period = period;
    _started = started; 
    
    if (started) {
      started_at = millis();
    }
  }

  void setPeriod(uint32_t period) {
    _period = period;
  }

  void restart() {
    started_at = millis();
    _started = true;
  }

  bool elapsed() {
    return (!_started || check());
  }

  bool period() {
    if (_started && check()) {
      restart();
      return 1;
    } return 0;
  }

  bool check() {
    return millis() - started_at >= _period;
  }
};
