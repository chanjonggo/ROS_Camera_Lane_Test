#include <chrono>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "custom_action_interfaces/action/path_planner.hpp"


namespace custom_action_cpp
{


class PathPlannerServer : public rclcpp::Node
{
public:
    using PathPlanner = custom_action_interfaces::action::PathPlanner;
    using GoalHandlePathPlanner = rclcpp_action::ServerGoalHandle<PathPlanner>;

public:
    CUSTOM_ACTION_CPP_PUBLIC
    explicit PathPlannerServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions()) : Node("path_planner_action_server", options)
    {

    }

private:
    rclcpp_action::Server<...>::SharedPtr action_server_;

    void handle_goal(...);
    void handle_cancel(...);
    void handle_accepted(...);
};

}