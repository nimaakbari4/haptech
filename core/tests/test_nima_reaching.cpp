#include "../src/nima_reaching/reach_logic.h"
#include "../src/nima_reaching/joint_return.h"
#include <iostream>
#include <string>
#include <limits>
using namespace nima_reaching;
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct Rig{Config c;Logic logic;Input in;Rig(Config config=Config{}):c(config),logic(config){}
 State tick(){in.time+=.002;return logic.step(in);}
 void wait(Phase p,int max=30000){for(int i=0;i<max&&logic.state().phase!=p;++i){
  if(logic.state().phase==Phase::Return||logic.state().phase==Phase::InitialReturn){
   if(logic.state().request_return){in.return_status=1;in.p={};in.v={};}else if(in.return_status==1)in.return_status=2;}
  tick();if(logic.state().phase==Phase::Fault)throw std::runtime_error(logic.state().result);}
  check(logic.state().phase==p,"phase not reached");}
 void arm(){tick();in.return_status=1;tick();in.return_status=2;wait(Phase::Go);}
 void move(double duration){double start=in.time;
  while(logic.state().phase!=Phase::Hold&&logic.state().phase!=Phase::Feedback){
   double u=std::clamp((in.time-start)/duration,0.0,1.0);
   in.p={0,c.sign*.245*blend(u),0};in.v={0,c.sign*.245*30*u*u*(1-u)*(1-u)/duration,0};tick();
   if(logic.state().phase==Phase::Fault)throw std::runtime_error(logic.state().result);
   check(in.time-start<3,"reach timeout in test");}
  in.p={0,c.sign*.245,0};in.v={};wait(Phase::Feedback);
 }
};
int main(){try{
 {JointReturn r(Joints{},Joints{0,1,0,0,0,0,0},.1);check(r.at(0)[1]==0,"return initial reference");check(r.at(r.duration())[1]==1,"return endpoint");
  double last=0;for(int i=0;i<1000;++i){double t=i*r.duration()/999;double q=r.at(t)[1];check(q>=last,"return monotonic");last=q;}
  check(r.duration()>=9.375,"return velocity bound");}
 {Rig r;r.arm();double go=r.logic.state().go;for(int i=0;i<100;++i)r.tick();r.in.v={0,.04,0};double first=r.in.time+.002;
  for(int i=0;i<12;++i){r.tick();}
  check(r.logic.state().phase==Phase::Reach,"onset confirmed");check(std::abs(r.logic.state().onset-first)<1e-8,"onset backdated");check(r.logic.state().reaction==r.logic.state().onset-go,"reaction separate");}
 {Rig r;r.arm();r.in.v={0,.04,0};for(int i=0;i<4;++i)r.tick();r.in.v={};r.tick();check(r.logic.state().onset<0,"short spike rejected");}
 {Rig r;r.arm();r.in.v={0,-.1,0};for(int i=0;i<50;++i)r.tick();check(r.logic.state().onset<0,"wrong direction rejected");}
 for(auto test: {std::pair<double,const char*>{.35,"too_fast"},{.78,"good"},{1.1,"too_slow"}}){Config c;c.repetitions=1;Rig r(c);r.arm();r.move(test.first);auto s=r.logic.state();
  check(std::string(s.result)==test.second,"speed result");check(s.movement==s.entry-s.onset,"movement time");check(s.time-s.entry>=c.hold,"hold excluded");
  r.wait(Phase::Done);check(r.in.p==Point{},"final automatic return");}
 {Config c;c.repetitions=2;c.sign=-1;Rig r(c);r.arm();r.move(.78);r.wait(Phase::Go);check(r.logic.state().trial==2,"next repetition");check(r.in.p==Point{},"same start");r.move(.78);r.wait(Phase::Done);}
 {Rig r;r.arm();int trial=r.logic.state().trial;for(int i=0;i<300000;++i)r.tick();
  check(r.logic.state().phase==Phase::Go&&r.logic.state().onset<0&&!r.logic.state().request_return,"ten-minute pre-movement wait must not end attempt");
  check(r.logic.state().trial==trial,"waiting must not consume repetitions");r.move(.78);check(std::string(r.logic.state().result)=="good","reaction wait excluded from movement score");}
 {Rig r;r.arm();r.in.v={0,.04,0};for(int i=0;i<12;++i)r.tick();r.in.v={};for(int i=0;i<30000;++i)r.tick();
  check(r.logic.state().phase==Phase::Reach&&!r.logic.state().request_return,"long reach must wait for target");
  r.move(.78);check(std::string(r.logic.state().result)=="too_slow","late target arrival still receives timing feedback");}
 {Rig r;r.arm();r.in.v={0,.04,0};for(int i=0;i<12;++i)r.tick();r.in.v={};r.in.p={0,.27,0};r.tick();
  check(r.logic.state().phase==Phase::Reach,"overshoot alone must not finish");r.in.p={0,.245,0};r.tick();
  check(r.logic.state().phase==Phase::Hold,"corrective target entry");r.in.p={0,.27,0};for(int i=0;i<5000;++i)r.tick();
  check(r.logic.state().phase==Phase::Hold,"leaving target must wait for renewed hold");r.in.p={0,.245,0};r.wait(Phase::Feedback);}
 {Rig r;r.tick();r.in.return_status=2;r.wait(Phase::Countdown);r.in.p={0,.004,0};r.tick();
  check(r.logic.state().phase==Phase::Countdown&&r.logic.state().count==3,"premature drift restarts countdown, not attempt");
  r.in.p={};r.wait(Phase::Go);check(r.logic.state().trial==1,"countdown restart preserves trial number");}
 {Rig r;r.tick();r.in.return_status=2;r.wait(Phase::Settling);r.in.p={0,.004,0};for(int i=0;i<30000;++i)r.tick();
  check(r.logic.state().phase==Phase::Settling,"settling can wait without ending attempt");r.in.p={};r.wait(Phase::Countdown);}
 {Rig r;r.arm();r.logic.cancel();for(int i=0;i<2000;++i)r.tick();check(r.logic.state().phase==Phase::Cancelled&&!r.logic.state().request_return,"cancel never auto-returns");}
 {Rig r;r.arm();r.in.alive=false;r.tick();check(r.logic.state().phase==Phase::Fault&&!r.logic.state().request_return,"robot fault never returns");}
 {Rig r;r.arm();r.in.p={.3,0,0};r.tick();check(std::string(r.logic.state().result)=="workspace_limit","workspace guard");}
 {Rig r;r.arm();r.in.time+=.2;r.tick();check(std::string(r.logic.state().result)=="task_loop_gap","loop gap guard");}
 {Config c;c.distance=std::numeric_limits<double>::quiet_NaN();bool bad=false;try{c.validate();}catch(...){bad=true;}check(bad,"NaN rejected");}
 std::cout<<"PASS: indefinite participant wait, late arrival scoring, corrective reaches, countdown restart, returns and safety guards\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
