#include <memory>

#include "rclcpp/rclcpp.hpp"

class LogStatusNode : public rclcpp::Node
{
public:
    LogStatusNode()
        : Node("log_status_node")
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Log Status Node iniciado"
        );
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<LogStatusNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}