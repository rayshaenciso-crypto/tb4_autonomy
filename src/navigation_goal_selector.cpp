#include <memory>
#include <string>
#include <mutex>
#include <cstdint>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

#include "behaviortree_cpp_v3/action_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

class NavigationGoalSelector : public BT::SyncActionNode
{
public:
    NavigationGoalSelector(
        const std::string & name,
        const BT::NodeConfiguration & config)
        : BT::SyncActionNode(name, config)
    {
        node_ = config.blackboard->get<
            rclcpp::Node::SharedPtr>("node");

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
                    std::lock_guard<std::mutex> lock(mutex_);

                    urgent_goal_ = *msg;
                    urgent_pending_ = true;

                    ++urgent_generation_;

                    RCLCPP_WARN(
                        node_->get_logger(),
                        "EMERGENCIA %u RECIBIDA: x=%.2f, y=%.2f",
                        urgent_generation_,
                        msg->pose.position.x,
                        msg->pose.position.y);
                },
                options);

        executor_.add_callback_group(
            callback_group_,
            node_->get_node_base_interface());
    }

    static BT::PortsList providedPorts()
    {
        return {
            BT::InputPort<geometry_msgs::msg::PoseStamped>(
                "normal_goal"),

            BT::InputPort<unsigned int>(
                "completed_generation"),

            BT::OutputPort<geometry_msgs::msg::PoseStamped>(
                "selected_goal"),

            BT::OutputPort<bool>(
                "urgent_active"),

            BT::OutputPort<unsigned int>(
                "urgent_generation")
        };
    }

    BT::NodeStatus tick() override
    {
        executor_.spin_some();

        geometry_msgs::msg::PoseStamped normal_goal;

        if (!getInput("normal_goal", normal_goal))
        {
            RCLCPP_ERROR(
                node_->get_logger(),
                "No se pudo obtener la meta normal");

            return BT::NodeStatus::FAILURE;
        }

        unsigned int completed_generation = 0;

        getInput(
            "completed_generation",
            completed_generation);

        std::lock_guard<std::mutex> lock(mutex_);

        // Una confirmacion solo puede limpiar
        // la emergencia correspondiente.
        if (urgent_pending_ &&
            urgent_generation_ != 0 &&
            completed_generation == urgent_generation_)
        {
            urgent_pending_ = false;

            RCLCPP_INFO(
                node_->get_logger(),
                "Emergencia %u completada. Retomando A.",
                urgent_generation_);
        }

        if (urgent_pending_)
        {
            setOutput("selected_goal", urgent_goal_);
            setOutput("urgent_active", true);
        }
        else
        {
            setOutput("selected_goal", normal_goal);
            setOutput("urgent_active", false);
        }

        setOutput(
            "urgent_generation",
            urgent_generation_);

        return BT::NodeStatus::SUCCESS;
    }

private:
    rclcpp::Node::SharedPtr node_;

    rclcpp::CallbackGroup::SharedPtr callback_group_;

    rclcpp::executors::SingleThreadedExecutor executor_;

    rclcpp::Subscription<
        geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;

    geometry_msgs::msg::PoseStamped urgent_goal_;

    std::mutex mutex_;

    bool urgent_pending_{false};

    unsigned int urgent_generation_{0};
};

BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<NavigationGoalSelector>(
        "NavigationGoalSelector");
}