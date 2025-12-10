import grpc
import csv
import logging
import franka_trial_pb2 as pb2
import franka_trial_pb2_grpc as pb2_grpc

# Start a trial and stream back data/status updates
def run_trial(stub: pb2_grpc.FrankaTrialServiceStub, trial_id: str, output_csv_stream, 
              objects=None, start_conditions=None, end_conditions=None, scene_settings=None, 
              data_collection=None, metadata={}):
    request = pb2.TrialRequest(trial_id=trial_id)
    
    # Add objects to the request if provided
    if objects:
        for obj in objects:
            # Basic validation for object config
            if obj.type == "box":
                # Width/height/depth via params attributes
                for k in ("width", "height", "depth"):
                    try:
                        val = float(next((p.value for p in obj.params if p.key == k), "nan"))
                        if val <= 0:
                            raise ValueError
                    except Exception:
                        raise ValueError(f"Invalid {k} for object '{obj.id}' (must be > 0)")
            if obj.type == "sphere":
                try:
                    radius = float(next((p.value for p in obj.params if p.key in ("radius", "r")), "nan"))
                    if radius <= 0:
                        raise ValueError
                except Exception:
                    raise ValueError(f"Invalid radius for object '{obj.id}' (must be > 0)")
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
    
    if start_conditions:
        for condition in start_conditions:
            # Basic validation for common start condition parameters
            if condition.name == "time":
                try:
                    duration = float(next((p.value for p in condition.parameters if p.key in ("duration", "duration_seconds")), "nan"))
                    if duration < 0:
                        raise ValueError
                except Exception:
                    raise ValueError("Start condition 'time' requires positive duration")
            # Create StartCondition message
            start_cond_msg = request.startConditions.add()
            start_cond_msg.name = condition.name
            start_cond_msg.success = condition.success
            start_cond_msg.priority = condition.priority
            
            # Add parameters
            for param in condition.parameters:
                param_msg = start_cond_msg.params.add()
                param_msg.key = param.key
                param_msg.value = param.value

    # Add end conditions to the request if provided
    if end_conditions:
        for condition in end_conditions:
            # Basic validation for common end condition parameters
            if condition.name == "time":
                try:
                    duration = float(next((p.value for p in condition.parameters if p.key in ("duration", "duration_seconds")), "nan"))
                    if duration < 0:
                        raise ValueError
                except Exception:
                    raise ValueError("End condition 'time' requires positive duration")
            if condition.name == "position":
                try:
                    tol = float(next((p.value for p in condition.parameters if p.key in ("tolerance", "tol")), "nan"))
                    if tol <= 0:
                        raise ValueError
                except Exception:
                    raise ValueError("End condition 'position' requires positive tolerance")
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
    
    # Add scene settings if provided
    if scene_settings:
        scene_msg = pb2.SceneSettings()
        
        # Set background color
        if scene_settings.background_color:
            scene_msg.backgroundColor.r = scene_settings.background_color.r
            scene_msg.backgroundColor.g = scene_settings.background_color.g
            scene_msg.backgroundColor.b = scene_settings.background_color.b
        
        # Set camera
        if scene_settings.camera:
            scene_msg.camera.position.x = scene_settings.camera.position.x
            scene_msg.camera.position.y = scene_settings.camera.position.y
            scene_msg.camera.position.z = scene_settings.camera.position.z
            
            scene_msg.camera.lookAt.x = scene_settings.camera.look_at.x
            scene_msg.camera.lookAt.y = scene_settings.camera.look_at.y
            scene_msg.camera.lookAt.z = scene_settings.camera.look_at.z
            
            scene_msg.camera.up.x = scene_settings.camera.up.x
            scene_msg.camera.up.y = scene_settings.camera.up.y
            scene_msg.camera.up.z = scene_settings.camera.up.z
        
        # Set text overlays
        if scene_settings.text_overlays:
            for overlay in scene_settings.text_overlays:
                overlay_msg = scene_msg.textOverlays.add()
                overlay_msg.text = overlay.text
                overlay_msg.x = overlay.x
                overlay_msg.y = overlay.y
                overlay_msg.fontSize = overlay.font_size
                if overlay.color:
                    overlay_msg.color.r = overlay.color.r
                    overlay_msg.color.g = overlay.color.g
                    overlay_msg.color.b = overlay.color.b
                else:
                    # Default to white
                    overlay_msg.color.r = 1.0
                    overlay_msg.color.g = 1.0
                    overlay_msg.color.b = 1.0
        
        request.sceneSettings.CopyFrom(scene_msg)
    
    # Add data collection configuration if provided
    if data_collection:
        dc_msg = pb2.DataCollectionConfig()
        
        # Device data configuration
        if data_collection.device:
            dc_msg.device.position = data_collection.device.position
            dc_msg.device.velocity = data_collection.device.velocity
            dc_msg.device.angular_velocity = data_collection.device.angular_velocity
            dc_msg.device.orientation = data_collection.device.orientation
            dc_msg.device.proxy_position = data_collection.device.proxy_position
            dc_msg.device.force = data_collection.device.force
        
        # Objects data configuration
        if data_collection.objects:
            dc_msg.objects.position = data_collection.objects.position
            dc_msg.objects.orientation = data_collection.objects.orientation
            dc_msg.objects.contact = data_collection.objects.contact
            dc_msg.objects.contact_point = data_collection.objects.contact_point
            dc_msg.objects.contact_normal = data_collection.objects.contact_normal
            dc_msg.objects.penetration_depth = data_collection.objects.penetration_depth
            dc_msg.objects.force = data_collection.objects.force
            dc_msg.objects.spring_force = data_collection.objects.spring_force
            dc_msg.objects.damping_force = data_collection.objects.damping_force
        
        request.dataCollection.CopyFrom(dc_msg)
    
    try:
        response_stream = stub.RunTrial(request)
        handle_stream(response_stream, output_csv_stream, metadata, data_collection)
    except grpc.RpcError as e:
        logger = logging.getLogger("haptech")
        code = e.code() if hasattr(e, 'code') else None
        details = e.details() if hasattr(e, 'details') else str(e)
        print(f"gRPC error while starting/streaming trial: {code} - {details}")
        logger.error("gRPC error while running trial '%s': code=%s details=%s", trial_id, code, details)
        logger.exception("gRPC exception in run_trial")
        raise

