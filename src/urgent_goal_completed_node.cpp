#include <string>

#include "behaviortree_cpp_v3/action_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

class UrgentGoalCompleted : public BT::SyncActionNode
{
public:
    UrgentGoalCompleted(
        const std::string & name,
        const BT::NodeConfiguration & config)
        : BT::SyncActionNode(name, config)
    {
    }

    static BT::PortsList providedPorts()
    {
        return {
            BT::InputPort<unsigned int>(
                "urgent_generation"),

            BT::InputPort<bool>(
                "urgent_active"),

            BT::OutputPort<unsigned int>(
                "completed_generation")
        };
    }

    BT::NodeStatus tick() override
    {
        unsigned int generation = 0;
        bool urgent_active = false;

        if (!getInput("urgent_generation", generation) ||
            !getInput("urgent_active", urgent_active))
        {
            return BT::NodeStatus::FAILURE;
        }

        // Se completo una emergencia.
        if (urgent_active)
        {
            setOutput("completed_generation", generation);

            // Solicitar otro ciclo de navegacion.
            return BT::NodeStatus::FAILURE;
        }

        // Se completo la mision normal.
        return BT::NodeStatus::SUCCESS;
    }
};

BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<UrgentGoalCompleted>(
        "UrgentGoalCompleted");
}