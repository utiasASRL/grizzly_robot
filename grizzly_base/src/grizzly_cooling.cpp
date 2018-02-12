/**
 *
 *  \file
 *  \brief      Cooling control class for Grizzly
 *  \author     Tony Baltovski <tbaltovski@clearpathrobotics.com>
 *  \copyright  Copyright (c) 2015, Clearpath Robotics, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Clearpath Robotics, Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CLEARPATH ROBOTICS, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Please send comments, questions, or patches to code@clearpathrobotics.com
 *
 */

#include "grizzly_base/grizzly_cooling.h"


namespace grizzly_base
{

const double GrizzlyCooling::LINEAR_VEL_THRESHOLD = 0.1;  // m/s
const double GrizzlyCooling::ANGULAR_VEL_THRESHOLD = 0.4;  // rad/s
const double GrizzlyCooling::MOITON_COMMAND_TIMEOUT = 3.0;  // Seconds.

GrizzlyCooling::GrizzlyCooling(ros::NodeHandle* nh) :
  nh_(nh)
{
  cmd_fans_pub_ = nh_->advertise<std_msgs::Bool>("mcu/enable_fan", 1);

  status_sub_ = nh_->subscribe("mcu/status", 1, &GrizzlyCooling::statusCallback, this);
  cmd_vel_sub_ = nh_->subscribe("cmd_vel", 1, &GrizzlyCooling::cmdVelCallback, this);

  cmd_fans_timer_ = nh_->createTimer(ros::Duration(1.0/10), &GrizzlyCooling::cmdFansCallback, this);

  cmd_fans_msg_.data = false;
}

void GrizzlyCooling::statusCallback(const grizzly_msgs::Status::ConstPtr& status)
{
  // if (status->charger_connected)
  // {
  //   cmd_fans_msg_.fans[grizzly_msgs::Fans::CHARGER_BAY_INTAKE] = grizzly_msgs::Fans::FAN_ON_HIGH;
  //   cmd_fans_msg_.fans[grizzly_msgs::Fans::CHARGER_BAY_EXHAUST] = grizzly_msgs::Fans::FAN_ON_HIGH;
  //   charger_disconnected_ = false;
  // }
  // else if (!charger_disconnected_)
  // {
  //   cmd_fans_msg_.fans[grizzly_msgs::Fans::CHARGER_BAY_INTAKE] = grizzly_msgs::Fans::FAN_ON_LOW;
  //   cmd_fans_msg_.fans[grizzly_msgs::Fans::CHARGER_BAY_EXHAUST] = grizzly_msgs::Fans::FAN_ON_LOW;
  //   charger_disconnected_ = true;
  // }
}
void GrizzlyCooling::cmdVelCallback(const geometry_msgs::Twist::ConstPtr& twist)
{
  if (twist->linear.x >= LINEAR_VEL_THRESHOLD ||
      twist->linear.y >= LINEAR_VEL_THRESHOLD ||
      twist->angular.z >= ANGULAR_VEL_THRESHOLD)
  {
    cmd_fans_msg_.data = true;
  }
  last_motion_cmd_time_ = ros::Time::now().toSec();
}

void GrizzlyCooling::cmdFansCallback(const ros::TimerEvent&)
{
  if ((ros::Time::now().toSec() - last_motion_cmd_time_ > MOITON_COMMAND_TIMEOUT))
  {
    cmd_fans_msg_.data = false;
  }
  if (cmd_fans_msg_.data)
  {
      ROS_INFO("MESSAGE RECEIVED AS TRUE.");
  }
  else
  {
    ROS_INFO("FALSE");
  }
  cmd_fans_pub_.publish(cmd_fans_msg_);
}

}  // namespace grizzly_base