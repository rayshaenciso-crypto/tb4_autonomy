#include <memory>
#include <string>

#include "behaviortree_cpp_v3/action_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

class ClearUrgentGoal : public BT::SyncActionNode
{
public:
    ClearUrgentGoal(
        const std::string & name,
        const BT::NodeConfiguration & config)
        : BT::SyncActionNode(name, config)
    {
    }

    BT::NodeStatus tick() override
    {
        auto blackboard = config().blackboard;
        blackboard->set("urgent_handled", true);

        return BT::NodeStatus::SUCCESS;
    }

    static BT::PortsList providedPorts()
    {
        return {};
    }
};

BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<ClearUrgentGoal>("ClearUrgentGoal");
}
