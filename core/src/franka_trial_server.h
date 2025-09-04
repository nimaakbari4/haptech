#include <grpcpp/grpcpp.h>
#include "franka_trial.grpc.pb.h"
#include "simulation_manager.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

class FrankaTrialServiceImpl final : public FrankaTrialService::Service {
    Status RunTrial(ServerContext* context,
        const TrialRequest* request,
        ServerWriter<TrialResponseStream>* writer) override;
};

void runServer();