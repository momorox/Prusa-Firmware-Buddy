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

#include "../../../inc/MarlinConfig.h"

#if HAS_STEALTHCHOP

#include "../../gcode.h"
#include "../../../feature/tmc_util.h"
#include "../../../module/stepper/indirection.h"

template<typename TMC>
void tmc_say_stealth_status(TMC &st) {
  st.printLabel();
  SERIAL_ECHOPGM(" driver mode:\t");
  serialprintPGM(st.get_stealthChop_status() ? PSTR("stealthChop") : PSTR("spreadCycle"));
  SERIAL_EOL();
}
template<typename TMC>
void tmc_set_stealthChop(TMC &st, const bool enable) {
  st.stored.stealthChop_enabled = enable;
  st.refresh_stepping_mode();
}

static void set_stealth_status(const bool enable, const int8_t target_extruder) {

  #if AXIS_HAS_STEALTHCHOP(X) || AXIS_HAS_STEALTHCHOP(X2) || AXIS_HAS_STEALTHCHOP(Y) || AXIS_HAS_STEALTHCHOP(Y2) || AXIS_HAS_STEALTHCHOP(Z) || AXIS_HAS_STEALTHCHOP(Z2) || AXIS_HAS_STEALTHCHOP(Z3)
    const uint8_t index = parser.byteval('I');
  #endif
  #if defined(DEFAULT_MAX_FEEDRATE_STEALTHCHOP)
    constexpr feedRate_t DMF[] = DEFAULT_MAX_FEEDRATE;
    constexpr feedRate_t DMFS[] = DEFAULT_MAX_FEEDRATE_STEALTHCHOP;
    #define TMC_SET_STEALTH(Q, I) do{ \
      tmc_set_stealthChop(stepper##Q, enable); \
      planner.settings.max_feedrate_mm_s[I] = enable ? DMFS[I] : DMF[I]; \
      } while(0)
  #else
    #define TMC_SET_STEALTH(Q, I) tmc_set_stealthChop(stepper##Q, enable);
  #endif

  LOOP_XYZE(i) if (parser.seen(axis_codes[i])) {
    switch (i) {
      case X_AXIS:
        #if AXIS_HAS_STEALTHCHOP(X)
          if (index == 0) TMC_SET_STEALTH(X, i);
        #endif
        #if AXIS_HAS_STEALTHCHOP(X2)
          if (index == 1) TMC_SET_STEALTH(X2, i);
        #endif
        break;
      case Y_AXIS:
        #if AXIS_HAS_STEALTHCHOP(Y)
          if (index == 0) TMC_SET_STEALTH(Y, i);
        #endif
        #if AXIS_HAS_STEALTHCHOP(Y2)
          if (index == 1) TMC_SET_STEALTH(Y2, i);
        #endif
        break;
      case Z_AXIS:
        #if AXIS_HAS_STEALTHCHOP(Z)
          if (index == 0) TMC_SET_STEALTH(Z, i);
        #endif
        #if AXIS_HAS_STEALTHCHOP(Z2)
          if (index == 1) TMC_SET_STEALTH(Z2, i);
        #endif
        #if AXIS_HAS_STEALTHCHOP(Z3)
          if (index == 2) TMC_SET_STEALTH(Z3, i);
        #endif
        break;
      case E_AXIS: {
        if (target_extruder < 0) return;
        switch (target_extruder) {
          #if AXIS_HAS_STEALTHCHOP(E0)
            case 0: TMC_SET_STEALTH(E0, i); break;
          #endif
          #if AXIS_HAS_STEALTHCHOP(E1)
            case 1: TMC_SET_STEALTH(E1, i); break;
          #endif
          #if AXIS_HAS_STEALTHCHOP(E2)
            case 2: TMC_SET_STEALTH(E2, i); break;
          #endif
          #if AXIS_HAS_STEALTHCHOP(E3)
            case 3: TMC_SET_STEALTH(E3, i); break;
          #endif
          #if AXIS_HAS_STEALTHCHOP(E4)
            case 4: TMC_SET_STEALTH(E4, i); break;
          #endif
          #if AXIS_HAS_STEALTHCHOP(E5)
            case 5: TMC_SET_STEALTH(E5, i); break;
          #endif
        }
      } break;
    }
  }
  #undef TMC_SET_STEALTH
}

static void say_stealth_status() {
  #define TMC_SAY_STEALTH_STATUS(Q) tmc_say_stealth_status(stepper##Q)

  #if AXIS_HAS_STEALTHCHOP(X)
    TMC_SAY_STEALTH_STATUS(X);
  #endif
  #if AXIS_HAS_STEALTHCHOP(X2)
    TMC_SAY_STEALTH_STATUS(X2);
  #endif
  #if AXIS_HAS_STEALTHCHOP(Y)
    TMC_SAY_STEALTH_STATUS(Y);
  #endif
  #if AXIS_HAS_STEALTHCHOP(Y2)
    TMC_SAY_STEALTH_STATUS(Y2);
  #endif
  #if AXIS_HAS_STEALTHCHOP(Z)
    TMC_SAY_STEALTH_STATUS(Z);
  #endif
  #if AXIS_HAS_STEALTHCHOP(Z2)
    TMC_SAY_STEALTH_STATUS(Z2);
  #endif
  #if AXIS_HAS_STEALTHCHOP(Z3)
    TMC_SAY_STEALTH_STATUS(Z3);
  #endif
  #if AXIS_HAS_STEALTHCHOP(E0)
    TMC_SAY_STEALTH_STATUS(E0);
  #endif
  #if AXIS_HAS_STEALTHCHOP(E1)
    TMC_SAY_STEALTH_STATUS(E1);
  #endif
  #if AXIS_HAS_STEALTHCHOP(E2)
    TMC_SAY_STEALTH_STATUS(E2);
  #endif
  #if AXIS_HAS_STEALTHCHOP(E3)
    TMC_SAY_STEALTH_STATUS(E3);
  #endif
  #if AXIS_HAS_STEALTHCHOP(E4)
    TMC_SAY_STEALTH_STATUS(E4);
  #endif
  #if AXIS_HAS_STEALTHCHOP(E5)
    TMC_SAY_STEALTH_STATUS(E5);
  #endif
}

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M569: Stepper driver control <a href="https://reprap.org/wiki/G-code#M569:_Stepper_driver_control">M569: Stepper driver control</a>
 *
 *#### Usage
 *
 *    M569 [ S | X | Y | Z | E ]
 *
 *#### Parameters
 *
 * - `S` - Switch
 *   - `0` - spreadCycle
 *   - `1` - stealthChop
 * - `X` - X axis
 * - `Y` - Y axis
 * - `Z` - Z axis
 * - `E` - E axis
 *
 * Without parameters prints the current  stealthChop status of all capable drivers.
 */
void GcodeSuite::M569() {
  if (parser.seen('S')){
    int8_t target_extruder = get_target_extruder_from_command();
    if (target_extruder < 0) return;
    set_stealth_status(parser.value_bool(), target_extruder);
  } else
    say_stealth_status();
}

/** @}*/

#endif // HAS_STEALTHCHOP
