#ifndef C_ENABLE_FRANKA_DEVICE_SUPPORT
#define C_ENABLE_FRANKA_DEVICE_SUPPORT
#endif
#include "nima_reaching_task.h"
#include "simulation_manager.h"
#include "devices/CFrankaDevices.h"
#include "nima_reaching/reach_logic.h"
#include <mutex>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
namespace {
using namespace nima_reaching;
std::mutex taskMutex;
struct ActiveTask {
 Config config;Logic logic;std::shared_ptr<chai3d::cFrankaDevice> device;
 std::vector<std::string> events;
 State previous;bool emitted=false,sceneReady=false;double scale=1;chai3d::cVector3d previousForce{0,0,0};
 ActiveTask(Config c,std::shared_ptr<chai3d::cFrankaDevice> d,double s):config(c),logic(c),device(d),scale(s){}
};
std::unique_ptr<ActiveTask> task;
const std::array<double,7> referenceJoints{{0,-.785398,0,-2.356194,0,1.570796,.785398}};
bool quiet(Phase p){return p==Phase::InitialReturn||p==Phase::Settling||p==Phase::ReturnWait||p==Phase::Return||p==Phase::Done||p==Phase::Fault||p==Phase::Cancelled;}
std::string event(const ActiveTask& t){const auto& s=t.logic.state();std::ostringstream out;out<<std::setprecision(12)
 <<"NIMA_REACH_EVENT {\"time_s\":"<<s.time<<",\"trial\":"<<s.trial<<",\"phase\":\""<<name(s.phase)
 <<"\",\"result\":\""<<s.result<<"\",\"countdown\":"<<s.count<<",\"go_s\":"<<s.go<<",\"onset_s\":"<<s.onset
 <<",\"entry_s\":"<<s.entry<<",\"movement_s\":"<<s.movement<<",\"reaction_s\":"<<s.reaction<<",\"workspace_scale\":"<<t.scale
 <<",\"home_m\":["<<s.home[0]<<','<<s.home[1]<<','<<s.home[2]<<"]}";return out.str();}
void emit(){auto s=task->logic.state();auto old=task->previous;
 if(!task->emitted||old.phase!=s.phase||old.trial!=s.trial||old.count!=s.count||old.onset!=s.onset||old.entry!=s.entry){
  task->events.push_back(event(*task));task->previous=s;task->emitted=true;}
}
void placeScene(){
 auto& sim=SimulationManager::simContext;const auto& s=task->logic.state();double scale=task->scale;
 chai3d::cVector3d home(s.home[0]*scale,s.home[1]*scale,s.home[2]*scale);
 chai3d::cVector3d target=home;target.y(target.y()+task->config.sign*task->config.distance*scale);
 {std::lock_guard<std::mutex> lock(sim.objectMapMutex);
  auto green=sim.objectMap.find("nima_home"),red=sim.objectMap.find("nima_target");
  if(green==sim.objectMap.end()||red==sim.objectMap.end())throw std::runtime_error("Missing nima_home or nima_target object");
  green->second->setLocalPos(home);green->second->setHapticEnabled(false);
  red->second->setLocalPos(target);red->second->m_material->setStiffness(task->config.target_k/scale);
 }
 std::map<std::string,std::string> lineParams{{"point_a_x","0"},{"point_a_y","0"},{"point_a_z","0"},
  {"point_b_x","0"},{"point_b_y",std::to_string(task->config.sign*task->config.distance*scale)},
  {"point_b_z","0"},{"magnet_max_force","0"},{"magnet_max_distance","0.001"},{"stiffness","0"},{"line_width","3"}};
 if(!SimulationManager::addObjectFromConfig("nima_path","line",home.x(),home.y(),home.z(),0,0,0,.5,.6,.7,lineParams))
  throw std::runtime_error("Could not add reaching path display");
 {std::lock_guard<std::mutex> objects(sim.objectMapMutex);sim.objectMap.at("nima_path")->setHapticEnabled(false);}
 auto center=(home+target)*.5;
 SimulationManager::setCamera(center.x()+1.5,center.y(),center.z(),center.x(),center.y(),center.z(),0,0,1);
 task->sceneReady=true;
}
}
void NimaReachingTask::configure(const std::map<std::string,std::string>& params){
 Config c;
 auto get=[&](const char* key,double& value){auto i=params.find(key);if(i!=params.end()){size_t n=0;value=std::stod(i->second,&n);if(n!=i->second.size())throw std::runtime_error("Invalid reaching parameter");}};
 double repetitions=c.repetitions,sign=c.sign;get("repetitions",repetitions);get("axis_sign",sign);
 if(!std::isfinite(repetitions)||repetitions!=std::floor(repetitions)||repetitions<1||repetitions>200||!std::isfinite(sign)||(sign!=1&&sign!=-1))throw std::runtime_error("Invalid repetition count or axis sign");
 c.repetitions=int(repetitions);c.sign=int(sign);
 double free_reach = 0.0;
 get("free_reach", free_reach);
 if (free_reach != 0.0 && free_reach != 1.0) {
     throw std::runtime_error("free_reach must be 0 or 1");
 }
 c.free_reach = (free_reach == 1.0);
 get("distance_m",c.distance);get("min_time_s",c.min_time);get("max_time_s",c.max_time);get("onset_m_s",c.onset_speed);get("onset_confirm_s",c.onset_confirm);
 get("target_hold_s",c.hold);get("feedback_s",c.feedback);get("return_speed_factor",c.return_speed);get("channel_k_n_m",c.channel_k);
 c.depth_k = c.channel_k;
 get("depth_k_n_m", c.depth_k);
 get("channel_b_ns_m",c.channel_b);get("max_force_n",c.max_force);get("force_slew_n_s",c.force_slew);get("target_k_n_m",c.target_k);c.validate();
 auto& sim=SimulationManager::simContext;
 auto device=std::dynamic_pointer_cast<chai3d::cFrankaDevice>(sim.hapticDevice);
 if(!device||!device->nimaControlAlive())throw std::runtime_error("Nima reaching needs the running Franka device, not a blank/mouse device");
 double scale=sim.tool->getWorkspaceScaleFactor();if(!std::isfinite(scale)||scale<=0)throw std::runtime_error("Invalid workspace scale");
 {std::lock_guard<std::mutex> objects(sim.objectMapMutex);
  auto green=sim.objectMap.find("nima_home"),red=sim.objectMap.find("nima_target");
  if(green==sim.objectMap.end()||red==sim.objectMap.end())throw std::runtime_error("Nima task requires its green and red boxes");
  // Scale the dedicated task box meshes once; other experiments retain their existing scale.
  green->second->scale(scale);red->second->scale(scale);
  green->second->setHapticEnabled(false);
  red->second->m_material->setStiffness(c.target_k/scale);
 }
 std::lock_guard<std::mutex> lock(taskMutex);if(task)throw std::runtime_error("A Nima task is already active");
 task=std::make_unique<ActiveTask>(c,device,scale);
}
bool NimaReachingTask::evaluate(){
 std::lock_guard<std::mutex> lock(taskMutex);if(!task)return true;
 std::array<double,7> q{};chai3d::cVector3d p,v;
 bool alive=task->device->nimaReadState(q,p,v);
 Input in;in.time=SimulationManager::trialContext.time;in.alive=alive;in.return_status=task->device->nimaReturnStatus();
 in.p={p.x(),p.y(),p.z()};in.v={v.x(),v.y(),v.z()};
 auto s=task->logic.step(in);
 if(s.request_return&&!task->device->nimaRequestJointReturn(referenceJoints,task->config.return_speed))task->logic.fault("return_request_rejected");
 if(task->logic.state().home_set&&!task->sceneReady)placeScene();
 s=task->logic.state();if(s.phase==Phase::Fault)task->device->nimaAbortTask();emit();
 return s.phase==Phase::Done||s.phase==Phase::Fault||s.phase==Phase::Cancelled;
}
void NimaReachingTask::cancel(){std::lock_guard<std::mutex> lock(taskMutex);if(task&&task->logic.state().phase!=Phase::Done){task->logic.cancel();task->device->nimaAbortTask();emit();}}
void NimaReachingTask::clear(){std::lock_guard<std::mutex> lock(taskMutex);if(task){if(task->logic.state().phase!=Phase::Done)task->device->nimaAbortTask();task.reset();}}
bool NimaReachingTask::successful(){std::lock_guard<std::mutex> lock(taskMutex);return task&&task->logic.state().phase==Phase::Done;}
bool NimaReachingTask::active(){std::lock_guard<std::mutex> lock(taskMutex);return bool(task);}
bool NimaReachingTask::returning(){std::lock_guard<std::mutex> lock(taskMutex);return task&&quiet(task->logic.state().phase);}
bool NimaReachingTask::shouldRecord() {
    std::lock_guard<std::mutex> lock(taskMutex);
    if (!task) return true;

    const auto phase = task->logic.state().phase;
    return phase == Phase::Go || phase == Phase::Reach || phase == Phase::Hold;
}
std::string NimaReachingTask::text(){std::lock_guard<std::mutex> lock(taskMutex);if(!task)return "";auto s=task->logic.state();std::ostringstream out;
 out<<"Reach "<<s.trial<<'/'<<task->config.repetitions<<" | ";
 switch(s.phase){
 case Phase::InitialReturn:case Phase::Return:out<<"Returning to GREEN";break;
 case Phase::Settling:out<<"At GREEN - hold still";break;
 case Phase::Countdown:out<<"Hold still: "<<s.count;break;
 case Phase::Go:case Phase::Reach:out<<"GO - RED ("<<int(task->config.min_time*1000)<<"-"<<int(task->config.max_time*1000)<<" ms)";break;
 case Phase::Hold:out<<"Hold at RED";break;
 case Phase::Feedback:
  if(std::string(s.result)=="good")out<<"GOOD TIMING";
  else if(std::string(s.result)=="too_fast")out<<"TOO FAST";
  else if(std::string(s.result)=="too_slow")out<<"TOO SLOW";
  else out<<s.result;
  if(s.movement>=0)out<<" - "<<int(std::round(s.movement*1000))<<" ms";
  break;
 case Phase::ReturnWait:out<<"Returning next - let your hand settle";break;
 case Phase::Done:out<<"Finished - robot back at GREEN";break;
 case Phase::Fault:case Phase::Cancelled:out<<"STOPPED: "<<s.result;break;
 }
 return out.str();
}
std::vector<std::string> NimaReachingTask::takeEvents(){std::lock_guard<std::mutex> lock(taskMutex);std::vector<std::string> result;if(task)result.swap(task->events);return result;}
std::string NimaReachingTask::summary(){std::lock_guard<std::mutex> lock(taskMutex);return task?event(*task):"Nima reaching inactive";}
void NimaReachingTask::applyHaptics(double dt){
 std::lock_guard<std::mutex> lock(taskMutex);if(!task)return;auto& sim=SimulationManager::simContext;auto s=task->logic.state();
 if(quiet(s.phase)){
  double step=task->config.force_slew*std::clamp(dt,0.0,.005);
  double magnitude=task->previousForce.length();
  if(magnitude<=step)task->previousForce.zero();else task->previousForce=task->previousForce*((magnitude-step)/magnitude);
  sim.tool->setDeviceGlobalForce(task->previousForce);return;
 }
 std::array<double,7> q;chai3d::cVector3d p,v;
 if(!task->device->nimaReadState(q,p,v)){sim.tool->setDeviceGlobalForce(chai3d::cVector3d(0,0,0));return;}
 auto d=p-chai3d::cVector3d(s.home[0],s.home[1],s.home[2]);
  chai3d::cVector3d guide(
     -task->config.depth_k * d.x() - task->config.channel_b * v.x(),
     0,
     -task->config.channel_k * d.z() - task->config.channel_b * v.z());
 if (task->config.free_reach) {
     guide.zero();
 }
 if (s.phase == Phase::Countdown) {
     guide.y(-50 * d.y() - 5 * v.y());
 }
 auto force=sim.tool->getDeviceGlobalForce()+guide;
 if(force.length()>task->config.max_force)force=force*(task->config.max_force/force.length());
 auto delta=force-task->previousForce;double step=task->config.force_slew*std::clamp(dt,0.0,.005);
 if(delta.length()>step&&delta.length()>0)delta=delta*(step/delta.length());
 force=task->previousForce+delta;task->previousForce=force;sim.tool->setDeviceGlobalForce(force);
}
bool NimaReachingTask::readData(
    chai3d::cVector3d& p,
    chai3d::cVector3d& v,
    chai3d::cVector3d& f,
    chai3d::cVector3d* external_force, chai3d::cVector3d* angularVelocity) {
    std::lock_guard<std::mutex> lock(taskMutex);
    if (!task) return false;

    std::array<double,7> q;
    if (!task->device->nimaReadState(q, p, v, external_force, angularVelocity)) {
        return false;
    }

    if (task->device->nimaReturnStatus() == 1) {
        f.zero();
    } else {
        task->device->getForce(f);
    }
    return true;
}
