#include <memory>
#include <string>
#include <deque>
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"

using namespace std::chrono_literals;

class PriorityNavigationNode : public rclcpp::Node
{
public:

    using NavigateToPose = nav2_msgs::action::NavigateToPose;
    using GoalHandle = rclcpp_action::ClientGoalHandle<NavigateToPose>;

    enum class MissionType
    {
        NONE,
        NORMAL,
        URGENT
    };

    PriorityNavigationNode()
        : Node("priority_navigation_node")
    {
        // Cliente de acciones Nav2
        action_client_ =
            rclcpp_action::create_client<NavigateToPose>(
                this,
                "/navigate_to_pose");

        // Suscripcion a metas normales
        normal_subscription_ =
            create_subscription<geometry_msgs::msg::PoseStamped>(
                "/mission_goal",
                10,
                [this](
                    const geometry_msgs::msg::PoseStamped::SharedPtr msg)
                {
                    normal_goal_ = *msg;
                    normal_pending_ = true;

                    RCLCPP_INFO(
                        get_logger(),
                        "Nueva mision normal recibida: A");

                    if (current_type_ == MissionType::NORMAL)
                    {
                        cancelCurrentGoal();
                    }

                    dispatch();
                });

        // Suscripcion a emergencias
        urgent_subscription_ =
            create_subscription<geometry_msgs::msg::PoseStamped>(
                "/urgent_goal",
                10,
                [this](
                    const geometry_msgs::msg::PoseStamped::SharedPtr msg)
                {
                    urgent_queue_.push_back(*msg);

                    RCLCPP_WARN(
                        get_logger(),
                        "EMERGENCIA RECIBIDA. Cola: %zu",
                        urgent_queue_.size());

                    if (current_type_ == MissionType::NORMAL)
                    {
                        cancelCurrentGoal();
                    }

                    dispatch();
                });

        // Temporizador para gestionar solicitudes pendientes
        timer_ = create_wall_timer(
            250ms,
            [this]()
            {
                dispatch();
            });

        RCLCPP_INFO(
            get_logger(),
            "Priority Navigation Node iniciado.");

        RCLCPP_INFO(
            get_logger(),
            "Esperando metas en /mission_goal y /urgent_goal");
    }

private:

    void cancelCurrentGoal()
    {
        if (!current_goal_handle_ || cancel_requested_)
        {
            return;
        }

        if (current_type_ != MissionType::NORMAL)
        {
            return;
        }

        cancel_requested_ = true;

        RCLCPP_WARN(
            get_logger(),
            "Cancelando navegacion normal para atender emergencia.");

        action_client_->async_cancel_goal(current_goal_handle_);
    }

    void dispatch()
    {
        if (goal_request_pending_ || current_goal_handle_)
        {
            return;
        }

        if (!action_client_->action_server_is_ready())
        {
            return;
        }

        // Prioridad 1: emergencias pendientes
        if (!urgent_queue_.empty())
        {
            sendGoal(
                urgent_queue_.front(),
                MissionType::URGENT);

            return;
        }

        // Prioridad 2: recuperar mision normal
        if (normal_pending_)
        {
            sendGoal(
                normal_goal_,
                MissionType::NORMAL);
        }
    }

    void sendGoal(
        const geometry_msgs::msg::PoseStamped & pose,
        MissionType type)
    {
        NavigateToPose::Goal goal;

        goal.pose = pose;

        current_type_ = type;
        goal_request_pending_ = true;
        cancel_requested_ = false;

        if (type == MissionType::URGENT)
        {
            RCLCPP_WARN(
                get_logger(),
                "ENVIANDO META DE EMERGENCIA B");
        }
        else
        {
            RCLCPP_INFO(
                get_logger(),
                "ENVIANDO O RECUPERANDO META NORMAL A");
        }

        rclcpp_action::Client<NavigateToPose>::SendGoalOptions options;

        options.goal_response_callback =
            [this](const GoalHandle::SharedPtr & handle)
            {
                goal_request_pending_ = false;

                if (!handle)
                {
                    RCLCPP_ERROR(
                        get_logger(),
                        "Nav2 rechazo la meta.");

                    current_type_ = MissionType::NONE;
                    return;
                }

                current_goal_handle_ = handle;

                RCLCPP_INFO(
                    get_logger(),
                    "Nav2 acepto la meta.");

                // Si llego una emergencia mientras se solicitaba
                // la meta normal, cancelar inmediatamente.
                if (!urgent_queue_.empty() &&
                    current_type_ == MissionType::NORMAL)
                {
                    cancelCurrentGoal();
                }
            };

        options.result_callback =
            [this](const GoalHandle::WrappedResult & result)
            {
                MissionType finished_type = current_type_;

                current_goal_handle_.reset();
                current_type_ = MissionType::NONE;
                cancel_requested_ = false;

                if (finished_type == MissionType::URGENT)
                {
                    if (result.code == rclcpp_action::ResultCode::SUCCEEDED)
                    {
                        RCLCPP_INFO(
                            get_logger(),
                            "EMERGENCIA ATENDIDA CORRECTAMENTE.");
                    }
                    else
                    {
                        RCLCPP_ERROR(
                            get_logger(),
                            "La navegacion de emergencia no termino correctamente.");
                    }

                    if (!urgent_queue_.empty())
                    {
                        urgent_queue_.pop_front();
                    }
                }

                if (finished_type == MissionType::NORMAL)
                {
                    if (result.code == rclcpp_action::ResultCode::SUCCEEDED)
                    {
                        normal_pending_ = false;

                        RCLCPP_INFO(
                            get_logger(),
                            "MISION NORMAL COMPLETADA.");
                    }
                    else if (result.code == rclcpp_action::ResultCode::CANCELED)
                    {
                        RCLCPP_INFO(
                            get_logger(),
                            "Mision normal interrumpida. Se recuperara despues.");
                    }
                    else
                    {
                        RCLCPP_ERROR(
                            get_logger(),
                            "La navegacion normal fallo.");
                    }
                }

                dispatch();
            };

        action_client_->async_send_goal(goal, options);
    }

    // Cliente de acciones
    rclcpp_action::Client<NavigateToPose>::SharedPtr action_client_;

    // Suscripciones
    rclcpp::Subscription<
        geometry_msgs::msg::PoseStamped>::SharedPtr normal_subscription_;

    rclcpp::Subscription<
        geometry_msgs::msg::PoseStamped>::SharedPtr urgent_subscription_;

    // Temporizador
    rclcpp::TimerBase::SharedPtr timer_;

    // Objetivo normal
    geometry_msgs::msg::PoseStamped normal_goal_;

    bool normal_pending_{false};

    // Cola de emergencias
    std::deque<geometry_msgs::msg::PoseStamped> urgent_queue_;

    // Estado de navegacion
    GoalHandle::SharedPtr current_goal_handle_;

    MissionType current_type_{MissionType::NONE};

    bool goal_request_pending_{false};

    bool cancel_requested_{false};
};


int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<PriorityNavigationNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
