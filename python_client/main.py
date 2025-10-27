import util
import grpc
import franka_trial_pb2_grpc as pb2_grpc
import traceback
import logging
from parser import xml_parser

def main():
    # Take user input for trial ID, trial duration from config

    # Get user input
    trial_id = input("Enter a trial ID: ")
    config_file_path = input("Enter a path to your config file: ")

    # Setup per-trial logging (console + file)
    logger = logging.getLogger("haptech")
    logger.setLevel(logging.INFO)
    if logger.handlers:
        for h in list(logger.handlers):
            logger.removeHandler(h)
    fh = logging.FileHandler(f"{trial_id}.log", mode='w')
    fmt = logging.Formatter('%(asctime)s %(levelname)s: %(message)s')
    fh.setFormatter(fmt)
    ch = logging.StreamHandler()
    ch.setFormatter(fmt)
    logger.addHandler(fh)
    logger.addHandler(ch)

    logger.info("Parsing config: %s", config_file_path)
    config = xml_parser(config_file_path)
    end_conditions = config.end_conditions.conditions
    start_conditions = config.start_conditions.conditions
    
    # Display all start conditions that will be used
    print(f"Found {len(start_conditions)} start condition(s):")
    for condition in start_conditions:
        print(f"  - {condition.name} (success={condition.success}, priority={condition.priority})")
        for param in condition.parameters:
            print(f"    {param.key}: {param.value}")

    # Display all end conditions that will be used
    print(f"Found {len(end_conditions)} end condition(s):")
    for condition in end_conditions:
        print(f"  - {condition.name} (success={condition.success}, priority={condition.priority})")
        for param in condition.parameters:
            print(f"    {param.key}: {param.value}")
    
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
        channel = grpc.insecure_channel('localhost:50051')
        # channel = grpc.insecure_channel('134.114.232.201:50051')
        stub = pb2_grpc.FrankaTrialServiceStub(channel)

        # call run_trial with those values
        condition_types = [c.name for c in start_conditions]
        print(f"Starting trial '{trial_id}' (start conditions: {', '.join(condition_types)})...")

        condition_types = [c.name for c in end_conditions]
        print(f"Starting trial '{trial_id}' (end conditions: {', '.join(condition_types)})...")

        util.run_trial(stub, trial_id, output_filestream, config.objects, config.start_conditions.conditions, config.end_conditions.conditions)
        print("Trial completed.")
        logger.info("Trial '%s' completed successfully", trial_id)
    except grpc.RpcError as e:
        code = e.code() if hasattr(e, 'code') else None
        details = e.details() if hasattr(e, 'details') else str(e)
        print(f"gRPC error: {code} - {details}")
        logger.error("gRPC error: code=%s details=%s", code, details)
        logger.exception("gRPC exception during trial")
    except Exception:
        print("Something went wrong.")
        traceback.print_exc()
        logger.exception("Unhandled exception during trial")
    finally:
        output_filestream.close()

if __name__ == "__main__":
    main()