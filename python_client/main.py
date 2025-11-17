import settings
from trials import run_trial
from parser import xml_parser

def main():
    # Get needed settings
    trial_subject = settings.get_setting("trial_subject")
    last_configuration_file = settings.get_setting("last_configuration_file")
    config_path = settings.get_setting("config_path")

    if not config_path:
        print("[ERROR] Need to specify 'config_path' in settings.json")
        return

    # Get user input
    if not last_configuration_file:
        config_file_path = input("Trial configuration file: ")
    else:
        config_file_choice = input(f"Last configuration file: '{last_configuration_file}'\nPress Enter to continue or enter a new file: ")
        if config_file_choice:
            config_file_path = config_file_choice
        else:
            config_file_path = last_configuration_file

    settings.change_setting("last_configuration_file", config_file_path)
    config = xml_parser(f"{config_path}/{config_file_path}")

    # If config is not in test mode, get trial subject and id
    if not config.test:
        if not trial_subject:
            trial_subject = input("Trial subject: ")
            settings.change_setting("trial_subject", trial_subject)
        else:
            trial_subject_choice = input(f"Last trial subject: '{trial_subject}'\nPress Enter to continue or enter a new subject: ")
            if trial_subject_choice:
                trial_subject = trial_subject_choice
                settings.change_setting("trial_subject", trial_subject)
        
        trial_id = input("Trial ID: ")

        run_trial(config_file_path, trial_subject, trial_id)
    else:
        run_trial(config_file_path, "TEST", "TEST")

if __name__ == "__main__":
    main()