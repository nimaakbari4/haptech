# Performance Issues

Diagnosing and solving performance problems.

## Understanding Performance Requirements

### Critical Timing Requirements

**Haptic Loop**: Must run at **1000 Hz** (1 millisecond periods)
- **Why critical**: Human haptic perception requires high update rates
- **Violation symptoms**: Choppy feel, instability, reduced force quality
- **Tolerance**: Brief violations (< 5ms) acceptable, frequent violations problematic

**Graphics Loop**: Typically 60 Hz (16.7 millisecond periods)
- **Less critical**: Visual lag less noticeable than haptic lag
- **Can be reduced**: Graphics quality can be lowered to maintain haptic performance

### Performance Monitoring

**Real-time Monitoring:**
```bash
# Monitor core server performance
top -p $(pgrep core)

# Detailed system monitoring
htop

# I/O performance
iostat -x 1
```

## CPU Performance Issues

### High CPU Usage

**Symptoms:**
- Haptic loop frequency drops below 950 Hz
- System feels sluggish during trials
- Fan noise increases significantly

**Diagnostic Commands:**
```bash
# Check overall CPU usage
top

# Find CPU-intensive processes
ps aux --sort=-%cpu | head -10

# Monitor per-core usage
mpstat -P ALL 1

# Check CPU governor
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

**Solutions:**

1. **Close Unnecessary Applications**
   ```bash
   # Find and close heavy processes
   pkill -f firefox    # Example: close web browser
   pkill -f code       # Example: close VS Code
   ```

2. **Set CPU Governor to Performance**
   ```bash
   # Check current setting
   cpufreq-info
   
   # Set to performance mode (requires root)
   sudo cpufreq-set -g performance
   
   # Or for all cores
   echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
   ```

3. **Process Priority Optimization**
   ```bash
   # Run core server with high priority
   sudo nice -n -10 ./core
   
   # Or use real-time priority (careful!)
   sudo chrt -f 50 ./core
   ```

4. **Disable CPU Features That Cause Latency**
   ```bash
   # Disable CPU frequency scaling
   sudo systemctl disable ondemand
   
   # Disable Turbo Boost (can cause timing jitter)
   echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
   ```

### Context Switching Issues

**Symptoms:**
- High CPU usage but low actual work completion
- Frequent "scheduling" warnings in logs
- Inconsistent timing even with low average CPU usage

**Diagnosis:**
```bash
# Check context switches per second
vmstat 1

# Detailed process scheduling info
sudo perf stat -e context-switches,cpu-migrations ./core
```

**Solutions:**

1. **CPU Affinity**
   ```bash
   # Pin core server to specific CPU cores
   taskset -c 0,1 ./core
   ```

2. **Reduce Background Tasks**
   ```bash
   # Stop unnecessary services
   sudo systemctl stop bluetooth
   sudo systemctl stop cups
   sudo systemctl stop avahi-daemon
   ```

3. **Thread Optimization**
   - Use dedicated cores for haptic loop
   - Avoid thread migration between cores
   - Consider NUMA topology on multi-socket systems

## Memory Performance Issues

### Insufficient RAM

**Symptoms:**
- System using swap space during trials
- Memory usage > 90% of available RAM
- Sudden performance drops during long trials

**Diagnosis:**
```bash
# Check memory usage
free -h

# Monitor memory over time
watch free -h

# Check swap usage
swapon --show

