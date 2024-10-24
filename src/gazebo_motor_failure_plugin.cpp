#include <gazebo_motor_failure_plugin.h>
#include <gazebo/common/Events.hh>
#include <gazebo/physics/physics.hh>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>

namespace gazebo {

GazeboMotorFailure::GazeboMotorFailure() :
    ModelPlugin(),
    ROS_motor_num_sub_topic_(kDefaultROSMotorNumSubTopic),
    motor_failure_num_pub_topic_(kDefaultMotorFailureNumPubTopic)
{ }

GazeboMotorFailure::~GazeboMotorFailure() {
  this->updateConnection_.reset();
}

void GazeboMotorFailure::Load(physics::ModelPtr _model, sdf::ElementPtr _sdf) {

  this->namespace_.clear();

  if (_sdf->HasElement("robotNamespace"))
    this->namespace_ = _sdf->GetElement("robotNamespace")->Get<std::string>() + "/";

  node_handle_ = transport::NodePtr(new transport::Node());
  node_handle_->Init(namespace_);

  motor_failure_pub_ = node_handle_->Advertise<msgs::Int>(motor_failure_num_pub_topic_, 1);

  if (_sdf->HasElement("ROSMotorNumSubTopic")) {
    this->ROS_motor_num_sub_topic_ = _sdf->GetElement("ROSMotorNumSubTopic")->Get<std::string>();
  }

  if (_sdf->HasElement("MotorFailureNumPubTopic")) {
    this->motor_failure_num_pub_topic_ = _sdf->GetElement("MotorFailureNumPubTopic")->Get<std::string>();
  }

  // ROS 2 Topic subscriber
  // Initialize ROS 2, if it has not already been initialized.
  if (!rclcpp::ok()) {
    int argc = 0;
    char **argv = nullptr;
    rclcpp::init(argc, argv);
  }

  // Create our ROS 2 node. This acts similarly to the Gazebo node.
  this->ros_node_ = rclcpp::Node::make_shared("motor_failure");

  // Create a named topic and subscribe to it.
  subscription = this->ros_node_->create_subscription<std_msgs::msg::Int32>(
    this->ROS_motor_num_sub_topic_, 10,
    std::bind(&GazeboMotorFailure::motorFailNumCallBack, this, std::placeholders::_1));

  std::cout << "[gazebo_motor_failure_plugin]: Subscribed to ROS topic: " << ROS_motor_num_sub_topic_ << std::endl;

  // Listen to the update event. This event is broadcast every simulation iteration.
  this->updateConnection_ = event::Events::ConnectWorldUpdateBegin(
    std::bind(&GazeboMotorFailure::OnUpdate, this, std::placeholders::_1));
}

void GazeboMotorFailure::OnUpdate(const common::UpdateInfo &info) {
    this->motor_failure_msg_.set_data(motor_Failure_Number_);
    this->motor_failure_pub_->Publish(motor_failure_msg_);
    rclcpp::spin_some(this->ros_node_);
}

void GazeboMotorFailure::motorFailNumCallBack(const std_msgs::msg::Int32::SharedPtr msg) {
  this->motor_Failure_Number_ = msg->data;
}

GZ_REGISTER_MODEL_PLUGIN(GazeboMotorFailure);
}
