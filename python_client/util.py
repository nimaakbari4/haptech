import grpc
import csv
import franka_trial_pb2 as pb2
import franka_trial_pb2_grpc as pb2_grpc

# Start a trial and stream back data/status updates
def run_trial(stub: pb2_grpc.FrankaTrialServiceStub, trial_id: str, duration: float, output_csv_stream, objects=None, end_conditions=None, data_collection=None):
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
    
    # Add data collection configuration if provided
    if data_collection:
        dc_msg = request.dataCollection
        # Device data config
        dc_msg.device.position = data_collection.device.position
        dc_msg.device.velocity = data_collection.device.velocity
        dc_msg.device.angularVelocity = data_collection.device.angular_velocity
        dc_msg.device.orientation = data_collection.device.orientation
        dc_msg.device.proxyPosition = data_collection.device.proxy_position
        dc_msg.device.force = data_collection.device.force
        # Object data config
        dc_msg.objects.position = data_collection.objects.position
        dc_msg.objects.orientation = data_collection.objects.orientation
        dc_msg.objects.contact = data_collection.objects.contact
        dc_msg.objects.contactPoint = data_collection.objects.contact_point
        dc_msg.objects.contactNormal = data_collection.objects.contact_normal
        dc_msg.objects.penetrationDepth = data_collection.objects.penetration_depth
        dc_msg.objects.force = data_collection.objects.force
        dc_msg.objects.springForce = data_collection.objects.spring_force
        dc_msg.objects.dampingForce = data_collection.objects.damping_force
    
    response_stream = stub.RunTrial(request)
    handle_stream(response_stream, output_csv_stream, data_collection)

# Process messages coming from the stream
def handle_stream(stream, output_csv_stream, data_collection=None):
    for message in stream:
        if message.HasField("statusUpdate"):
            handle_status_update(message.statusUpdate)
        elif message.HasField("dataPoint"):
            # Write CSV header on first data point (when we know object IDs)
            if not hasattr(output_csv_stream, '_header_written'):
                write_csv_header(output_csv_stream, message.dataPoint, data_collection)
                output_csv_stream._header_written = True
            handle_data_point(message.dataPoint, output_csv_stream, data_collection)

def write_csv_header(output_csv_stream, first_data_point, data_collection=None):
    """Write CSV header based on the first data point received and data collection config"""
    header = ["t"]  # Timestamp is always included
    
    # Default to collecting everything if no config provided
    if data_collection is None:
        from parser import DeviceDataConfig, ObjectDataConfig, DataCollectionConfig
        data_collection = DataCollectionConfig(
            device=DeviceDataConfig(),
            objects=ObjectDataConfig()
        )
    
    # Device data headers based on config
    if data_collection.device.position:
        header += ["device_pos_x", "device_pos_y", "device_pos_z"]
    if data_collection.device.proxy_position:
        header += ["proxy_pos_x", "proxy_pos_y", "proxy_pos_z"]
    if data_collection.device.force:
        header += ["force_x", "force_y", "force_z"]
    if data_collection.device.velocity:
        header += ["device_vel_x", "device_vel_y", "device_vel_z"]
    if data_collection.device.angular_velocity:
        header += ["device_ang_vel_x", "device_ang_vel_y", "device_ang_vel_z"]
    if data_collection.device.orientation:
        header += ["device_quat_x", "device_quat_y", "device_quat_z", "device_quat_w"]
    
    # Object state columns based on config
    for obj in first_data_point.objects:
        obj_id = obj.id
        
        if data_collection.objects.position:
            header += [f"obj_{obj_id}_pos_x", f"obj_{obj_id}_pos_y", f"obj_{obj_id}_pos_z"]
        if data_collection.objects.orientation:
            header += [f"obj_{obj_id}_quat_x", f"obj_{obj_id}_quat_y", f"obj_{obj_id}_quat_z", f"obj_{obj_id}_quat_w"]
        if data_collection.objects.contact:
            header += [f"obj_{obj_id}_in_contact"]
        if data_collection.objects.contact_point:
            header += [f"obj_{obj_id}_contact_pt_x", f"obj_{obj_id}_contact_pt_y", f"obj_{obj_id}_contact_pt_z"]
        if data_collection.objects.contact_normal:
            header += [f"obj_{obj_id}_contact_norm_x", f"obj_{obj_id}_contact_norm_y", f"obj_{obj_id}_contact_norm_z"]
        if data_collection.objects.penetration_depth:
            header += [f"obj_{obj_id}_penetration_depth"]
        if data_collection.objects.force:
            header += [f"obj_{obj_id}_force_x", f"obj_{obj_id}_force_y", f"obj_{obj_id}_force_z"]
        if data_collection.objects.spring_force:
            header += [f"obj_{obj_id}_spring_force_x", f"obj_{obj_id}_spring_force_y", f"obj_{obj_id}_spring_force_z"]
        if data_collection.objects.damping_force:
            header += [f"obj_{obj_id}_damping_force_x", f"obj_{obj_id}_damping_force_y", f"obj_{obj_id}_damping_force_z"]
    
    output_csv_stream.write(",".join(header) + "\n")

# Handle a TrialStatusUpdate
def handle_status_update(status):
    # TODO: Display status
    print(f"Status Update: {status.message} (Status: {status.status})")

