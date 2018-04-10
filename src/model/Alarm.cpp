/*
 * Copyright 2018 WolkAbout Technology s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "model/Alarm.h"

#include <string>
#include <utility>

namespace wolkabout
{
Alarm::Alarm() : Reading("", "") {}

Alarm::Alarm(std::string value, std::string reference, unsigned long long int rtc)
: Reading(std::move(value), std::move(reference), rtc)
{
}

void Alarm::acceptVisit(ReadingVisitor& visitor)
{
    visitor.visit(*this);
}
}    // namespace wolkabout