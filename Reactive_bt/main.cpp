#include <iostream>
#include <chrono>
#include <thread>
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/action_node.h"


namespace chr = std::chrono;
using namespace BT;
namespace chr = std::chrono;


// ====== CheckBattery Node ======
BT::NodeStatus CheckBattery()
{
  std::cout << "[Battery] OK ✅" << std::endl;
  return BT::NodeStatus::SUCCESS;
}

// ====== SaySomething Node ======
class SaySomething : public SyncActionNode
{
public:
  SaySomething(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config) {}

  static PortsList providedPorts()
  {
    return { InputPort<std::string>("message") };
  }

  NodeStatus tick() override
  {
    auto msg = getInput<std::string>("message");
    if (!msg)
    {
      throw RuntimeError("Missing required input [message]: ", msg.error());
    }

    std::cout << "[Say] " << msg.value() << std::endl;
    return NodeStatus::SUCCESS;
  }
};

// Custom type
namespace BT
{
    // ====== تعريف نوع Pose2D ======
    struct Pose2D
    {
        double x, y, theta;
    };


    template <>
    Pose2D convertFromString<Pose2D>(StringView str)
    {
        Pose2D pose;
        sscanf(str.data(), "%lf;%lf;%lf", &pose.x, &pose.y, &pose.theta);
        return pose;
    }
}

class MoveBaseAction : public BT::StatefulActionNode
{
  public:
    // Any TreeNode with ports must have a constructor with this signature
    MoveBaseAction(const std::string& name, const BT::NodeConfig& config)
      : StatefulActionNode(name, config)
    {}

    // It is mandatory to define this static method.
    static BT::PortsList providedPorts()
    {
        return{ BT::InputPort<Pose2D>("goal") };
    }

    // this function is invoked once at the beginning.
    BT::NodeStatus onStart() override;

    // If onStart() returned RUNNING, we will keep calling
    // this method until it return something different from RUNNING
    BT::NodeStatus onRunning() override;

    // callback to execute if the action was aborted by another node
    void onHalted() override;

  private:
    Pose2D _goal;
    chr::system_clock::time_point _completion_time;
};

//-------------------------

BT::NodeStatus MoveBaseAction::onStart()
{
  if ( !getInput<Pose2D>("goal", _goal))
  {
    throw BT::RuntimeError("missing required input [goal]");
  }
  printf("[ MoveBase: SEND REQUEST ]. goal: x=%f y=%f theta=%f\n",
         _goal.x, _goal.y, _goal.theta);

  // We use this counter to simulate an action that takes a certain
  // amount of time to be completed (200 ms)
  _completion_time = chr::system_clock::now() + chr::milliseconds(220);

  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus MoveBaseAction::onRunning()
{
  // Pretend that we are checking if the reply has been received
  // you don't want to block inside this function too much time.
  std::this_thread::sleep_for(chr::milliseconds(10));

  // Pretend that, after a certain amount of time,
  // we have completed the operation
  if(chr::system_clock::now() >= _completion_time)
  {
    std::cout << "[ MoveBase: FINISHED ]" << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::RUNNING;
}

void MoveBaseAction::onHalted()
{
  printf("[ MoveBase: ABORTED ]");
}

int main()
{
  BT::BehaviorTreeFactory factory;
  factory.registerSimpleCondition("CheckBattery", std::bind(CheckBattery));
  factory.registerNodeType<MoveBaseAction>("MoveBase");
  factory.registerNodeType<SaySomething>("SaySomething");

  auto tree = factory.createTreeFromFile("./../tree.xml");

  // Here, instead of tree.tickWhileRunning(),
  // we prefer our own loop.
  std::cout << "--- ticking\n";
  auto status = tree.tickOnce();
  std::cout << "--- status: " << toStr(status) << "\n\n";

  while(status == NodeStatus::RUNNING) 
  {
    // Sleep to avoid busy loops.
    // do NOT use other sleep functions!
    // Small sleep time is OK, here we use a large one only to
    // have less messages on the console.
    tree.sleep(std::chrono::milliseconds(100));

    std::cout << "--- ticking\n";
    status = tree.tickOnce();
    std::cout << "--- status: " << toStr(status) << "\n\n";
  }

  return 0;
}