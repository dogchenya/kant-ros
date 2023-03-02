#!/usr/bin/env bash

#-------------------------------------------------------------------------------
# Author:       DogChen
# Date:         2021/08/28
# Description:  安装框架需要的所有依赖
#-------------------------------------------------------------------------------

function LOG_INFO()
{
  local msg=$(date +%Y-%m-%d" "%H:%M:%S);

  for p in $@
  do
    msg=${msg}" "${p};
  done
  echo -e "\033[32m $msg \033[0m"
}

sudo -E sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
sudo -E apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
sudo -E apt update
sudo -E apt -y install ros-melodic-ros-base

# 16.04 LTS match Kinetic LTS
# 18.04 LTS match Melodic LTS
# 20.04 LTS match Noetic LTS
ret=`apt-cache search ros-melodic`
if [[ "$ret" != "" ]]; then
  LOG_INFO "install ros-melodic success..."
else
  LOG_INFO "install ros-melodic failed..."
  exit
fi

echo "source /opt/ros/melodic/setup.bash" >> ~/.bashrc
source ~/.bashrc

sudo -E apt -y install python-rosdep python-rosinstall python-rosinstall-generator python-wstool build-essential python-pip
sudo -E pip install rosdepc
#sudo -E rosdep init
#rosdep update
sudo -E rosdepc init >> /dev/null
LOG_INFO "rosdepc init finished!"
rosdepc update >> /dev/null
LOG_INFO "rosdepc update finished!"


#pip install -i https://pypi.tuna.tsinghua.edu.cn/simple -r ./requirements.txt
