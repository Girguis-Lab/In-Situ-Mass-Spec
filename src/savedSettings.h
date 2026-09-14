// Operator settings that persist across reboots and power cycles.
//
// Defines the settings struct, its compiled-in defaults, the single global
// instance the rest of the firmware reads, and the routines that move it in
// and out of EEPROM. Settings are changed by serial command and are relied on
// at boot -- autostart in particular -- so an instrument keeps behaving the
// same way after an unattended reset. Header-only, and defines objects, so it
// is included exactly once (from includes.h).

#include <Arduino.h>
#include <PROMPLUS.h>
#include <DebugLog.h>
#include "includes.h"

// ISMS Settings are stored in EEPROM (redundantly via the PROMPLUS library) using a c++ struct.
// When the ISMS boots up, it will attempt to load the settings saved in EEPROM.
// When settings are changed, the full struct is saved to EEPROM so that the settings persist by calling save_settings().
// Writes are done using the PROM.put() function which only updated the bits that have changed,
// to keep the EEPROM wear to a minimum. In the unlikely event that all three copies stored in the eeprom by the PROMPLUS
// library have been worn out and settings do not save or load correctly, increment eeprom_settings_slot to move the
// struct to a fresh set of cells. Increment it by one, not by the size of the struct -- see below for why.

// Which slot of the EEPROM the settings struct occupies.
//
// This is an ARRAY INDEX, not a byte offset. PROM.get()/PROM.put() scale it by
// sizeof(saved_settings_t), and PROMPLUS then stores every logical byte three
// times for redundancy, so the physical address works out as:
//
//     physical = eeprom_settings_slot * sizeof(saved_settings_t) * 3
//
// Slot 50 with the current 16 byte struct therefore lands at physical bytes
// 2400-2447, not at byte 50. The consequence worth knowing: the physical
// address moves whenever the struct's size changes, and the AVR's EEPROM
// address register is only 12 bits, so an address past E2END wraps silently
// and corrupts an unrelated region while every write appears to succeed. The
// static_assert below the struct is what catches that at compile time.
constexpr size_t eeprom_settings_slot = 50;

// ISMS settings struct - contains all settings that should persist through
// reboots and power cycles - stored in EEPROM.
//
// !!! IMPORTANT: To change this struct follow these guidelines:
//  - If all you change is the names of existing struct fields, nothing else is
//    needed.
//  - If you do anythhing else to the struct (e.g. change types of existing
//    fields, add new fields, remove fields, change the order of fields, etc),
//    you must increment the struct_version field in the default_settings
//    variable below so that the system knows to reset to default settings
//    instead of reading a potentially incompatible/miss-alinged struct from
//    EEPROM.
//  - Make sure to set a default value in the default_settings variable below
//    for any relavant fields you add to the struct!
struct saved_settings_t
{
    // Metadata fields to determine if the saved settings are valid and compatible with the current code
    bool struct_initialized; // Set to true once the settings have been initialized at least once, used to determine if we need to write default settings to EEPROM
    uint8_t struct_version;  // Used to determine if the structure of the saved settings in the firmware has changed from that saved in arduino EEPROM in a non-backwards compatible way.

    // ISMS Settings
    bool autostart_on;                // Run the startup sequence after boot.
    unsigned long autostart_delay;    // Milliseconds to wait before autostart.
    bool stats_loging_enabled;        // Broadcast the periodic stats telegram.
    unsigned long stats_log_interval; // Milliseconds between stats telegrams.
    DebugLogLevel log_level;          // Verbosity, per the DEBUG_LOGGING command.
    int16_t fluidpump_rate;           // Fluid pump speed, -100 to 100 percent.
};

// Guards the struct against outgrowing the EEPROM at its slot. Because the
// slot is scaled by sizeof(saved_settings_t) (see eeprom_settings_slot above),
// adding fields pushes the physical address up by 3 bytes per added byte per
// slot -- at slot 50, a struct larger than 26 bytes writes past E2END, where
// the 12-bit address register wraps and quietly corrupts low EEPROM instead of
// failing. struct_version cannot catch this: the layout is fine, the address
// is not. Today: 51 * 16 * 3 = 2448 of 4096 bytes used.
static_assert((eeprom_settings_slot + 1) * sizeof(saved_settings_t) * 3 <= E2END + 1,
              "saved_settings_t no longer fits in EEPROM at eeprom_settings_slot - "
              "shrink the struct or lower the slot number");

