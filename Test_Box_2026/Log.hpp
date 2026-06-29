// =================================== FUNCION DE LOGS  ============================================

#pragma once

#include <deque>

std::deque<String> eventLog;
const size_t maxLogSize = 20;

void addLogEvent(const String& event) {
  if (eventLog.size() >= maxLogSize) {
    eventLog.pop_front();  // Elimina el más viejo
  }
  eventLog.push_back(event);
}

