#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

#include "behaviortree_cpp_v3/condition_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

class UrgentGoalCondition : public BT::ConditionNode
{
public:
    UrgentGoalCondition(
        const std::string & name,
        const BT::NodeConfiguration & config)
        : BT::ConditionNode(name, config)
    {
        blackboard_ = config.blackboard;

        node_ = blackboard_->get<
            rclcpp_lifecycle::LifecycleNode::SharedPtr>("node");

        // Inicializar solamente si la variable no existe.
        bool handled = false;

        if (!blackboard_->get("urgent_handled", handled))
        {
            blackboard_->set("urgent_handled", false);
        }

        // Crear un grupo de callbacks independiente.
        callback_group_ = node_->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive,
            false);

        rclcpp::SubscriptionOptions options;
        options.callback_group = callback_group_;

        subscription_ =
            node_->create_subscription<
                geometry_msgs::msg::PoseStamped>(
                "/urgent_goal",
                10,
                [this](
                    const geometry_msgs::msg::PoseStamped::SharedPtr msg)
                {
                    urgent_goal_ = *msg;
                    goal_received_ = true;

                    blackboard_->set("urgent_handled", false);
                    blackboard_->set("urgent_goal_msg", urgent_goal_);

                    RCLCPP_INFO(
                        node_->get_logger(),
                        "Meta urgente recibida: x=%.2f, y=%.2f",
                        msg->pose.position.x,
                        msg->pose.position.y);
                },
                options);

        executor_.add_callback_group(
            callback_group_,
            node_->get_node_base_interface());
    }

    BT::NodeStatus tick() override
    {
        // Procesar mensajes pendientes de la suscripción.
        executor_.spin_some();

        bool handled = false;
        blackboard_->get("urgent_handled", handled);

        if (goal_received_ && !handled)
        {
            setOutput("goal", urgent_goal_);

            return BT::NodeStatus::SUCCESS;
        }

        return BT::NodeStatus::FAILURE;
    }

    static BT::PortsList providedPorts()
    {
        return {
            BT::OutputPort<geometry_msgs::msg::PoseStamped>(
                "goal")
        };
    }

private:
    rclcpp_lifecycle::LifecycleNode::SharedPtr node_;

    BT::Blackboard::Ptr blackboard_;

    rclcpp::CallbackGroup::SharedPtr callback_group_;

    rclcpp::executors::SingleThreadedExecutor executor_;

    rclcpp::Subscription<
        geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;

    geometry_msgs::msg::PoseStamped urgent_goal_;

    bool goal_received_{false};
};

BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<UrgentGoalCondition>(
        "UrgentGoalCondition");
}