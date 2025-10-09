import util
import grpc
import franka_trial_pb2_grpc as pb2_grpc
import traceback
from parser import xml_parser

def main():
    # Take user input for trial ID, trial duration from config

    # Get user input
    trial_id = input("Enter a trial ID: ")
    config_file_path = input("Enter a path to your config file: ")

    config = xml_parser(config_file_path)
    end_conditions = config.end_conditions.conditions
    duration = None

    for condition in end_conditions:
        if condition.type == "time":
            duration = float(condition.value)

    if not duration:
        raise "Invalid trial duration provided. Check your config file for a 'time' end condition."
    
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
        #channel = grpc.insecure_channel('localhost:50051')
        channel = grpc.insecure_channel('134.114.232.201:50051')
        stub = pb2_grpc.FrankaTrialServiceStub(channel)

        # call run_trial with those values
        print(f"Starting trial '{trial_id}' for {duration} seconds...")
        util.run_trial(stub, trial_id, duration, output_filestream, config.objects)
        print("Trial completed.")
    except Exception:
        print("Something went wrong.")
        traceback.print_exc()
    finally:
        output_filestream.close()

if __name__ == "__main__":
    main()