// Default settings to use if there are no valid saved settings in EEPROM
saved_settings_t default_settings{
    .struct_initialized = true,
    .struct_version = 4, // Increment this if the structure of the settings changes in a non-backwards compatible way (e.g. changing types of existing fields, removing fields, changing the order of fields, etc) so that we can detect when the saved settings are no longer valid and need to be reset to defaults

    // ISMS Settings
    .autostart_on = false,
    .autostart_delay = 5000, // Default delay of 5 seconds for autostartup
    .stats_loging_enabled = true,
    .stats_log_interval = 1000,           // 1 second default interval for periodic stats logging
    .log_level = DebugLogLevel::LVL_INFO, // Default log level is INFO which corresponds to DEBUG_LOG_LEVEL_OFF and includes all stats logging
    .fluidpump_rate = 100,                // Default fluid pump rate in percentage of full speed
};

// The live settings the whole firmware reads. Zeroed until load_settings()
// runs in setup(), so nothing should be read out of it before then.
saved_settings_t saved_settings = {};

// Writes the live settings to EEPROM.
//
// PROMPLUS keeps three redundant copies and rewrites only the bytes that
// actually changed, so this is cheap enough for every command that edits a
// setting to call it immediately.
void save_settings()
{
    // Save the current settings to EEPROM
    PROM.put(eeprom_settings_slot, saved_settings);
}

// Restores the compiled-in defaults and persists them to EEPROM.
//
// Called by load_settings() when what EEPROM holds cannot be trusted, and by
// the RESET_SETTINGS serial command. Logs the metadata it wrote so an operator
// can confirm the reset took effect.
void reset_settings()
{
    // Reset to defaults and save
    saved_settings = default_settings;
    save_settings();

    // Print out eeprom info
    LOG_INFO(F("| EEPROM: Wrote default settings to EEPROM [ struct_initialized: "));
    LOG_INFO(saved_settings.struct_initialized);
    LOG_INFO(F(", struct_version: "));
    LOG_INFO(saved_settings.struct_version);
    LOG_INFO(F(" ]\n"));
}

// Loads the saved settings from EEPROM, falling back to the defaults whenever
// they are invalid.
//
// Seeds the live settings with the defaults first, so even a failed read
// leaves the firmware with a usable configuration, then reads EEPROM and
// discards the result if it was never initialized or was written by a
// different struct version -- rewriting the defaults in that case rather than
// acting on a misaligned struct. Logs every loaded value for the operator's
// record. Call once from setup(), before any setting is read.
void load_settings()
{

    // use default settings in case eeprom read fails.
    memcpy(&saved_settings, &default_settings, sizeof(saved_settings));

    // Load saved settings from EEPROM, if they are valid (i.e. struct_initialized is true and struct_version matches)
    PROM.get(eeprom_settings_slot, saved_settings);
    LOG_INFO(F("| EEPROM: Loading saved settings [ struct_initialized: "));
    LOG_INFO(saved_settings.struct_initialized);
    LOG_INFO(F(", struct_version: "));
    LOG_INFO(saved_settings.struct_version);
    LOG_INFO(F(" ]\n"));

    // Check if settings are valid - must be initialized AND version must match
    bool settings_valid = (saved_settings.struct_initialized == true) && (saved_settings.struct_version == default_settings.struct_version);
    if (!settings_valid)
    {
        if (!saved_settings.struct_initialized)
        {
            LOG_WARN(F("!WARN: No ISMS saved settings found in EEPROM, saving default values...\n"));
        }
        else if (saved_settings.struct_version != default_settings.struct_version)
        {
            LOG_WARN(F("!WARN: ISMS saved settings in EEPROM are invalid (struct version mismatch), saving default values...\n"));
        }

        // Reset to defaults
        reset_settings();
    }

    // Print out loaded settings:
    LOG_INFO(F("| SETTINGS autostart_on: "));
    LOG_INFO(saved_settings.autostart_on ? "yes" : "no");
    LOG_INFO(F(", autostart_delay: "));
    LOG_INFO(saved_settings.autostart_delay);
    LOG_INFO(F("ms, stats_loging_enabled: "));
    LOG_INFO(saved_settings.stats_loging_enabled);
    LOG_INFO(F(", stats_log_interval: "));
    LOG_INFO(saved_settings.stats_log_interval);
    LOG_INFO(F("ms, log_level: "));
    LOG_INFO(logLevelToString(saved_settings.log_level).c_str());
    LOG_INFO(F(", fluidpump_rate: "));
    LOG_INFO(saved_settings.fluidpump_rate);
    LOG_INFO("%\n");
}
