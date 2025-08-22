import grpc
import csv
import franka_trial_pb2 as pb2
import franka_trial_pb2_grpc as pb2_grpc

# Start a trial and stream back data/status updates
def run_trial(stub: pb2_grpc.FrankaTrialServiceStub, trial_id: str, duration: float, output_csv_stream):
    request = pb2.TrialRequest(trial_id=trial_id, trialDuration=duration)
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

