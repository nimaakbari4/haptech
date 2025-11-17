import json
import util
import grpc
import franka_trial_pb2_grpc as pb2_grpc
import traceback
import logging
from parser import xml_parser
import settings
import datetime
import os
from dataclasses import asdict

def main():
    # Set up logger
    logger = logging.getLogger("haptech")
    logger.setLevel(logging.INFO)
    if logger.handlers:
        for h in list(logger.handlers):
            logger.removeHandler(h)

    # Get needed settings
    server_host = settings.get_setting("server_host")
    server_port = settings.get_setting("server_port")
    config_path = settings.get_setting("config_path")
    results_path = settings.get_setting("results_path")
    trial_subject = settings.get_setting("trial_subject")
    last_configuration_file = settings.get_setting("last_configuration_file")

    if not server_host:
        print("[ERROR] Need to specify 'server_host' in settings.json")
        return
    
    if not server_port:
        print("[ERROR] Need to specify 'server_port' in settings.json")
        return
    
    if not results_path:
        print("[ERROR] Need to specify 'results_path' in settings.json")
        return
    
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

        # Setup per-trial logging (console + file)
        fh = logging.FileHandler(f"{trial_id}.log", mode='w')
        fmt = logging.Formatter('%(asctime)s %(levelname)s: %(message)s')
        fh.setFormatter(fmt)
        ch = logging.StreamHandler()
        ch.setFormatter(fmt)
        logger.addHandler(fh)
        logger.addHandler(ch)
    else:
        trial_subject = "TEST"
        trial_id = f"TEST_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}"

    end_conditions = config.end_conditions.conditions
    start_conditions = config.start_conditions.conditions
    
    # Only create output files if not in test mode
    output_filestream = None
    if not config.test:
        # Create output directory structure: results_path/Subject_YYYY-MM-DD/
        current_date = datetime.datetime.now().strftime("%Y-%m-%d")
        output_folder = f"{trial_subject}_{current_date}"
        output_dir = os.path.join(results_path, output_folder)
        
        # Create the directory if it doesn't exist
        os.makedirs(output_dir, exist_ok=True)
        
        # Define file paths within the organized directory
        csv_file_path = os.path.join(output_dir, f"{trial_id}_results.csv")
        metadata_file_path = os.path.join(output_dir, f"{trial_id}_metadata.json")
        
        output_filestream = open(csv_file_path, 'w')
        output_filestream.write(
            "t,"
            "device_pos_x,device_pos_y,device_pos_z,"
            "proxy_pos_x,proxy_pos_y,proxy_pos_z,"
            "force_x,force_y,force_z,"
            "device_vel_x,device_vel_y,device_vel_z,"
            "device_ang_vel_x,device_ang_vel_y,device_ang_vel_z,"
            "device_quat_x,device_quat_y,device_quat_z,device_quat_w\n"
        )

    try:
        # connect to the server
        channel = grpc.insecure_channel(f"{server_host}:{server_port}")
        stub = pb2_grpc.FrankaTrialServiceStub(channel)

        # call run_trial with those values
        condition_types = [c.name for c in start_conditions]
        # print(f"Starting trial '{trial_id}' (start conditions: {', '.join(condition_types)})...")

        condition_types = [c.name for c in end_conditions]
        # print(f"Starting trial '{trial_id}' (end conditions: {', '.join(condition_types)})...")

        metadata = {
            "trial_subject": trial_subject,
            "trial_id": trial_id,
            "configuration_file": config_file_path,
            "started_at": datetime.datetime.now(),
        }

        print("Starting trial...\n")
        util.run_trial(
            stub,
            trial_id,
            output_filestream,
            config.objects,
            config.start_conditions.conditions,
            config.end_conditions.conditions,
            scene_settings=config.scene_settings,
            metadata=metadata,
            cursor_settings=config.cursor_settings,
        )

        metadata["ended_at"] = datetime.datetime.now()
        metadata["configuration"] = asdict(config)

        # Only write metadata file if not in test mode
        if not config.test:
            json.dump(metadata, open(metadata_file_path, 'w'), default=str, indent=4)
            print_trial_summary(metadata, csv_file_path, metadata_file_path)
        else:
            print_trial_summary(metadata)

        print("Trial complete.")
        logger.info("Trial '%s' completed successfully", trial_id)
    except grpc.RpcError as e:
        code = e.code() if hasattr(e, 'code') else None
        details = e.details() if hasattr(e, 'details') else str(e)
        print(f"gRPC error: {code} - {details}")
        logger.error("gRPC error: code=%s details=%s", code, details)
        logger.exception("gRPC exception during trial")
    except Exception:
        print("Something went wrong.")
        logger.exception("Unhandled exception during trial")
    finally:
        if output_filestream:
            output_filestream.close()

def print_trial_summary(metadata, csv_file_path=None, metadata_file_path=None):
    # Calculate duration
    duration = metadata.get("trial_duration")
    minutes = duration // 60
    seconds = duration % 60
    duration_str = f"{minutes}m {seconds:.1f}s" if minutes > 0 else f"{seconds:.1f}s"
    
    # Table data
    table_data = [
        ("Subject", metadata.get("trial_subject", "Unknown")),
        ("Trial ID", metadata.get("trial_id", "Unknown")),
        ("Duration", duration_str),
        ("Config File", metadata.get("configuration_file", "Unknown")),
    ]
    
    # Add file paths if they exist
    if csv_file_path:
        table_data.append(("Results CSV", os.path.relpath(csv_file_path)))
    if metadata_file_path:
        table_data.append(("Metadata JSON", os.path.relpath(metadata_file_path)))

    # Calculate column widths
    max_label_width = max(len(label) for label, _ in table_data)
    max_value_width = max(len(str(value)) for _, value in table_data)
    total_table_width = max_label_width + max_value_width + 7 

    # Create the table
    print("\n" + "=" * total_table_width)
    print(" " * (total_table_width // 2 - 7) + "TRIAL SUMMARY")
    print("=" * total_table_width)

    # Print the table
    for label, value in table_data:
        print(f"│ {label:<{max_label_width}} │ {value:<{max_value_width}} │")

    print("=" * total_table_width + "\n")

if __name__ == "__main__":
    main()