# Process messages coming from the stream
def handle_stream(stream, output_csv_stream, metadata={}, data_collection=None):
    for message in stream:
        if message.HasField("statusUpdate"):
            handle_status_update(message.statusUpdate, metadata)
        elif message.HasField("dataPoint"):
            handle_data_point(message.dataPoint, output_csv_stream, metadata, data_collection)

# Handle a TrialStatusUpdate
def handle_status_update(status, metadata):
    logger = logging.getLogger("haptech")
    status_map = {0: "STARTED", 1: "ERROR", 2: "COMPLETED"}
    name = status_map.get(status.status, str(status.status))

    if name == "ERROR":
        logger.error("Server Error: %s", status.message)
    else:
        logger.info("Status: %s - %s", name, status.message)

    if not "status_updates" in metadata:
        metadata["status_updates"] = []

    metadata["status_updates"].append({"message": status.message, "status": status.status})

    print(f"Status Update: {status.message} (Status: {name})")

# Handle a TrialDataPoint
def handle_data_point(data, output_csv_stream, metadata, data_collection=None):
    timestamp = data.timestamp
    
    # Default to collecting all data if no config specified
    device_cfg = data_collection.device if data_collection else None
    objects_cfg = data_collection.objects if data_collection else None
    
    # Extract data fields
    device_pos = data.devicePosition if data.HasField("devicePosition") else None
    proxy_pos = data.proxyPosition if data.HasField("proxyPosition") else None
    force = data.force if data.HasField("force") else None
    device_vel = data.deviceVelocity if data.HasField("deviceVelocity") else None
    device_ang_vel = data.deviceAngularVelocity if data.HasField("deviceAngularVelocity") else None
    device_quat = data.deviceOrientation if data.HasField("deviceOrientation") else None

    # Build display string based on what's enabled
    display_parts = [f"{timestamp:.4f}s"]
    
    if not device_cfg or device_cfg.position:
        if device_pos:
            device_pos_str = f"[{device_pos.x:.4f}, {device_pos.y:.4f}, {device_pos.z:.4f}]"
            display_parts.append(f"Device Position: {device_pos_str}")
    
    if not device_cfg or device_cfg.proxy_position:
        if proxy_pos:
            proxy_pos_str = f"[{proxy_pos.x:.4f}, {proxy_pos.y:.4f}, {proxy_pos.z:.4f}]"
            display_parts.append(f"Proxy Position: {proxy_pos_str}")
    
    if not device_cfg or device_cfg.force:
        if force:
            force_str = f"[{force.x:.4f}, {force.y:.4f}, {force.z:.4f}]"
            display_parts.append(f"Force: {force_str}")

    # display data point
    # print(f"Data Point: {' | '.join(display_parts)}")

    # log to CSV file
    if output_csv_stream:
        writer = csv.writer(output_csv_stream)
        row = [timestamp]
        
        # Add fields based on data collection config
        # If no config, collect everything (backward compatibility)
        if not device_cfg or device_cfg.position:
            if device_pos:
                row += [device_pos.x, device_pos.y, device_pos.z]
            else:
                row += ["", "", ""]
        
        if not device_cfg or device_cfg.proxy_position:
            if proxy_pos:
                row += [proxy_pos.x, proxy_pos.y, proxy_pos.z]
            else:
                row += ["", "", ""]
        
        if not device_cfg or device_cfg.force:
            if force:
                row += [force.x, force.y, force.z]
            else:
                row += ["", "", ""]
        
        if not device_cfg or device_cfg.velocity:
            if device_vel:
                row += [device_vel.x, device_vel.y, device_vel.z]
            else:
                row += ["", "", ""]
        
        if not device_cfg or device_cfg.angular_velocity:
            if device_ang_vel:
                row += [device_ang_vel.x, device_ang_vel.y, device_ang_vel.z]
            else:
                row += ["", "", ""]
        
        if not device_cfg or device_cfg.orientation:
            if device_quat:
                row += [device_quat.x, device_quat.y, device_quat.z, device_quat.w]
            else:
                row += ["", "", "", ""]
        
        # Add object data if configured
        if objects_cfg:
            # Create a dict for quick lookup by object ID
            obj_dict = {obj.id: obj for obj in data.objects}
            
            # Create a dict for quick lookup of contact data by object ID
            contact_dict = {}
            if hasattr(data, 'contacts') and len(data.contacts) > 0:
                for contact in data.contacts:
                    # Store LAST contact for each object (in case there are duplicates)
                    contact_dict[contact.objectId] = contact
            
            # Store object IDs on first data point if not already set (for consistent column ordering)
            if "object_ids" not in metadata:
                metadata["object_ids"] = sorted([obj.id for obj in data.objects])

            # Write data for each object in consistent order
            for obj_id in metadata.get("object_ids", []):
                obj = obj_dict.get(obj_id, None)
                contact = contact_dict.get(obj_id, None)
                
                if objects_cfg.position:
                    if obj and obj.HasField("position"):
                        row += [obj.position.x, obj.position.y, obj.position.z]
                    else:
                        row += ["", "", ""]
                
                if objects_cfg.orientation:
                    if obj and obj.HasField("orientation"):
                        row += [obj.orientation.x, obj.orientation.y, obj.orientation.z, obj.orientation.w]
                    else:
                        row += ["", "", "", ""]
                
                if objects_cfg.contact:
                    # Boolean: is there a contact with this object?
                    row += [1 if contact else 0]
                
                if objects_cfg.contact_point:
                    if contact and contact.HasField("point"):
                        row += [contact.point.x, contact.point.y, contact.point.z]
                    else:
                        row += ["", "", ""]
                
                if objects_cfg.contact_normal:
                    if contact and contact.HasField("normal"):
                        row += [contact.normal.x, contact.normal.y, contact.normal.z]
                    else:
                        row += ["", "", ""]
                
                if objects_cfg.penetration_depth:
                    if contact:
                        row += [contact.penetrationDepth if hasattr(contact, 'penetrationDepth') else ""]
                    else:
                        row += [""]

        writer.writerow(row)

    metadata["trial_duration"] = data.timestamp