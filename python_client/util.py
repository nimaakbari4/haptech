import grpc
import csv
import franka_trial_pb2 as pb2
import franka_trial_pb2_grpc as pb2_grpc

# Start a trial and stream back data/status updates
def run_trial(stub: pb2_grpc.FrankaTrialServiceStub, trial_id: str, duration: float, output_csv_stream, objects=None, end_conditions=None):
    request = pb2.TrialRequest(trial_id=trial_id, trialDuration=duration)
    
    # Add objects to the request if provided
    if objects:
        for obj in objects:
            # Create Object message
            obj_msg = request.objects.add()
            obj_msg.id = obj.id
            obj_msg.type = obj.type
            obj_msg.position.x = obj.position.x
            obj_msg.position.y = obj.position.y
            obj_msg.position.z = obj.position.z
            obj_msg.orientation.x = obj.orientation.x
            obj_msg.orientation.y = obj.orientation.y
            obj_msg.orientation.z = obj.orientation.z
            
            if obj.color:
                obj_msg.color.r = obj.color.r
                obj_msg.color.g = obj.color.g
                obj_msg.color.b = obj.color.b
            
            # Add all parameters from the params list
            if obj.params:
                for param in obj.params:
                    param_msg = obj_msg.params.add()
                    param_msg.key = param.key
                    param_msg.value = param.value
    
    # Add end conditions to the request if provided
    if end_conditions:
        for condition in end_conditions:
            # Create EndCondition message
            end_cond_msg = request.endConditions.add()
            end_cond_msg.name = condition.name
            end_cond_msg.success = condition.success
            end_cond_msg.priority = condition.priority
            
            # Add parameters
            for param in condition.parameters:
                param_msg = end_cond_msg.params.add()
                param_msg.key = param.key
                param_msg.value = param.value
    
    response_stream = stub.RunTrial(request)
    handle_stream(response_stream, output_csv_stream)

# Process messages coming from the stream
def handle_stream(stream, output_csv_stream):
    for message in stream:
        if message.HasField("statusUpdate"):
            handle_status_update(message.statusUpdate)
        elif message.HasField("dataPoint"):
            handle_data_point(message.dataPoint, output_csv_stream)

# Handle a TrialStatusUpdate
def handle_status_update(status):
    # TODO: Display status
    print(f"Status Update: {status.message} (Status: {status.status})")

# Handle a TrialDataPoint
def handle_data_point(data, output_csv_stream):
    timestamp = data.timestamp
    device_pos = data.devicePosition
    proxy_pos = data.proxyPosition
    force = data.force

    # Format vectors as [x, y, z]
    device_pos_str = f"[{device_pos.x:.4f}, {device_pos.y:.4f}, {device_pos.z:.4f}]"
    proxy_pos_str = f"[{proxy_pos.x:.4f}, {proxy_pos.y:.4f}, {proxy_pos.z:.4f}]"
    force_str = f"[{force.x:.4f}, {force.y:.4f}, {force.z:.4f}]"

    # display data point
    print(f"Data Point: {timestamp:.4f}s | Device Position: {device_pos_str} | Proxy Position: {proxy_pos_str} | Force: {force_str}")

    # log to CSV file
    writer = csv.writer(output_csv_stream)
    writer.writerow([
        timestamp,
        device_pos.x, device_pos.y, device_pos.z,
        proxy_pos.x, proxy_pos.y, proxy_pos.z,
        force.x, force.y, force.z
    ])

