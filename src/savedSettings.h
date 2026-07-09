#include <Arduino.h>
#include <PROMPLUS.h>
#include <DebugLog.h>
#include "includes.h"

// ISMS Settings are stored in EEPROM (redundantly via the PROMPLUS library) using a c++ struct.
// When the ISMS boots up, it will attempt to load the settings saved in EEPROM.
// When settings are changed, the full struct is saved to EEPROM so that the settings persist by calling save_settings().
// Writes are done using the PROM.put() function which only updated the bits that have changed,
// to keep the EEPROM wear to a minimum. In the unlikely event that all three copies stored in the eeprom by the PROMPLUS
// library have been worn out and settings do not save or load correctly,  Update the eeprom_start_address variable to point
// to a part of the eeprom past where the settings struct was stored.

size_t eeprom_start_address = 50;

/** ISMS Settings struct - contains all settings that should persist through reboots and power cycles - stored in EEPROM
 *  !!! IMPORTANT: To change this struct follow these guidelines:
 *   - If all you change is the names of existing struct fields, nothing else is needed.
 *   - If you do anythhing else to the struct (e.g. change types of existing fields, add new fields,
 *     remove fields, change the order of fields, etc), you must increment the struct_version field
 *     in the default_settings variable below so that the system knows to reset to default settings
 *     instead of reading a potentially incompatible/miss-alinged struct from EEPROM.
 *   - Make sure to set a default value in the default_settings variable below for any relavant fields you add to the struct!
 */
struct saved_settings_t
{
    // Metadata fields to determine if the saved settings are valid and compatible with the current code
    bool struct_initialized; // Set to true once the settings have been initialized at least once, used to determine if we need to write default settings to EEPROM
    uint8_t struct_version;  // Used to determine if the structure of the saved settings in the firmware has changed from that saved in arduino EEPROM in a non-backwards compatible way.

    // ISMS Settings
    bool autostart_on;
    unsigned long autostart_delay;
    bool stats_loging_enabled;
    unsigned long stats_log_interval;
    DebugLogLevel log_level;
    int16_t fluidpump_rate;
};

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

saved_settings_t saved_settings = {};

void save_settings()
{
    // Save the current settings to EEPROM
    PROM.put(eeprom_start_address, saved_settings);
}

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

void load_settings()
{

    // use default settings in case eeprom read fails.
    memcpy(&saved_settings, &default_settings, sizeof(saved_settings));

    // Load saved settings from EEPROM, if they are valid (i.e. struct_initialized is true and struct_version matches)
    PROM.get(eeprom_start_address, saved_settings);
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