# Find memory-heavy processes
ps aux --sort=-%mem | head -10
```

**Solutions:**

1. **Increase Available RAM**
   ```bash
   # Close memory-intensive applications
   pkill -f chrome
   pkill -f firefox
   
   # Clear system caches (safe)
   sudo sync
   sudo echo 3 > /proc/sys/vm/drop_caches
   ```

2. **Optimize Swap Settings**
   ```bash
   # Reduce swappiness (prefer RAM over swap)
   echo 10 | sudo tee /proc/sys/vm/swappiness
   
   # For experiments, temporarily disable swap
   sudo swapoff -a
   # Re-enable after: sudo swapon -a
   ```

3. **Memory Leak Detection**
   ```bash
   # Monitor core server memory over time
   while true; do
     ps -o pid,vsz,rss,comm -p $(pgrep core)
     sleep 5
   done
   ```

### Memory Fragmentation

**Symptoms:**
- Available memory shown but allocation failures
- Performance degradation over time
- "Cannot allocate memory" errors despite free RAM

**Solutions:**

1. **Restart Core Server Periodically**
   - For long experimental sessions
   - Between participant groups
   - If memory usage creeps up over time

2. **Hugepages for Real-time Performance**
   ```bash
   # Allocate hugepages (requires reboot)
   echo 128 | sudo tee /proc/sys/vm/nr_hugepages
   
   # Check hugepage usage
   cat /proc/meminfo | grep -i huge
   ```

## Network Performance Issues

### Python Client to Core Server Communication

**Symptoms:**
- Delays between client commands and server response
- "RPC timeout" errors
- Trial start/stop lag

**Diagnosis:**
```bash
# Test network latency
ping localhost

# Check network interface statistics
netstat -i

# Monitor network traffic
sudo iftop -i lo  # For localhost traffic
```

**Solutions:**

1. **Use Localhost for Local Setup**
   - Configure `settings.json` with `"server_host": "localhost"`
   - Avoid `127.0.0.1` if localhost works better
   - Don't use network IP for local connections

2. **Network Buffer Optimization**
   ```bash
   # Increase network buffers
   sudo sysctl net.core.rmem_max=16777216
   sudo sysctl net.core.wmem_max=16777216
   ```

3. **Disable Network Power Management**
   ```bash
   # Check network interface power management
   sudo ethtool eth0 | grep -i wake
   
   # Disable if enabled
   sudo ethtool -s eth0 wol d
   ```

### Franka Robot Network Issues

**Symptoms:**
- Robot connection drops during trials
- "Network timeout" errors
- Inconsistent force feedback

**Solutions:**

1. **Dedicated Network Interface**
   - Use separate ethernet port for robot
   - Avoid sharing network with internet/other devices
   - Use high-quality ethernet cable

2. **Network Configuration**
   ```bash
   # Set large MTU if supported
   sudo ip link set dev eth0 mtu 9000
   
   # Disable network manager for robot interface
   sudo nmcli dev set eth0 managed no
   ```

3. **Real-time Network Settings**
   ```bash
   # Increase network interrupt priority
   echo 50 | sudo tee /proc/irq/$(cat /proc/interrupts | grep eth0 | cut -d: -f1)/sched_setaffinity
   ```

## Graphics Performance Issues

### Low Frame Rate

**Symptoms:**
- Visual display updates < 30 FPS
- Jerky visual movement
- Graphics lag behind haptic feedback

**Note**: Graphics performance is less critical than haptic performance. Better to reduce graphics quality than compromise haptic loop.

**Solutions:**

1. **Reduce Graphics Complexity**
   ```xml
   <!-- In XML configuration, reduce visual quality -->
   <SceneSettings>
     <Shadows enabled="false"/>
     <AntiAliasing enabled="false"/>
     <TextureQuality>low</TextureQuality>
   </SceneSettings>
   ```

2. **Graphics Driver Optimization**
   ```bash
   # Check GPU usage
   nvidia-smi  # For NVIDIA cards
   
   # Update graphics drivers
   sudo apt update && sudo apt upgrade
   ```

3. **Display Settings**
   ```bash
   # Reduce display resolution for testing
   xrandr --output HDMI-1 --mode 1920x1080  # Example
   
   # Disable compositing
   sudo systemctl stop gdm3  # Be careful - this stops GUI
   ```

## Storage Performance Issues

### Data Collection Lag

**Symptoms:**
- CSV files missing data points
- "Write error" messages during trials
- Delays when saving large datasets

**Diagnosis:**
```bash
# Check disk I/O
iostat -x 1

# Check disk space
df -h

