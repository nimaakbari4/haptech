import json

SETTINGS = {}

with open("settings.json") as settings_file:
    SETTINGS = json.load(settings_file)

def get_setting(key):
    if key in SETTINGS:
        return SETTINGS[key]
    
    return None

def change_setting(key, value):
    SETTINGS[key] = value

    with open("settings.json", 'w') as settings_file:
        json.dump(SETTINGS, settings_file, indent=4)
