#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoLog.h>

// ISMS Settings are stored in EEPROM using a c++ struct.
// When the ISMS boots up, it will attempt to load the settings from EEPROM.
// When settings are changed, the full struct is saved to EEPROM so that the settings persist.
// Writes are done using the EEPROM.put() function which only updated the bits that have changed,
// to keep the eeprom wear to a minimum.
// In the unlikely event that part of the eeprom has been worn out and settings do not save or load correctly,
// Update the eeprom_start_address variable to point to a part of the eeprom past where the settings struct was stored.

size_t eeprom_start_address = 0;

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
    uint8_t log_level;
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
    .stats_log_interval = 1000,  // 1 second default interval for periodic stats logging
    .log_level = LOG_LEVEL_INFO, // Default log level is INFO which includes stats logging
    .fluidpump_rate = 100,       // Default fluid pump rate in percentage of full speed
};

saved_settings_t saved_settings;

void save_settings()
{
    // Save the current settings to EEPROM
    EEPROM.put(eeprom_start_address, saved_settings);
}

void load_settings()
{
    // Load saved settings from EEPROM, if they are valid (i.e. struct_initialized is true and struct_version matches)
    EEPROM.get(eeprom_start_address, saved_settings);
    Log.infoln("| EEPROM: Loading saved settings [ struct_initialized: %T, struct_version: %d ]", saved_settings.struct_initialized, saved_settings.struct_version);

    // Check if settings are valid - must be initialized AND version must match
    bool settings_valid = (saved_settings.struct_initialized == true) && (saved_settings.struct_version == default_settings.struct_version);

    if (!settings_valid)
    {
        if (!saved_settings.struct_initialized)
        {
            Log.warningln(F("WARN: No ISMS saved settings found in EEPROM, using defaults"));
        }
        else if (saved_settings.struct_version != default_settings.struct_version)
        {
            Log.warningln(F("WARN: ISMS saved settings in EEPROM are invalid (struct version mismatch), reverting to default values..."));
        }

        // Reset to defaults and save
        saved_settings = default_settings;
        save_settings();
        Log.infoln("| EEPROM: wrote default settings to EEPROM [ struct_initialized: %T, struct_version: %d ]", saved_settings.struct_initialized, saved_settings.struct_version);
    }
    Log.infoln("| SETTINGS: autostart_on: %T, autostart_delay: %u ms, stats_loging_enabled: %T, stats_log_interval: %lu ms, log_level: %s(%d), fluidpump_rate: %d", saved_settings.autostart_on, saved_settings.autostart_delay, saved_settings.stats_loging_enabled, saved_settings.stats_log_interval, logLevelToString(saved_settings.log_level).c_str(), saved_settings.log_level, saved_settings.fluidpump_rate);
}
