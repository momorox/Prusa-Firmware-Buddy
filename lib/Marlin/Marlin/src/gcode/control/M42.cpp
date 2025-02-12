/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "../gcode.h"
#include "../../Marlin.h" // for pin_is_protected
#include "../../inc/MarlinConfig.h"

#if FAN_COUNT > 0
  #include "../../module/temperature.h"
#endif

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M42: Change pin status via GCode <a href="https://reprap.org/wiki/G-code#M42:_Switch_I.2FO_pin">M42: Switch I/O pin</a>
 *
 *#### Usage
 *
 *    M42 [ P | S | I ]
 *
 *#### Parameters
 *
 *  - `P` - Pin number
 *  - `S` - Pin status (from 0 - 255)
 *  - `I` - Flag to ignore pin protection
 *
 */
void GcodeSuite::M42() {
  if (!parser.seenval('S')) return;
  const byte pin_status = parser.value_byte();

  const int pin_index = PARSED_PIN_INDEX('P', GET_PIN_MAP_INDEX(LED_PIN));
  if (pin_index < 0) return;

  const pin_t pin = GET_PIN_MAP_PIN(pin_index);

  if (!parser.boolval('I') && pin_is_protected(pin)) return protected_pin_err();

  pinMode(pin, OUTPUT);
  extDigitalWrite(pin, pin_status);
  analogWrite(pin, pin_status);

  #if FAN_COUNT > 0
    switch (pin) {
      #if HAS_FAN0
        case FAN0_PIN: thermalManager.fan_speed[0] = pin_status; break;
      #endif
      #if HAS_FAN1
        case FAN1_PIN: thermalManager.fan_speed[1] = pin_status; break;
      #endif
      #if HAS_FAN2
        case FAN2_PIN: thermalManager.fan_speed[2] = pin_status; break;
      #endif
    }
  #endif
}

/** @}*/
