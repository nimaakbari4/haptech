import json
import util
import grpc
import franka_trial_pb2_grpc as pb2_grpc
import traceback
from parser import xml_parser
import settings
import datetime
from dataclasses import asdict

def main():
    # Get needed settings
    server_host = settings.get_setting("server_host")
    server_port = settings.get_setting("server_port")

    if not server_host:
        print("[ERROR] Need to specify 'server_host' in settings.json")
        return
    
    if not server_port:
        print("[ERROR] Need to specify 'server_port' in settings.json")
        return
    
    # Get user input
    trial_id = input("Enter a trial ID: ")
    config_file_path = input("Enter a path to your config file: ")

    config = xml_parser(config_file_path)
    end_conditions = config.end_conditions.conditions
    start_conditions = config.start_conditions.conditions
    
    # # Display all start conditions that will be used
    # print(f"Found {len(start_conditions)} start condition(s):")
    # for condition in start_conditions:
    #     print(f"  - {condition.name} (success={condition.success}, priority={condition.priority})")
    #     for param in condition.parameters:
    #         print(f"    {param.key}: {param.value}")

    # # Display all end conditions that will be used
    # print(f"Found {len(end_conditions)} end condition(s):")
    # for condition in end_conditions:
    #     print(f"  - {condition.name} (success={condition.success}, priority={condition.priority})")
    #     for param in condition.parameters:
    #         print(f"    {param.key}: {param.value}")
    
    output_filestream = open(f"{trial_id}_results.csv", 'w')
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
            "trial_id": trial_id,
            "configuration_file": config_file_path,
            "started_at": datetime.datetime.now(),
        }

        util.run_trial(stub, trial_id, output_filestream, config.objects, 
                       config.start_conditions.conditions, config.end_conditions.conditions, 
                       metadata)
        print("Trial completed.")

        metadata["ended_at"] = datetime.datetime.now()
        metadata["configuration"] = asdict(config)

        json.dump(metadata, open(f"{trial_id}_metadata.json", 'w'), default=str, indent=4)   
    except Exception:
        print("Something went wrong.")
        traceback.print_exc()
    finally:
        output_filestream.close()

if __name__ == "__main__":
    main()