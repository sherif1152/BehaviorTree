#include <iostream>
#include <chrono>
#include <thread>
#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/bt_factory.h"
#include "spdlog/spdlog.h"  // ✅ Logging

using namespace std::chrono_literals;

// ============================ Battery Check ============================
bool battery_ok = false;

BT::NodeStatus CheckBattery()
{
  if (!battery_ok)
  {
    spdlog::warn("[Battery] LOW 🔋");
    std::this_thread::sleep_for(1s);
    battery_ok = true;
    return BT::NodeStatus::FAILURE;
  }
  spdlog::info("[Battery] OK ✅");
  return BT::NodeStatus::SUCCESS;
}

// ============================ Gripper ============================
class GripperInterface
{
private:
  bool _open = false;

public:
  BT::NodeStatus open()
  {
    if (_open)
    {
      spdlog::warn("[Gripper] Already open ❌");
      return BT::NodeStatus::SUCCESS;
    }
    spdlog::info("[Gripper] Opening ✅");
    _open = true;
    return BT::NodeStatus::SUCCESS;
  }

  BT::NodeStatus close()
  {
    if (!_open)
    {
      spdlog::warn("[Gripper] Already closed ❌");
      return BT::NodeStatus::SUCCESS;
    }
    spdlog::info("[Gripper] Closing ✅");
    _open = false;
    return BT::NodeStatus::SUCCESS;
  }
};

// ============================ ApproachObject ============================
class ApproachObject : public BT::StatefulActionNode
{
private:
  int _attempts;
  int _elapsed;
public:
  ApproachObject(const std::string &name, const BT::NodeConfig &config)
      : BT::StatefulActionNode(name, config), _attempts(0) {}

  static BT::PortsList providedPorts()
  {
    return {};
  }

  BT::NodeStatus onStart() override
  {
    spdlog::info("[Approach] Start moving to object...");
    _attempts = 0;
    _elapsed = 0;
    return BT::NodeStatus::RUNNING;
  }

  BT::NodeStatus onRunning() override
  {
    spdlog::debug("[Approach] Still approaching... {}s", _elapsed);
    std::this_thread::sleep_for(1s);
    _elapsed++;

    if (_elapsed < 3)
    {
      return BT::NodeStatus::RUNNING;
    }
    else if (_attempts < 1)
    {
      spdlog::warn("[Approach] Failed to reach! Retrying...");
      _attempts++;
      _elapsed = 0;
      return BT::NodeStatus::RUNNING;
    }
    else
    {
      spdlog::info("[Approach] Successfully reached the object ✅");
      return BT::NodeStatus::SUCCESS;
    }
  }

  void onHalted() override
  {
    spdlog::warn("[Approach] Halted ❌");
  }
};

// ============================ Main ============================
int main()
{
  spdlog::set_level(spdlog::level::debug);  // 👈 لو عايز تطبع كل حاجة من debug وفوق
  spdlog::info("---------------------- Start Tree ----------------------");

  BT::BehaviorTreeFactory factory;

  factory.registerSimpleCondition("CheckBattery", std::bind(CheckBattery));
  factory.registerNodeType<ApproachObject>("ApproachObject");

  GripperInterface gripper;
  factory.registerSimpleAction("OpenGripper", std::bind(&GripperInterface::open, &gripper));
  factory.registerSimpleAction("CloseGripper", std::bind(&GripperInterface::close, &gripper));

  auto tree = factory.createTreeFromFile("../tree.xml");
  tree.tickWhileRunning();

  spdlog::info("---------------------- Tree Finished -------------------");
  return 0;
}