# Handle a TrialDataPoint
def handle_data_point(data, output_csv_stream, data_collection=None):
    timestamp = data.timestamp
    
    # Default to collecting everything if no config provided
    if data_collection is None:
        from parser import DeviceDataConfig, ObjectDataConfig, DataCollectionConfig
        data_collection = DataCollectionConfig(
            device=DeviceDataConfig(),
            objects=ObjectDataConfig()
        )
    
    # Extract device data
    device_pos = data.devicePosition if data.HasField("devicePosition") else None
    proxy_pos = data.proxyPosition if data.HasField("proxyPosition") else None
    force = data.force if data.HasField("force") else None
    device_vel = data.deviceVelocity if data.HasField("deviceVelocity") else None
    device_ang_vel = data.deviceAngularVelocity if data.HasField("deviceAngularVelocity") else None
    device_quat = data.deviceOrientation if data.HasField("deviceOrientation") else None

    # Display data point (summary only)
    print(f"Data Point: {timestamp:.4f}s", end="")
    if device_pos and data_collection.device.position:
        print(f" | Device: [{device_pos.x:.4f}, {device_pos.y:.4f}, {device_pos.z:.4f}]", end="")
    if force and data_collection.device.force:
        print(f" | Force: [{force.x:.4f}, {force.y:.4f}, {force.z:.4f}]", end="")
    print()

    # Build CSV row based on data collection config
    writer = csv.writer(output_csv_stream)
    row = [timestamp]
    
    # Device data based on config
    if data_collection.device.position and device_pos:
        row += [device_pos.x, device_pos.y, device_pos.z]
    if data_collection.device.proxy_position and proxy_pos:
        row += [proxy_pos.x, proxy_pos.y, proxy_pos.z]
    if data_collection.device.force and force:
        row += [force.x, force.y, force.z]
    if data_collection.device.velocity:
        if device_vel:
            row += [device_vel.x, device_vel.y, device_vel.z]
        else:
            row += ["", "", ""]
    if data_collection.device.angular_velocity:
        if device_ang_vel:
            row += [device_ang_vel.x, device_ang_vel.y, device_ang_vel.z]
        else:
            row += ["", "", ""]
    if data_collection.device.orientation:
        if device_quat:
            row += [device_quat.x, device_quat.y, device_quat.z, device_quat.w]
        else:
            row += ["", "", "", ""]

    # Add object state data based on config
    # Store object IDs as CSV writer metadata if this is the first data point
    if not hasattr(output_csv_stream, '_object_ids'):
        output_csv_stream._object_ids = [obj.id for obj in data.objects]
    
    # Write object data in consistent order
    for obj_id in output_csv_stream._object_ids:
        # Find this object in the current data
        obj = next((o for o in data.objects if o.id == obj_id), None)
        
        # Count expected columns for this object
        expected_cols = 0
        if data_collection.objects.position: expected_cols += 3
        if data_collection.objects.orientation: expected_cols += 4
        if data_collection.objects.contact: expected_cols += 1
        if data_collection.objects.contact_point: expected_cols += 3
        if data_collection.objects.contact_normal: expected_cols += 3
        if data_collection.objects.penetration_depth: expected_cols += 1
        if data_collection.objects.force: expected_cols += 3
        if data_collection.objects.spring_force: expected_cols += 3
        if data_collection.objects.damping_force: expected_cols += 3
        
        if obj:
            # Position
            if data_collection.objects.position:
                row += [obj.position.x, obj.position.y, obj.position.z]
            
            # Orientation
            if data_collection.objects.orientation:
                row += [obj.orientation.x, obj.orientation.y, obj.orientation.z, obj.orientation.w]
            
            # Contact detection
            if data_collection.objects.contact:
                in_contact = obj.inContact if obj.HasField("inContact") else False
                row.append(1 if in_contact else 0)
            
            # Contact point
            if data_collection.objects.contact_point:
                if obj.HasField("contactPoint"):
                    row += [obj.contactPoint.x, obj.contactPoint.y, obj.contactPoint.z]
                else:
                    row += ["", "", ""]
            
            # Contact normal
            if data_collection.objects.contact_normal:
                if obj.HasField("contactNormal"):
                    row += [obj.contactNormal.x, obj.contactNormal.y, obj.contactNormal.z]
                else:
                    row += ["", "", ""]
            
            # Penetration depth
            if data_collection.objects.penetration_depth:
                if obj.HasField("penetrationDepth"):
                    row.append(obj.penetrationDepth)
                else:
                    row.append("")
            
            # Per-object force
            if data_collection.objects.force:
                if obj.HasField("force"):
                    row += [obj.force.x, obj.force.y, obj.force.z]
                else:
                    row += ["", "", ""]
            
            # Spring force
            if data_collection.objects.spring_force:
                if obj.HasField("springForce"):
                    row += [obj.springForce.x, obj.springForce.y, obj.springForce.z]
                else:
                    row += ["", "", ""]
            
            # Damping force
            if data_collection.objects.damping_force:
                if obj.HasField("dampingForce"):
                    row += [obj.dampingForce.x, obj.dampingForce.y, obj.dampingForce.z]
                else:
                    row += ["", "", ""]
        else:
            # Object not present in this frame, fill with empty values
            row += [""] * expected_cols

    writer.writerow(row)
