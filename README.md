# ROS teleoperation node for gamepads with racing game like controls

This software is inspired by and forked from [http://wiki.ros.org/teleop_twist_joy](http://wiki.ros.org/teleop_twist_joy).

Instead of using the analog axes of a gamepad, the trigger buttons are used for forward and backwards control to simulate the behaviour of typical racing video games.
The right trigger increases the forward speed while the left trigger reduces it. Fully pressing both triggers negates both speed to 0. Default output topic of this node is "/cmd_vel" with a [geometry_msgs/Twist](http://docs.ros.org/api/geometry_msgs/html/msg/Twist.html) message.

## Supported Hardware

This node should work with any joystick that is supported by Linux. However, I only tested it with an Xbox360 wireless controller and therefore only provided a config file for that controller. Feel free to create and send me yours.

