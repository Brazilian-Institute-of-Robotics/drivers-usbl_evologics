// Copyright (c) 2026, SENAI Cimatec
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//              http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdexcept>
#include <string>

namespace evologics_usbl_driver
{
struct ValidationError : std::runtime_error
{
  explicit ValidationError(std::string const & desc)
  : std::runtime_error(desc) {}
};

struct ParseError : std::runtime_error
{
  explicit ParseError(std::string const & desc)
  : std::runtime_error(desc) {}
};

struct DeviceError : std::runtime_error
{
  explicit DeviceError(std::string const & desc)
  : std::runtime_error(desc) {}
};

struct WrongInputValue : std::runtime_error
{
  explicit WrongInputValue(std::string const & desc)
  : std::runtime_error(desc) {}
};

struct InstantMessagingError : std::runtime_error
{
  explicit InstantMessagingError(std::string const & desc)
  : std::runtime_error(desc) {}
};

struct ModeError : std::runtime_error
{
  explicit ModeError(std::string const & desc)
  : std::runtime_error(desc) {}
};

struct BusyError : std::runtime_error
{
  explicit BusyError(std::string const & desc)
  : std::runtime_error(desc) {}
};

struct UnexpectedRawPacket : std::runtime_error
{
  explicit UnexpectedRawPacket(std::string const & desc)
  : std::runtime_error(desc) {}
};
}  // namespace evologics_usbl_driver
