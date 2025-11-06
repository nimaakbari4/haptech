import json
import os

SETTINGS = {}
DEFAULT_SETTINGS = {
    "config_path": "config",
    "results_path": "results",
    "server_host": "localhost",
    "server_port": 50051
}

# Check if settings.json exists, if not create it with defaults
if not os.path.exists("settings.json"):
    with open("settings.json", 'w') as settings_file:
        json.dump(DEFAULT_SETTINGS, settings_file, indent=4)

    SETTINGS = DEFAULT_SETTINGS.copy()
else:
    with open("settings.json") as settings_file:
        SETTINGS = json.load(settings_file)

def get_setting(key):
    if key in SETTINGS:
        return SETTINGS[key]
    
    return None

def change_setting(key, value):
    new_settings = SETTINGS.copy()
    new_settings[key] = value

    with open("settings.json", 'w') as settings_file:
        json.dump(new_settings, settings_file, indent=4)
