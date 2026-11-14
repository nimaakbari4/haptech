#pragma once
#include <array>
#include <cmath>
#include <algorithm>
#include <stdexcept>
namespace nima_reaching {
using Point=std::array<double,3>;
inline double length(Point p){return std::sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2]);}
inline Point difference(Point a,Point b){return {a[0]-b[0],a[1]-b[1],a[2]-b[2]};}
struct Config {
 int repetitions=3,sign=1;
 double distance=.25,min_time=.625,max_time=.775,onset_speed=.03,onset_confirm=.020;
 double hold=.2,feedback=1.5,return_speed=.1,channel_k=300,channel_b=12,max_force=8,force_slew=40,target_k=300;
 double depth_k = 600.0;  // X-axis stiffness in N/m
 bool free_reach = false;
 void validate() const {
  double v[]={distance,min_time,max_time,onset_speed,onset_confirm,hold,feedback,return_speed,channel_k,depth_k,channel_b,max_force,force_slew,target_k};
  for(double x:v)if(!std::isfinite(x)||x<=0)throw std::runtime_error("Nima reaching parameters must be finite and positive");
  if(repetitions<1||repetitions>200||(sign!=1&&sign!=-1)||distance<.05||distance>.25||min_time>=max_time||max_time>=2||
    onset_speed>.1||onset_confirm>.1||hold>1||feedback>5||return_speed<.01||return_speed>.1||channel_k>1000||depth_k>1000||
    channel_b>30||max_force>10||force_slew>50||target_k>1000)throw std::runtime_error("Nima reaching parameter out of range");
 }
};
enum class Phase{InitialReturn,Settling,Countdown,Go,Reach,Hold,Feedback,ReturnWait,Return,Done,Fault,Cancelled};
inline const char* name(Phase p){switch(p){
 case Phase::InitialReturn:return "initial_return";case Phase::Settling:return "settling";case Phase::Countdown:return "countdown";
 case Phase::Go:return "go";case Phase::Reach:return "reaching";case Phase::Hold:return "target_hold";case Phase::Feedback:return "feedback";
 case Phase::ReturnWait:return "return_wait";case Phase::Return:return "returning";case Phase::Done:return "done";
 case Phase::Fault:return "fault";case Phase::Cancelled:return "cancelled";}return "unknown";}
struct Input{double time=0;Point p{},v{};int return_status=0;bool alive=true;};
struct State{
 Phase phase=Phase::InitialReturn;int trial=0,count=3;Point home{};bool home_set=false,request_return=false;
 double time=0,go=-1,onset=-1,entry=-1,movement=-1,reaction=-1,progress=0,cross_track=0;
 const char* result="none";
};
class Logic {
 Config c_;State s_;double entered_=0,stable_=-1,candidate_=-1,hold_from_=-1;bool issued_=false;
 void phase(Phase p){s_.phase=p;entered_=s_.time;stable_=-1;issued_=false;}
 void feedback(const char* result){s_.result=result;phase(Phase::Feedback);}
 bool inside()const{return s_.progress>=c_.distance-.01&&s_.progress<=c_.distance+.01&&s_.cross_track<=.01;}
public:
 explicit Logic(Config c):c_(c){c_.validate();}
 const State& state()const{return s_;}
 void fault(const char* reason){s_.result=reason;phase(Phase::Fault);}
 void cancel(){s_.result="cancelled";phase(Phase::Cancelled);}
 const State& step(Input in){
  s_.request_return=false;
  if(s_.phase==Phase::Done||s_.phase==Phase::Fault||s_.phase==Phase::Cancelled)return s_;
  if(!std::isfinite(in.time)||in.time<s_.time){fault("invalid_clock");return s_;}
  if(in.time-s_.time>.1&&s_.time>0){fault("task_loop_gap");return s_;}
  s_.time=in.time;
  for(double v:in.p)if(!std::isfinite(v)){fault("nonfinite_position");return s_;}
  for(double v:in.v)if(!std::isfinite(v)){fault("nonfinite_velocity");return s_;}
  if(!in.alive||in.return_status<0){fault("robot_control_stopped");return s_;}
  auto d=difference(in.p,s_.home);s_.progress=c_.sign*d[1];s_.cross_track=std::abs(d[2]);
  if(s_.home_set&&length(d)>.350){fault("workspace_limit");return s_;}
  if(length(in.v)>1.5){fault("speed_limit");return s_;}
  switch(s_.phase){
  case Phase::InitialReturn:case Phase::Return:
   if(!issued_){s_.request_return=true;issued_=true;}
   else if(in.return_status==2){
    if(!s_.home_set){s_.home=in.p;s_.home_set=true;}
    if(length(difference(in.p,s_.home))>.003){fault("return_position_mismatch");break;}
    phase(Phase::Settling);
   }
   if(s_.time-entered_>45)fault("return_timeout");
   break;
  case Phase::Settling:
   if(length(difference(in.p,s_.home))<=.003&&length(in.v)<.015){
    if(stable_<0)stable_=s_.time;
    if(s_.time-stable_>=.5){if(s_.trial>=c_.repetitions){phase(Phase::Done);break;}
     ++s_.trial;s_.go=s_.onset=s_.entry=s_.movement=s_.reaction=-1;s_.result="none";candidate_=hold_from_=-1;s_.count=3;phase(Phase::Countdown);}
   }else stable_=-1;
   // Wait at green without a behavioral settling deadline.
   break;
  case Phase::Countdown:
   s_.count=std::max(1,3-int(s_.time-entered_));
   if(s_.time-entered_>=3){s_.go=s_.time;phase(Phase::Go);s_.count=0;}
   break;
  case Phase::Go:
   if(c_.sign*in.v[1]>=c_.onset_speed){if(candidate_<0)candidate_=s_.time;
    if(s_.time-candidate_+1e-9>=c_.onset_confirm){s_.onset=candidate_;s_.reaction=s_.onset-s_.go;phase(Phase::Reach);}}
   else candidate_=-1;
   // Wait indefinitely for confirmed participant movement after GO.
   break;
  case Phase::Reach:
      if (inside()) {
    s_.entry = s_.time;
    s_.movement = s_.entry - s_.onset;
    hold_from_ = -1;
    phase(Phase::Hold);
   }
   // Passing the target does not end the attempt; permit correction back to red.
   // No reaching deadline: classify timing only after target entry and hold.
   break;
  case Phase::Hold:
   // Permit leaving and re-entering the target; restart the hold timer below.
      if (!inside() || length(in.v) >= 0.015) {
    hold_from_ = -1;
   } else if (hold_from_ < 0) {
    hold_from_ = s_.time;
   }
   if(hold_from_>=0&&s_.time-hold_from_>=c_.hold)feedback(s_.movement<c_.min_time?"too_fast":s_.movement>c_.max_time?"too_slow":"good");
   // No hold-acquisition deadline. Completion requires the full target hold.
   break;
  case Phase::Feedback:
    if (s_.time - entered_ >= c_.feedback) {
        phase(s_.trial >= c_.repetitions ? Phase::Done : Phase::ReturnWait);
    }
    break;
  case Phase::ReturnWait:
   if(length(in.v)<.015){if(stable_<0)stable_=s_.time;if(s_.time-stable_>=.5)phase(Phase::Return);}
   else stable_=-1;
   if(s_.time-entered_>4)fault("not_stationary_for_return");
   break;
  default:break;
  }
  return s_;
 }
};
}
