#include <iostream>
#include <chrono>
#include <thread>
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/action_node.h"

using namespace std::chrono_literals;

// Custom action node to approach an object
class ApproachObject : public BT::CoroActionNode
{
public:
  explicit ApproachObject(const std::string &name) : BT::CoroActionNode(name, {}) {}

  BT::NodeStatus tick() override
  {
    std::cout << "Approaching object...\n";
    setStatusRunningAndYield();  // ⏸️ Pause for some time
    std::this_thread::sleep_for(3s);
    return BT::NodeStatus::SUCCESS;
  }

  void halt() override
  {
    std::cout << "ApproachObject halted\n";
  }
};

// Custom condition node to check battery status
BT::NodeStatus CheckBattery()
{
  std::cout << "Battery OK" << std::endl;
  return BT::NodeStatus::FAILURE;
}

// Custom condition node to check object location
BT::NodeStatus CheckObjectLocation()
{
  std::cout << "Object in range" << std::endl;
  return BT::NodeStatus::FAILURE;
}

class GripperInterface
{
private:
  bool _open;

public:
  GripperInterface() : _open(true) {}

  BT::NodeStatus open()
  {
    _open = true;
    std::cout << "Gripper open" << std::endl;
    return BT::NodeStatus::SUCCESS;
  }

  BT::NodeStatus close()
  {
    _open = false;
    std::cout << "Gripper close" << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
};

int main()
{
  BT::BehaviorTreeFactory factory;

  factory.registerNodeType<ApproachObject>("ApproachObject");
  factory.registerSimpleCondition("CheckBattery", std::bind(CheckBattery));
  factory.registerSimpleCondition("CheckObjectLocation", std::bind(CheckObjectLocation));

  GripperInterface gripper;
  factory.registerSimpleAction("OpenGripper", std::bind(&GripperInterface::open, &gripper));
  factory.registerSimpleAction("CloseGripper", std::bind(&GripperInterface::close, &gripper));

  auto tree = factory.createTreeFromFile("./../tree.xml");

  tree.tickWhileRunning();

  return 0;
}
