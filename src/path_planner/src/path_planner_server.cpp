#include <chrono>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "custom_action_interfaces/action/path_planner.hpp"


namespace path_planner
{


class PathPlannerServer : public rclcpp::Node
{
public:
    using PathPlanner = custom_action_interfaces::action::PathPlanner;
    using GoalHandlePathPlanner = rclcpp_action::ServerGoalHandle<PathPlanner>;

public:
    explicit PathPlannerServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions()) : Node("path_planner_action_server", options)
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Path Planner Server started");

        auto handle_goal = [this](const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const PathPlanner::Goal> goal)
        {
            RCLCPP_INFO(this->get_logger(), "Received goal request with target_pos %lf", goal->target_pos[0]);
            (void)uuid;
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        };

        auto handle_cancel = [this](const std::shared_ptr<GoalHandlePathPlanner> goal_handle)
        {
            RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
            (void)goal_handle;
            return rclcpp_action::CancelResponse::ACCEPT;
        };

        auto handle_accepted = [this](const std::shared_ptr<GoalHandlePathPlanner> goal_handle)
        {
            // this needs to return quickly to avoid blocking the executor,
            // so we declare a lambda function to be called inside a new thread
            auto execute_in_thread = [this, goal_handle](){return this->execute(goal_handle);};
            std::thread{execute_in_thread}.detach();
        };

        this->action_server_ = rclcpp_action::create_server<PathPlanner>(
        this,
        "PathPlanner",
        handle_goal,
        handle_cancel,
        handle_accepted);
    }

    void execute(const std::shared_ptr<GoalHandlePathPlanner> goal_handle)
    {
        auto feedback = std::make_shared<PathPlanner::Feedback>();

        auto result = std::make_shared<PathPlanner::Result>();

        for (int i = 0; i <= 10; ++i)
        {
            // ① 취소 여부 확인
            if (goal_handle->is_canceling())
            {
                result->success = false;

                goal_handle->canceled(result);

                RCLCPP_INFO(
                    this->get_logger(),
                    "Path planning canceled");

                return;
            }

            // ② 진행 상황 전달
            feedback->current_step = i;
            feedback->distance_remaining = 100.0f - i * 10.0f;

            goal_handle->publish_feedback(feedback);

            // ③ 실제 경로 계산을 시뮬레이션
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // ④ 정상 완료
        result->success = true;
        result->total_time = 5.0f;

        goal_handle->succeed(result);

        RCLCPP_INFO(
            this->get_logger(),
            "Path planning completed");
    }


private:
    rclcpp_action::Server<PathPlanner>::SharedPtr action_server_;

};// class PathPlannerServer


}  // namespace custom_action_cpp


int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    rclcpp::spin(
        std::make_shared<path_planner::PathPlannerServer>());

    rclcpp::shutdown();

    return 0;
}


//RCLCPP_COMPONENTS_REGISTER_NODE(path_planner::PathPlannerServer)