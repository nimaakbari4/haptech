import settings
import datetime
from parser import xml_parser
import trials

def main():
    # Get needed settings
    config_path = settings.get_setting("config_path")
    trial_subject = settings.get_setting("trial_subject")
    last_configuration_file = settings.get_setting("last_configuration_file")

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
    is_nima = any(
        condition.name == "nima_reaching"
        for condition in config.end_conditions.conditions
    )

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
    else:
        trial_subject = "TEST"
        trial_id = f"TEST_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}"

    # Run the trial using the trials module

    if not is_nima:
        trials.run_trial(config_file_path, trial_subject, trial_id)
        return

    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    session_id = f"{trial_id.strip() or 'reach'}_{stamp}"
    trial_number = 1

    while True:
        numbered_id = f"{session_id}_{trial_number:03d}"
        completed = trials.run_trial(config_file_path, trial_subject, numbered_id)
        if not completed:
            break

        answer = input("Go for another trial? [y/N] ").strip().lower()
        if answer not in {"y", "yes"}:
            break

        trial_number += 1


if __name__ == "__main__":
    main()
