import grpc
import franka_trial_pb2 as pb2
import franka_trial_pb2_grpc as pb2_grpc

def run_trial():
    # Connect to the gRPC server
    channel = grpc.insecure_channel('localhost:50051')
    stub = pb2_grpc.FrankaTrialServiceStub(channel)

    # Create a trial request for 10 seconds
    request = pb2.TrialRequest(trial_id="test_trial_10s", trialDuration=10.0)

    # Start the trial and process the stream
    print("[Client] Sending trial request...")
    stream = stub.RunTrial(request)

    for message in stream:
        if message.HasField("statusUpdate"):
            status = message.statusUpdate
            print(f"[Status] {pb2.TrialStatus.Name(status.status)} - {status.message}")
        elif message.HasField("dataPoint"):
            dp = message.dataPoint
            device = dp.devicePosition
            print(f"[Data] t={dp.timestamp:.2f}s - Device Pos: ({device.x:.2f}, {device.y:.2f}, {device.z:.2f})")

if __name__ == '__main__':
    run_trial()
