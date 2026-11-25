#include <memory>
#include <string>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "behaviortree_cpp_v3/action_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

class GetUrgentGoal : public BT::SyncActionNode
{
public:
    GetUrgentGoal(
        const std::string & name,
        const BT::NodeConfiguration & config)
        : BT::SyncActionNode(name, config)
    {
    }

    BT::NodeStatus tick() override
    {
        geometry_msgs::msg::PoseStamped goal;

        // Recuperar la meta urgente mediante el puerto de entrada.
        if (!getInput("urgent_goal_msg", goal))
        {
            return BT::NodeStatus::FAILURE;
        }

        // Enviar la meta al siguiente nodo del árbol.
        setOutput("goal", goal);

        return BT::NodeStatus::SUCCESS;
    }

    static BT::PortsList providedPorts()
    {
        return {
            BT::InputPort<geometry_msgs::msg::PoseStamped>(
                "urgent_goal_msg"),

            BT::OutputPort<geometry_msgs::msg::PoseStamped>(
                "goal")
        };
    }
};

BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<GetUrgentGoal>("GetUrgentGoal");
}