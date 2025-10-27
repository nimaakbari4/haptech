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
    
    # Display all end conditions that will be used
    print(f"Found {len(end_conditions)} end condition(s):")
    for condition in end_conditions:
        print(f"  - {condition.name} (success={condition.success}, priority={condition.priority})")
        for param in condition.parameters:
            print(f"    {param.key}: {param.value}")
    
    # For backward compatibility, still extract duration for display purposes
    duration = None
    for condition in end_conditions:
        if condition.name == "time":
            for param in condition.parameters:
                if param.key == "duration_seconds":
                    duration = float(param.value)
                    break
    
    output_filestream = open(f"{trial_id}_results.csv", 'w')
    # Header will be written dynamically after first data point (when we know object IDs)

    try:
        # connect to the server
        channel = grpc.insecure_channel('localhost:50051')
        #channel = grpc.insecure_channel('134.114.232.201:50051')
        stub = pb2_grpc.FrankaTrialServiceStub(channel)

        # call run_trial with those values
        if duration:
            print(f"Starting trial '{trial_id}' (time limit: {duration}s)...")
        else:
            condition_types = [c.name for c in end_conditions]
            print(f"Starting trial '{trial_id}' (end conditions: {', '.join(condition_types)})...")
        
        # Display data collection config if specified
        if config.data_collection:
            print("\nData Collection Configuration:")
            print(f"  Device: position={config.data_collection.device.position}, "
                  f"velocity={config.data_collection.device.velocity}, "
                  f"orientation={config.data_collection.device.orientation}, "
                  f"force={config.data_collection.device.force}")
            print(f"  Objects: position={config.data_collection.objects.position}, "
                  f"contact={config.data_collection.objects.contact}, "
                  f"force={config.data_collection.objects.force}")
        else:
            print("\nData Collection: Using defaults (collect everything)")
        
        util.run_trial(stub, trial_id, duration or 0, output_filestream, config.objects, config.end_conditions.conditions, config.data_collection)
        print("Trial completed.")
    except Exception:
        print("Something went wrong.")
        traceback.print_exc()
    finally:
        output_filestream.close()

if __name__ == "__main__":
    main()