# Test write speed
dd if=/dev/zero of=/tmp/testfile bs=1M count=100
```

**Solutions:**

1. **Use SSD for Data Collection**
   - Store results on SSD, not HDD
   - Use fast USB 3.0+ external SSD if needed
   - Avoid network-attached storage for real-time data

2. **Reduce Data Collection Frequency**
   ```xml
   <!-- Reduce sampling rate if needed -->
   <DataCollection>
     <SampleRate>500</SampleRate>  <!-- Instead of 1000 -->
   </DataCollection>
   ```

3. **Optimize File System**
   ```bash
   # Use ext4 with noatime option
   sudo mount -o remount,noatime /
   
   # Or add to /etc/fstab permanently
   # /dev/sda1 / ext4 defaults,noatime 0 1
   ```

## System-Level Optimizations

### Real-time Kernel

**When to consider:**
- Consistent timing violations despite other optimizations
- Critical research requiring guaranteed timing
- Professional/clinical applications

**Installation (Ubuntu):**
```bash
# Install real-time kernel
sudo apt install linux-image-rt-amd64

# Update GRUB to use RT kernel
sudo update-grub

# Reboot and select RT kernel in GRUB menu
```

**Configuration:**
```bash
# Add user to realtime group
sudo groupadd realtime
sudo usermod -a -G realtime $USER

# Configure real-time limits
echo '@realtime soft rtprio 99' | sudo tee -a /etc/security/limits.conf
echo '@realtime soft priority 99' | sudo tee -a /etc/security/limits.conf
echo '@realtime soft memlock 102400' | sudo tee -a /etc/security/limits.conf
echo '@realtime hard rtprio 99' | sudo tee -a /etc/security/limits.conf
echo '@realtime hard priority 99' | sudo tee -a /etc/security/limits.conf
echo '@realtime hard memlock 102400' | sudo tee -a /etc/security/limits.conf
```

### Power Management

**Disable Power Saving Features:**
```bash
# Disable CPU frequency scaling
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Disable USB autosuspend
echo -1 | sudo tee /sys/module/usbcore/parameters/autosuspend

# Disable ACPI power management
# Add to /etc/default/grub:
# GRUB_CMDLINE_LINUX="acpi=off"
```

### BIOS Settings

**Recommended BIOS changes:**
- Disable C-states (CPU power saving)
- Disable P-states (CPU frequency scaling)
- Enable VT-x/AMD-V if using virtualization
- Disable hyperthreading for maximum determinism
- Set memory frequency to maximum stable speed

## Performance Testing

### Benchmark Tests

**Haptic Loop Test:**
```bash
# Run core server with benchmark mode
./core --benchmark --duration=60  # 1-minute test
```

**System Stress Test:**
```bash
# Install stress testing tools
sudo apt install stress stress-ng

# CPU stress test
stress-ng --cpu 4 --timeout 60s

# Combined stress test
stress-ng --cpu 2 --io 1 --vm 1 --vm-bytes 128M --timeout 60s
```

**Network Latency Test:**
```bash
# Test localhost latency
ping -c 100 -i 0.01 localhost

# Test gRPC performance
# (requires custom test client)
python test_grpc_latency.py
```

### Performance Baseline

**Establish baseline performance:**
1. **Fresh system boot**
2. **Minimal running processes**
3. **Standard test configuration**
4. **Record all metrics**

**Acceptable Performance Targets:**
- Haptic loop: > 995 Hz average, < 5 violations/minute
- Graphics: > 30 FPS for visual trials
- Memory: < 80% usage during trials
- CPU: < 70% average usage
- Network: < 1ms localhost latency

### Continuous Monitoring

**Automated Performance Logging:**
```bash
#!/bin/bash
# performance_monitor.sh
while true; do
    echo "$(date): CPU: $(top -bn1 | grep 'Cpu(s)' | awk '{print $2}' | cut -d'%' -f1), " \
         "MEM: $(free | grep Mem | awk '{printf "%.1f", $3/$2 * 100.0}')%" \
         >> performance.log
    sleep 1
done
```

**Run during experiments to track performance over time.**

## Getting Performance Help

### Information to Collect

**System Information:**
```bash
# Hardware details
lshw -short

# CPU information
lscpu

# Memory information
sudo dmidecode --type memory

# Graphics information
lspci | grep -i vga
```

**Performance Logs:**
- Haptic loop frequency over time
- CPU usage during problem periods  
- Memory usage trends
- Any error messages or warnings
- Configuration files being used

**When to Seek Help:**
- Performance issues persist after following this guide
- Hardware limitations need assessment
- Real-time kernel setup required
- Custom optimization needed for specific use case