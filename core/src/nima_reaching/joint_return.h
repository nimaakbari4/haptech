#pragma once
// Shared minimum-jerk joint-position reference, used by the existing Franka device.
#include <array>
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace nima_reaching {
using Joints=std::array<double,7>;
inline double blend(double u){u=std::clamp(u,0.0,1.0);return u*u*u*(10+u*(-15+6*u));}
class JointReturn {
 Joints from_,to_;double duration_=4;
public:
 JointReturn(Joints from,Joints to,double speed):from_(from),to_(to){
  if(!std::isfinite(speed)||speed<.01||speed>.1)throw std::runtime_error("Return speed factor must be 0.01 to 0.1");
  for(int j=0;j<7;++j){if(!std::isfinite(from[j])||!std::isfinite(to[j]))throw std::runtime_error("Nonfinite return joints");
   // 1.875 is the peak derivative of minimum-jerk blend. Cap joint speed at 2*factor rad/s.
   duration_=std::max(duration_,1.875*std::abs(to[j]-from[j])/(2*speed));}
 }
 double duration() const{return duration_;}
 Joints at(double time)const{Joints q{};double a=blend(time/duration_);for(int j=0;j<7;++j)q[j]=from_[j]+a*(to_[j]-from_[j]);return q;}
};
}
