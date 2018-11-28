/**
Software License Agreement (BSD)

\authors   Reinhard Sprung <reinhard.sprung@gmail.com>
\copyright Copyright (c) 2018, Reinhard Sprung, All rights reserved.
\authors   Mike Purvis <mpurvis@clearpathrobotics.com>
\copyright Copyright (c) 2014, Clearpath Robotics, Inc., All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the
   following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
   following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Clearpath Robotics nor the names of its contributors may be used to endorse or promote
   products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WAR-
RANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, IN-
DIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/Twist.h>
#include "teleop_twist_racing/teleop_twist_racing.h"

namespace teleop_twist_racing
{

float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * Internal members of class. This is the pimpl idiom, and allows more flexibility in adding
 * parameters later without breaking ABI compatibility, for robots which link TeleopTwistRacing
 * directly into base nodes.
 */
struct TeleopTwistRacing::Impl
{
  void joyCallback(const sensor_msgs::Joy::ConstPtr& joy);

  ros::Subscriber joy_sub;
  ros::Publisher cmd_pub;

  int axis_accel, axis_brake, axis_steer, button_deadman;
  float scale_accel, scale_steer;

  bool disable_msg_sent;
};

/**
 * Constructs TeleopTwistRacing.
 * \param nh NodeHandle to use for setting up the publisher and subscriber.
 * \param nh_param NodeHandle to use for searching for configuration parameters.
 */
TeleopTwistRacing::TeleopTwistRacing(ros::NodeHandle* nh, ros::NodeHandle* nh_param)
{
  pimpl_ = new Impl;
  pimpl_->disable_msg_sent = false;

  pimpl_->joy_sub = nh->subscribe<sensor_msgs::Joy>("/joy", 1, &TeleopTwistRacing::Impl::joyCallback, pimpl_);
  pimpl_->cmd_pub = nh->advertise<geometry_msgs::Twist>(nh_param->param<std::string>("cmd_topic", "/cmd_vel"), 1, true);
  ROS_INFO("Command topic: %s", pimpl_->cmd_pub.getTopic().c_str());

  // Send a zero message
  geometry_msgs::Twist cmd_msg;
  pimpl_->cmd_pub.publish(cmd_msg);

  nh_param->param<int>("axis_accel", pimpl_->axis_accel, 5);
  nh_param->param<int>("axis_brake", pimpl_->axis_brake, 2);
  nh_param->param<int>("axis_steer", pimpl_->axis_steer, 0);

  nh_param->param<float>("scale_accel", pimpl_->scale_accel, 1.0f);
  nh_param->param<float>("scale_steer", pimpl_->scale_steer, 1.0f);

  if (nh_param->param<bool>("enable_deadman", true))
    nh_param->param<int>("button_deadman", pimpl_->button_deadman, 0);
  else
    pimpl_->button_deadman = -1;

  ROS_INFO("Deadman button: %i", pimpl_->button_deadman);
}

/**
 * Handles /Joy callbacks and transforms them to Twist messages.
 * @param joy_msg
 */
void TeleopTwistRacing::Impl::joyCallback(const sensor_msgs::Joy::ConstPtr& joy_msg)
{
  geometry_msgs::Twist cmd_msg;

  // Send cmd_vel messages if there is no deadman button or if it's pressed
  if (button_deadman < 0 || joy_msg->buttons[button_deadman])
  {
    // ROS_INFO("axis_accel %f | axis_brake %f", joy_msg->axes[axis_accel], joy_msg->axes[axis_brake]);

    // Map from [1 .. -1] to [0 .. 1] (Note that the xbox360 triggers report 1 when released and -1 when triggered).
    float vel = map(joy_msg->axes[axis_accel], 1, -1, 0, 1);
    vel -= map(joy_msg->axes[axis_brake], 1, -1, 0, 1);

    cmd_msg.linear.x = vel * scale_accel;
    cmd_msg.angular.z = joy_msg->axes[axis_steer] * scale_steer;

    disable_msg_sent = false;
    cmd_pub.publish(cmd_msg);
  }
  // Send one empty message if deadman button is set but not pressed.
  else if (!disable_msg_sent)
  {
    disable_msg_sent = true;
    cmd_pub.publish(cmd_msg);
  }
}

/**
 * TODO: Call this upon shutdown of the node.
 */
//void TeleopJoyRace::shutdown()
//{
//  // Send a zero message
//  geometry_msgs::Twist cmd_msg;
//  pimpl_->cmd_pub.publish(cmd_msg);
//}

}  // namespace teleop_joy_race
